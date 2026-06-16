#include "vocabularywidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QSplitter>
#include <QFrame>
#include <QScrollArea>
#include <QFont>
#include <QTimer>

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────
VocabularyWidget::VocabularyWidget(DataManager *data, QWidget *parent)
    : QWidget(parent), m_data(data)
{
    // Debounce timer: don't re-filter on every keystroke, wait 250ms
    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(250);
    connect(m_searchTimer, &QTimer::timeout, this, &VocabularyWidget::applySearchDebounced);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // ── Header ──────────────────────────────────────────────────────────────
    QHBoxLayout *headerRow = new QHBoxLayout;
    QLabel *title = new QLabel("📖  Vocabulary");
    title->setStyleSheet("font-size: 22px; font-weight: 700;");
    headerRow->addWidget(title);
    headerRow->addStretch();

    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("font-size: 13px; color: #64748b;");
    headerRow->addWidget(m_countLabel);
    mainLayout->addLayout(headerRow);

    // ── Error banner (shown only when vocabulary failed to load) ─────────────
    if (m_data->hasLoadError() || m_data->getTotalWords() == 0) {
        QWidget *errBanner = new QWidget;
        errBanner->setObjectName("errorBanner");
        errBanner->setStyleSheet(
            "QWidget#errorBanner {"
            "  background: #fee2e2; border: 1px solid #fca5a5;"
            "  border-radius: 12px;"
            "}"
        );
        QVBoxLayout *errLayout = new QVBoxLayout(errBanner);
        errLayout->setContentsMargins(20, 16, 20, 16);
        errLayout->setSpacing(4);

        QLabel *errTitle = new QLabel("⚠️  Couldn't load vocabulary data");
        errTitle->setStyleSheet("font-size: 15px; font-weight: 700; color: #991b1b; background: transparent;");

        QLabel *errDetail = new QLabel(
            "We couldn't find or read vocabulary.json. "
            "Please make sure the data file is placed in a 'data' folder next to the executable."
        );
        errDetail->setWordWrap(true);
        errDetail->setStyleSheet("font-size: 12px; color: #b91c1c; background: transparent;");

        errLayout->addWidget(errTitle);
        errLayout->addWidget(errDetail);
        mainLayout->addWidget(errBanner);
    }

    // ── Search + Filter row ──────────────────────────────────────────────────
    QHBoxLayout *toolRow = new QHBoxLayout;
    toolRow->setSpacing(12);

    m_searchBar = new QLineEdit;
    m_searchBar->setPlaceholderText("🔍  Search words, translations, categories...");
    m_searchBar->setMinimumHeight(38);
    connect(m_searchBar, &QLineEdit::textChanged, this, &VocabularyWidget::onSearch);
    toolRow->addWidget(m_searchBar, 1);

    QLabel *filterLabel = new QLabel("Filter:");
    filterLabel->setStyleSheet("font-size: 13px; color: #64748b;");
    toolRow->addWidget(filterLabel);

    m_filterCombo = new QComboBox;
    m_filterCombo->addItems({"All", "Easy", "Medium", "Hard", "Learned", "Not Learned"});
    m_filterCombo->setMinimumHeight(38);
    m_filterCombo->setMinimumWidth(140);
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VocabularyWidget::onFilterChanged);
    toolRow->addWidget(m_filterCombo);

    QLabel *catLabel = new QLabel("Category:");
    catLabel->setStyleSheet("font-size: 13px; color: #64748b;");
    toolRow->addWidget(catLabel);

    m_categoryCombo = new QComboBox;
    // Populate from DataManager so the list always matches the actual data —
    // no more hardcoded list that can drift out of sync.
    m_categoryCombo->addItem("All Categories");
    for (const QString &cat : m_data->getCategories())
        m_categoryCombo->addItem(cat);
    m_categoryCombo->setMinimumHeight(38);
    m_categoryCombo->setMinimumWidth(160);
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VocabularyWidget::onFilterChanged);
    toolRow->addWidget(m_categoryCombo);
    mainLayout->addLayout(toolRow);

    // ── Splitter: table | detail ─────────────────────────────────────────────
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(1);

    // ── Table ────────────────────────────────────────────────────────────────
    m_table = new QTableWidget;
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({"Word", "Translation", "Category", "Difficulty", "✓"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setMinimumWidth(380);
    m_table->setSortingEnabled(false); // keep off during bulk populate
    // Uniform row height: lets the viewport skip hidden-row geometry calculations
    m_table->verticalHeader()->setDefaultSectionSize(42);
    m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    connect(m_table, &QTableWidget::itemSelectionChanged,
            this, &VocabularyWidget::onTableSelectionChanged);
    splitter->addWidget(m_table);

    // ── Detail panel ─────────────────────────────────────────────────────────
    m_detailPanel = new QWidget;
    m_detailPanel->setMinimumWidth(280);
    m_detailPanel->setObjectName("detailPanel");
    m_detailPanel->setStyleSheet("QWidget#detailPanel { border-left: 1px solid #e2e8f0; }");

    QScrollArea *detailScroll = new QScrollArea;
    detailScroll->setWidgetResizable(true);
    detailScroll->setFrameShape(QFrame::NoFrame);
    detailScroll->setWidget(m_detailPanel);

    QVBoxLayout *detailLayout = new QVBoxLayout(m_detailPanel);
    detailLayout->setContentsMargins(24, 24, 24, 24);
    detailLayout->setSpacing(16);

    m_detailPlaceholder = new QLabel("Select a word\nto see details");
    m_detailPlaceholder->setAlignment(Qt::AlignCenter);
    m_detailPlaceholder->setStyleSheet("font-size: 14px; color: #94a3b8;");
    detailLayout->addWidget(m_detailPlaceholder);

    m_detailWord = new QLabel;
    m_detailWord->setStyleSheet("font-size: 26px; font-weight: 700;");
    m_detailWord->setWordWrap(true);

    m_detailTranslation = new QLabel;
    m_detailTranslation->setStyleSheet("font-size: 18px; color: #6366f1; font-weight: 600;");
    m_detailTranslation->setWordWrap(true);

    m_detailPronunciation = new QLabel;
    m_detailPronunciation->setStyleSheet("font-size: 13px; color: #64748b; font-style: italic;");
    m_detailPronunciation->setWordWrap(true);

    QFrame *divider = new QFrame;
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("color: #e2e8f0;");

    QLabel *exampleTitle = new QLabel("EXAMPLE");
    exampleTitle->setStyleSheet(
        "font-size: 10px; font-weight: 700; color: #94a3b8; letter-spacing: 2px;"
    );

    m_detailExample = new QLabel;
    m_detailExample->setStyleSheet("font-size: 13px; line-height: 1.6;");
    m_detailExample->setWordWrap(true);

    m_detailDifficulty = new QLabel;
    m_detailDifficulty->setStyleSheet("font-size: 12px;");

    m_markLearnedBtn = new QPushButton("Mark as Learned ✓");
    m_markLearnedBtn->setObjectName("primaryBtn");
    m_markLearnedBtn->setMinimumHeight(42);
    connect(m_markLearnedBtn, &QPushButton::clicked, this, &VocabularyWidget::onMarkLearned);

    detailLayout->addWidget(m_detailWord);
    detailLayout->addWidget(m_detailTranslation);
    detailLayout->addWidget(m_detailPronunciation);
    detailLayout->addWidget(divider);
    detailLayout->addWidget(exampleTitle);
    detailLayout->addWidget(m_detailExample);
    detailLayout->addWidget(m_detailDifficulty);
    detailLayout->addStretch();
    detailLayout->addWidget(m_markLearnedBtn);

    m_detailWidgets = {
        m_detailWord, m_detailTranslation, m_detailPronunciation,
        divider, exampleTitle, m_detailExample, m_detailDifficulty,
        m_markLearnedBtn
    };

    hideDetail();

    splitter->addWidget(detailScroll);
    splitter->setSizes({600, 320});
    mainLayout->addWidget(splitter, 1);

    refresh();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Public API
// ─────────────────────────────────────────────────────────────────────────────
void VocabularyWidget::refresh()
{
    applyFilters();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Search debounce
// ─────────────────────────────────────────────────────────────────────────────
void VocabularyWidget::onSearch(const QString &)
{
    m_searchTimer->start(); // restarts the 250ms countdown
}

void VocabularyWidget::applySearchDebounced()
{
    applyFilters();
}

void VocabularyWidget::onFilterChanged()
{
    // Cancel any pending debounced search so we don't double-fire
    m_searchTimer->stop();
    applyFilters();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Core filter — builds pointer list, no WordEntry copies
// ─────────────────────────────────────────────────────────────────────────────
void VocabularyWidget::applyFilters()
{
    const QString filter   = m_filterCombo->currentText();
    const QString category = m_categoryCombo->currentText();
    const QString search   = m_searchBar->text().trimmed();
    const QString searchLower = search.toLower(); // one allocation

    const bool allCats    = (category == "All Categories");
    const bool hasSearch  = !search.isEmpty();

    // Build result as pointers into m_data's vector — zero copies of WordEntry
    const QVector<WordEntry> &all = m_data->getAllWords();
    QVector<const WordEntry*> result;
    result.reserve(all.size());

    for (const WordEntry &e : all) {
        // ── Search filter (uses pre-lowercased fields) ──────────────────────
        if (hasSearch) {
            if (!e.wordLower.contains(searchLower) &&
                !e.translationLower.contains(searchLower) &&
                !e.categoryLower.contains(searchLower))
                continue;
        }

        // ── Difficulty / learned filter ─────────────────────────────────────
        if (filter == "Easy"        && e.difficulty != "easy")   continue;
        if (filter == "Medium"      && e.difficulty != "medium") continue;
        if (filter == "Hard"        && e.difficulty != "hard")   continue;
        if (filter == "Learned"     && !e.learned)               continue;
        if (filter == "Not Learned" && e.learned)                continue;

        // ── Category filter ─────────────────────────────────────────────────
        if (!allCats && e.category != category) continue;

        result.append(&e);
    }

    populateTable(result);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Populate table — recycles existing QTableWidgetItems, no alloc in hot path
// ─────────────────────────────────────────────────────────────────────────────
void VocabularyWidget::populateTable(const QVector<const WordEntry*> &words)
{
    m_currentWords = words;

    const int newCount = words.size();

    // Block signals during bulk update to prevent spurious selection events
    m_table->blockSignals(true);

    // ── Grow or shrink row capacity ─────────────────────────────────────────
    // Only call setRowCount when strictly necessary:
    //   - Growing: always needed (creates new items).
    //   - Shrinking: only when the excess is significant (> 20% unused) to avoid
    //     thrashing on incremental filter changes (e.g. typing one char at a time).
    if (newCount > m_tableRowCapacity) {
        // Pre-populate new rows with empty items so updateRow() can safely setItem
        m_table->setRowCount(newCount);
        for (int r = m_tableRowCapacity; r < newCount; ++r) {
            m_table->setItem(r, 0, new QTableWidgetItem);
            m_table->setItem(r, 1, new QTableWidgetItem);
            m_table->setItem(r, 2, new QTableWidgetItem);
            m_table->setItem(r, 3, new QTableWidgetItem);
            auto *chk = new QTableWidgetItem;
            chk->setTextAlignment(Qt::AlignCenter);
            m_table->setItem(r, 4, chk);
        }
        m_tableRowCapacity = newCount;
    } else if (newCount < m_tableRowCapacity / 2 && m_tableRowCapacity > 20) {
        // Trim aggressively only when less than half full — avoids repeated
        // alloc/dealloc when the user types/backspaces incrementally.
        m_table->setRowCount(newCount);
        m_tableRowCapacity = newCount;
    }
    // (If newCount <= capacity but > capacity/2: leave rowCount and capacity
    //  unchanged; we'll just update the visible rows and hide the rest.)

    // ── Update visible rows in-place ────────────────────────────────────────
    for (int r = 0; r < newCount; ++r)
        updateRow(r, *words[r]);

    // Hide rows beyond newCount without changing rowCount (avoids item destruction)
    const int totalRows = m_table->rowCount();
    for (int r = newCount; r < totalRows; ++r)
        m_table->hideRow(r);
    for (int r = 0; r < newCount; ++r)
        m_table->showRow(r);

    m_countLabel->setText(QString("%1 word%2").arg(newCount).arg(newCount == 1 ? "" : "s"));

    m_table->blockSignals(false);

    hideDetail();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Update a single pre-existing row (no new/delete)
// ─────────────────────────────────────────────────────────────────────────────
void VocabularyWidget::updateRow(int row, const WordEntry &e)
{
    // Column 0 — Word
    QTableWidgetItem *wItem = m_table->item(row, 0);
    wItem->setText(e.word);
    wItem->setFont(QFont("", -1, QFont::Medium));

    // Column 1 — Translation
    m_table->item(row, 1)->setText(e.translation);

    // Column 2 — Category
    QTableWidgetItem *catItem = m_table->item(row, 2);
    catItem->setText(e.category);
    catItem->setForeground(QColor("#64748b"));

    // Column 3 — Difficulty
    const QString diffColor = (e.difficulty == "easy")   ? "#10b981"
                            : (e.difficulty == "medium")  ? "#f59e0b"
                                                          : "#ef4444";
    const QString diffCap   = e.difficulty.left(1).toUpper() + e.difficulty.mid(1);
    QTableWidgetItem *diffItem = m_table->item(row, 3);
    diffItem->setText(diffCap);
    diffItem->setForeground(QColor(diffColor));
    diffItem->setFont(QFont("", -1, QFont::Medium));

    // Column 4 — Learned checkmark
    QTableWidgetItem *learnedItem = m_table->item(row, 4);
    learnedItem->setText(e.learned ? "✓" : "");
    learnedItem->setForeground(QColor("#10b981"));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Selection / detail
// ─────────────────────────────────────────────────────────────────────────────
void VocabularyWidget::onTableSelectionChanged()
{
    int row = m_table->currentRow();
    if (row < 0 || row >= m_currentWords.size()) {
        hideDetail();
        return;
    }
    showDetail(*m_currentWords[row]);
}

void VocabularyWidget::hideDetail()
{
    m_detailPlaceholder->show();
    for (QWidget *w : m_detailWidgets) w->hide();
}

void VocabularyWidget::showDetail(const WordEntry &entry)
{
    m_detailPlaceholder->hide();
    for (QWidget *w : m_detailWidgets) w->show();

    m_detailWord->setText(entry.word);
    m_detailTranslation->setText("→  " + entry.translation);
    m_detailPronunciation->setText("🔊  " + entry.pronunciation);
    m_detailExample->setText(entry.example);

    const QString diffColor = (entry.difficulty == "easy")   ? "#10b981"
                            : (entry.difficulty == "medium")  ? "#f59e0b"
                                                              : "#ef4444";
    const QString diffCap = entry.difficulty.left(1).toUpper() + entry.difficulty.mid(1);
    m_detailDifficulty->setText(QString(
        "<span style='background:%1; color:white; border-radius:6px;"
        " padding:3px 10px; font-size:12px; font-weight:600;'>%2</span>"
        "  <span style='color:#64748b; font-size:12px;'>%3</span>"
    ).arg(diffColor, diffCap, entry.category));

    if (entry.learned) {
        m_markLearnedBtn->setText("✓  Already Learned");
        m_markLearnedBtn->setEnabled(false);
        m_markLearnedBtn->setStyleSheet(
            "QPushButton { background: #d1fae5; color: #065f46; border: none;"
            " border-radius: 10px; font-size: 14px; font-weight: 600; padding: 10px 24px; }"
        );
    } else {
        m_markLearnedBtn->setText("Mark as Learned ✓");
        m_markLearnedBtn->setEnabled(true);
        m_markLearnedBtn->setObjectName("primaryBtn");
        m_markLearnedBtn->setStyleSheet("");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Mark learned
// ─────────────────────────────────────────────────────────────────────────────
void VocabularyWidget::onMarkLearned()
{
    int row = m_table->currentRow();
    if (row < 0 || row >= m_currentWords.size()) return;

    const int wordId = m_currentWords[row]->id;
    m_data->markWordLearned(wordId);

    // Refresh filters (the word's `learned` flag has changed, so counts may shift)
    // then restore selection to the same visual row if it still exists.
    applyFilters();
    if (row < m_table->rowCount())
        m_table->selectRow(row);
}
