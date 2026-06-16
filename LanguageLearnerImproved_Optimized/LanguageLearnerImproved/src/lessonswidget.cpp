#include "lessonswidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QSplitter>
#include <QDate>
#include <QScrollArea>
#include <algorithm>
#include <random>

// ─────────────────────────────────────────────────────────────────────────────
//  Helper: build a compact lesson card widget for use in list items
// ─────────────────────────────────────────────────────────────────────────────
static QWidget *makeLessonCard(const LessonInfo &lesson)
{
    QWidget *card = new QWidget;
    QHBoxLayout *lay = new QHBoxLayout(card);
    lay->setContentsMargins(14, 10, 14, 10);
    lay->setSpacing(14);

    QLabel *iconLbl = new QLabel(lesson.icon);
    iconLbl->setStyleSheet("font-size: 26px; background: transparent;");
    iconLbl->setFixedWidth(40);

    QVBoxLayout *textLay = new QVBoxLayout;
    textLay->setSpacing(3);

    QLabel *titleLbl = new QLabel(lesson.title);
    titleLbl->setStyleSheet("font-size: 13px; font-weight: 700; background: transparent;");

    QLabel *descLbl = new QLabel(lesson.description);
    descLbl->setStyleSheet("font-size: 11px; color: #64748b; background: transparent;");
    descLbl->setWordWrap(true);

    textLay->addWidget(titleLbl);
    textLay->addWidget(descLbl);

    QString tagText = lesson.completed
        ? "✅ Done"
        : QString("%1 words").arg(lesson.words.size());
    QString tagBg  = lesson.completed ? "#d1fae5" : "#ede9fe";
    QString tagFg  = lesson.completed ? "#065f46" : "#7c3aed";

    QLabel *tagLbl = new QLabel(tagText);
    tagLbl->setStyleSheet(QString(
        "font-size: 11px; font-weight: 700; background: %1;"
        " color: %2; border-radius: 8px; padding: 3px 10px;"
    ).arg(tagBg, tagFg));

    lay->addWidget(iconLbl);
    lay->addLayout(textLay, 1);
    lay->addWidget(tagLbl);

    return card;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────
LessonsWidget::LessonsWidget(DataManager *data, QWidget *parent)
    : QWidget(parent), m_data(data)
{
    buildDailyLesson();
    buildTopicLessons();

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_mainStack = new QStackedWidget;
    root->addWidget(m_mainStack);

    // ═══════════════════════════════════════════════════════════════════
    //  PAGE 0 — Lesson Browser
    // ═══════════════════════════════════════════════════════════════════
    QWidget *browserPage = new QWidget;
    QVBoxLayout *bLay = new QVBoxLayout(browserPage);
    bLay->setContentsMargins(24, 24, 24, 24);
    bLay->setSpacing(16);

    // Header row
    QHBoxLayout *headerRow = new QHBoxLayout;
    QLabel *pageTitle = new QLabel("📅  Lessons");
    pageTitle->setStyleSheet("font-size: 22px; font-weight: 700;");
    headerRow->addWidget(pageTitle);
    headerRow->addStretch();
    QLabel *dateLabel = new QLabel(QDate::currentDate().toString("dddd, MMMM d yyyy"));
    dateLabel->setStyleSheet("font-size: 13px; color: #64748b;");
    headerRow->addWidget(dateLabel);
    bLay->addLayout(headerRow);

    // Stats bar
    QWidget *statsBar = new QWidget;
    statsBar->setObjectName("statsBar");
    statsBar->setStyleSheet(
        "QWidget#statsBar {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "    stop:0 #4f46e5, stop:0.5 #7c3aed, stop:1 #ec4899);"
        "  border-radius: 14px;"
        "}"
    );
    statsBar->setFixedHeight(76);

    QHBoxLayout *sbLay = new QHBoxLayout(statsBar);
    sbLay->setContentsMargins(24, 0, 24, 0);
    sbLay->setSpacing(32);

    auto statWidget = [](const QString &value, const QString &label) -> QWidget* {
        QWidget *w = new QWidget;
        w->setAttribute(Qt::WA_TranslucentBackground);
        QVBoxLayout *l = new QVBoxLayout(w);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(2);
        QLabel *v = new QLabel(value);
        v->setStyleSheet("font-size: 24px; font-weight: 800; color: white; background: transparent;");
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet("font-size: 11px; color: rgba(255,255,255,0.7); background: transparent;");
        l->addWidget(v);
        l->addWidget(lbl);
        return w;
    };

    int streak    = data->getCurrentStreak();
    int completed = data->getCompletedLessonsCount();
    sbLay->addWidget(statWidget(QString::number(streak) + "🔥", "Day Streak"));
    sbLay->addWidget(statWidget(QString::number(completed),      "Lessons Done"));
    sbLay->addWidget(statWidget(QString::number(data->getLearnedCount()), "Words Learned"));
    sbLay->addStretch();

    QString motive = streak == 0 ? "Start today!"
                   : streak < 7  ? "Keep going!"
                   :               "Amazing streak!";
    QLabel *motiveLbl = new QLabel(motive);
    motiveLbl->setStyleSheet("font-size: 16px; font-weight: 700; color: white; background: transparent;");
    sbLay->addWidget(motiveLbl);
    bLay->addWidget(statsBar);

    // Tab widget
    m_tabWidget = new QTabWidget;
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: none; }"
        "QTabBar::tab { font-size: 13px; font-weight: 600; padding: 10px 24px;"
        "  border: none; border-radius: 8px; margin: 2px; }"
        "QTabBar::tab:selected { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "  stop:0 #7c3aed,stop:1 #ec4899); color: white; }"
        "QTabBar::tab:!selected { background: transparent; color: #64748b; }"
    );

    // ── Daily lessons tab ───────────────────────────────────────────
    QWidget *dailyTab = new QWidget;
    QVBoxLayout *dailyLay = new QVBoxLayout(dailyTab);
    dailyLay->setContentsMargins(0, 12, 0, 0);
    dailyLay->setSpacing(0);

    m_dailyList = new QListWidget;
    m_dailyList->setSpacing(6);
    m_dailyList->setFrameShape(QFrame::NoFrame);
    m_dailyList->setCursor(Qt::PointingHandCursor);
    // Use activated (single-click or Enter) to open lesson — NOT currentRowChanged
    // which fires spuriously during clear() calls in refreshListWidgets()
    connect(m_dailyList, &QListWidget::itemActivated, this, [this](QListWidgetItem *item) {
        int row = m_dailyList->row(item);
        onDailyLessonClicked(row);
    });
    // Also handle double-click on desktop
    connect(m_dailyList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        int row = m_dailyList->row(item);
        onDailyLessonClicked(row);
    });
    dailyLay->addWidget(m_dailyList, 1);
    m_tabWidget->addTab(dailyTab, "📅  Daily Lessons");

    // ── Topic lessons tab ───────────────────────────────────────────
    QWidget *topicTab = new QWidget;
    QVBoxLayout *topicLay = new QVBoxLayout(topicTab);
    topicLay->setContentsMargins(0, 12, 0, 0);
    topicLay->setSpacing(0);

    m_topicList = new QListWidget;
    m_topicList->setSpacing(6);
    m_topicList->setFrameShape(QFrame::NoFrame);
    m_topicList->setCursor(Qt::PointingHandCursor);
    connect(m_topicList, &QListWidget::itemActivated, this, [this](QListWidgetItem *item) {
        int row = m_topicList->row(item);
        onTopicLessonClicked(row);
    });
    connect(m_topicList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        int row = m_topicList->row(item);
        onTopicLessonClicked(row);
    });
    topicLay->addWidget(m_topicList, 1);
    m_tabWidget->addTab(topicTab, "🗂  Topic Lessons");

    bLay->addWidget(m_tabWidget, 1);
    m_mainStack->addWidget(browserPage);  // index 0

    // Populate lists initially
    refreshListWidgets();

    // ═══════════════════════════════════════════════════════════════════
    //  PAGE 1 — Lesson Detail
    // ═══════════════════════════════════════════════════════════════════
    QWidget *detailPage = new QWidget;
    QVBoxLayout *dLay = new QVBoxLayout(detailPage);
    dLay->setContentsMargins(24, 20, 24, 20);
    dLay->setSpacing(14);

    // Back button row
    QHBoxLayout *dHeader = new QHBoxLayout;
    m_backBtn = new QPushButton("← Back to Lessons");
    m_backBtn->setObjectName("secondaryBtn");
    m_backBtn->setFixedHeight(38);
    connect(m_backBtn, &QPushButton::clicked, this, &LessonsWidget::onBackToList);
    dHeader->addWidget(m_backBtn);
    dHeader->addStretch();
    dLay->addLayout(dHeader);

    // Lesson header card
    QWidget *lessonHdr = new QWidget;
    lessonHdr->setObjectName("lessonHdr");
    lessonHdr->setStyleSheet(
        "QWidget#lessonHdr {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "    stop:0 #1e1b4b, stop:0.5 #4f46e5, stop:1 #7c3aed);"
        "  border-radius: 16px;"
        "}"
    );
    lessonHdr->setFixedHeight(100);

    QHBoxLayout *lhLay = new QHBoxLayout(lessonHdr);
    lhLay->setContentsMargins(28, 0, 28, 0);
    lhLay->setSpacing(18);

    m_lessonIcon = new QLabel;
    m_lessonIcon->setStyleSheet("font-size: 36px; background: transparent;");
    m_lessonTitle = new QLabel;
    m_lessonTitle->setStyleSheet("font-size: 20px; font-weight: 800; color: white; background: transparent;");
    m_lessonDesc = new QLabel;
    m_lessonDesc->setStyleSheet("font-size: 12px; color: rgba(255,255,255,0.7); background: transparent;");
    m_lessonDesc->setWordWrap(true);

    QVBoxLayout *lhText = new QVBoxLayout;
    lhText->setSpacing(4);
    lhText->addWidget(m_lessonTitle);
    lhText->addWidget(m_lessonDesc);

    lhLay->addWidget(m_lessonIcon);
    lhLay->addLayout(lhText, 1);
    dLay->addWidget(lessonHdr);

    // Progress row
    QHBoxLayout *progRow = new QHBoxLayout;
    m_progressLbl = new QLabel("0 / 0 words");
    m_progressLbl->setStyleSheet("font-size: 12px; color: #64748b;");
    progRow->addWidget(m_progressLbl);
    progRow->addStretch();
    dLay->addLayout(progRow);

    m_progressBar = new QProgressBar;
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(8);
    m_progressBar->setRange(0, 1);
    m_progressBar->setValue(0);
    dLay->addWidget(m_progressBar);

    // Splitter: word list | detail panel
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(1);

    m_wordList = new QListWidget;
    m_wordList->setMinimumWidth(240);
    m_wordList->setMaximumWidth(320);
    m_wordList->setFrameShape(QFrame::NoFrame);
    m_wordList->setSpacing(4);
    connect(m_wordList, &QListWidget::currentRowChanged,
            this, &LessonsWidget::onWordSelected);
    splitter->addWidget(m_wordList);

    // Detail pane with scrolling
    QScrollArea *detScroll = new QScrollArea;
    detScroll->setWidgetResizable(true);
    detScroll->setFrameShape(QFrame::NoFrame);

    QWidget *detPane = new QWidget;
    QVBoxLayout *detLay = new QVBoxLayout(detPane);
    detLay->setContentsMargins(28, 28, 28, 28);
    detLay->setSpacing(14);

    m_detailStack = new QStackedWidget;

    // Hint page (index 0)
    QWidget *hintPage = new QWidget;
    QVBoxLayout *hintLay = new QVBoxLayout(hintPage);
    hintLay->setAlignment(Qt::AlignCenter);
    QLabel *hintLbl = new QLabel("👈  Select a word\nto study it");
    hintLbl->setAlignment(Qt::AlignCenter);
    hintLbl->setStyleSheet("font-size: 15px; color: #94a3b8;");
    hintLay->addWidget(hintLbl);
    m_detailStack->addWidget(hintPage);  // 0

    // Word detail page (index 1)
    QWidget *wordDetailPage = new QWidget;
    QVBoxLayout *wdLay = new QVBoxLayout(wordDetailPage);
    wdLay->setSpacing(10);
    wdLay->setContentsMargins(0, 0, 0, 0);

    m_detailWord = new QLabel;
    m_detailWord->setStyleSheet("font-size: 34px; font-weight: 800;");
    m_detailWord->setWordWrap(true);

    m_detailTranslation = new QLabel;
    m_detailTranslation->setStyleSheet("font-size: 22px; font-weight: 600; color: #7c3aed;");
    m_detailTranslation->setWordWrap(true);

    m_detailPronunciation = new QLabel;
    m_detailPronunciation->setStyleSheet("font-size: 14px; color: #64748b; font-style: italic;");

    QFrame *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #e2e8f0;");

    QLabel *exTitle = new QLabel("EXAMPLE SENTENCE");
    exTitle->setStyleSheet("font-size: 10px; font-weight: 800; color: #94a3b8; letter-spacing: 2px;");

    m_detailExample = new QLabel;
    m_detailExample->setStyleSheet(
        "font-size: 14px; color: #334155; background: #f8fafc;"
        " border-left: 4px solid #7c3aed; border-radius: 6px; padding: 12px 16px;"
    );
    m_detailExample->setWordWrap(true);

    wdLay->addWidget(m_detailWord);
    wdLay->addWidget(m_detailTranslation);
    wdLay->addWidget(m_detailPronunciation);
    wdLay->addSpacing(8);
    wdLay->addWidget(sep);
    wdLay->addWidget(exTitle);
    wdLay->addWidget(m_detailExample);
    wdLay->addStretch();
    m_detailStack->addWidget(wordDetailPage);  // 1

    detLay->addWidget(m_detailStack, 1);
    detScroll->setWidget(detPane);
    splitter->addWidget(detScroll);
    splitter->setSizes({290, 500});
    dLay->addWidget(splitter, 1);

    // Mark complete button
    m_markCompleteBtn = new QPushButton("✓  Mark Lesson as Complete");
    m_markCompleteBtn->setObjectName("primaryBtn");
    m_markCompleteBtn->setMinimumHeight(48);
    connect(m_markCompleteBtn, &QPushButton::clicked, this, &LessonsWidget::onMarkComplete);
    dLay->addWidget(m_markCompleteBtn);

    m_mainStack->addWidget(detailPage);  // index 1
}

// ─────────────────────────────────────────────────────────────────────────────
//  Build daily lessons (7 days of current week, 10 words each)
// ─────────────────────────────────────────────────────────────────────────────
void LessonsWidget::buildDailyLesson()
{
    m_dailyLessons.clear();
    const QVector<WordEntry> &allWords = m_data->getAllWords();  // no copy
    if (allWords.isEmpty()) return;

    const QStringList dayNames = {"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"};
    const QStringList dayIcons = {"☀️","📘","🌿","⭐","🎯","🧠","🌙"};
    const QDate today = QDate::currentDate();
    const int todayDow = today.dayOfWeek();  // 1=Mon … 7=Sun

    for (int d = 0; d < 7; ++d) {
        QDate date = today.addDays(d - todayDow + 1);  // Mon–Sun of current week
        int seed = date.year() * 10000 + date.month() * 100 + date.day();

        // Copy + shuffle only what we need
        QVector<WordEntry> shuffled = allWords;
        std::mt19937 rng(static_cast<unsigned>(seed));
        std::shuffle(shuffled.begin(), shuffled.end(), rng);

        LessonInfo info;
        info.id          = "daily_" + QString::number(d);
        info.title       = dayNames[d] + " — " + date.toString("MMM d");
        info.icon        = dayIcons[d];
        info.description = (date == today ? "Today's lesson · " : "") + QString("10 words to master");
        info.category.clear();
        info.words       = shuffled.mid(0, qMin(10, shuffled.size()));
        info.completed   = (date == today) ? m_data->isDailyLessonComplete()
                                           : m_data->getCompletedLessonsCount() > d;
        m_dailyLessons.append(info);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Build topic lessons from vocabulary categories
// ─────────────────────────────────────────────────────────────────────────────
void LessonsWidget::buildTopicLessons()
{
    m_topicLessons.clear();

    struct TopicDef {
        QString id, title, icon, description, category;
    };

    static const QVector<TopicDef> topics = {
        {"greetings",  "👋  Greetings & Phrases",    "👋", "Hello, goodbye, polite phrases",        "greetings"},
        {"food",       "🍽  Food & Drinks",           "🍽", "Meals, ingredients, tastes",             "food"},
        {"travel",     "✈  Travel & Transport",      "✈",  "Airports, hotels, getting around",       "travel"},
        {"school",     "🏫  School & Education",     "🏫", "Classroom, subjects, exams",             "school"},
        {"daily_life", "🌆  Daily Life",              "🌆", "Routines, chores, neighbourhood",        "daily_life"},
        {"home",       "🏠  Home & Family",           "🏠", "Rooms, furniture, family members",       "home"},
        {"people",     "👥  People & Relationships", "👥", "Friends, family, professions",           "people"},
        {"places",     "📍  Places & Locations",     "📍", "City, landmarks, buildings",             "places"},
        {"adjectives", "🎨  Adjectives",              "🎨", "Describing people, things, emotions",    "adjectives"},
        {"verbs",      "⚡  Action Verbs",             "⚡", "Common everyday actions",                "verbs"},
        {"transport",  "🚌  Transport",               "🚌", "Buses, trains, bicycles",                "transport"},
        {"nature",     "🌿  Nature & Environment",   "🌿", "Mountains, weather, outdoors",           "nature"},
        {"numbers",    "🔢  Numbers & Time",          "🔢", "Counting and telling time",              "numbers"},
        {"colors",     "🎨  Colors",                  "🖌", "All the colors of the rainbow",          "colors"},
        {"animals",    "🐾  Animals",                 "🐾", "Domestic and wild animals",              "animals"},
        {"health",     "💊  Health & Body",           "💊", "Medical terms, body parts, wellness",    "health"},
        {"business",   "💼  Business & Work",         "💼", "Office, meetings, professional terms",   "business"},
        {"technology", "💻  Technology",              "💻", "Computers, internet, gadgets",           "technology"},
        {"sports",     "⚽  Sports & Fitness",        "⚽", "Games, exercise, competitions",          "sports"},
        {"advanced",   "🧠  Advanced Words",          "🧠", "Hard vocabulary for fluency",            ""},
    };

    const QVector<WordEntry> &allWords = m_data->getAllWords();  // const-ref

    for (const auto &t : topics) {
        QVector<WordEntry> filtered;
        if (t.id == "advanced") {
            for (const WordEntry &w : allWords)
                if (w.difficulty == "hard") filtered.append(w);
        } else {
            for (const WordEntry &w : allWords)
                if (w.category == t.category) filtered.append(w);
        }
        if (filtered.isEmpty()) continue;

        // Cap at 20 words per topic lesson
        if (filtered.size() > 20) filtered.resize(20);

        LessonInfo info;
        info.id          = t.id;
        info.title       = t.title;
        info.icon        = t.icon;
        info.description = t.description + " · " + QString::number(filtered.size()) + " words";
        info.category    = t.category;
        info.words       = std::move(filtered);
        info.completed   = false;
        m_topicLessons.append(info);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Rebuild both list widgets without triggering lesson-open signal
// ─────────────────────────────────────────────────────────────────────────────
void LessonsWidget::refreshListWidgets()
{
    // Guard: suppress currentRowChanged / itemActivated during rebuild
    m_rebuilding = true;

    auto fillList = [](QListWidget *list, const QVector<LessonInfo> &lessons) {
        list->clear();
        for (const LessonInfo &l : lessons) {
            QListWidgetItem *item = new QListWidgetItem;
            item->setSizeHint(QSize(0, 80));
            list->addItem(item);
            list->setItemWidget(item, makeLessonCard(l));
        }
    };

    fillList(m_dailyList, m_dailyLessons);
    fillList(m_topicList, m_topicLessons);

    m_rebuilding = false;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Lesson selection handlers
// ─────────────────────────────────────────────────────────────────────────────
void LessonsWidget::onDailyLessonClicked(int row)
{
    if (m_rebuilding) return;
    if (row < 0 || row >= m_dailyLessons.size()) return;
    m_currentLesson = m_dailyLessons[row];
    showLesson(m_currentLesson);
}

void LessonsWidget::onTopicLessonClicked(int row)
{
    if (m_rebuilding) return;
    if (row < 0 || row >= m_topicLessons.size()) return;
    m_currentLesson = m_topicLessons[row];
    showLesson(m_currentLesson);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Show lesson detail page
// ─────────────────────────────────────────────────────────────────────────────
void LessonsWidget::showLesson(const LessonInfo &lesson)
{
    m_lessonIcon->setText(lesson.icon);
    m_lessonTitle->setText(lesson.title);
    m_lessonDesc->setText(lesson.description);

    // Count learned words
    int learned = 0;
    for (const WordEntry &w : lesson.words)
        if (m_data->isWordLearned(w.id)) ++learned;

    m_progressBar->setRange(0, qMax(1, lesson.words.size()));
    m_progressBar->setValue(learned);
    m_progressLbl->setText(
        QString("%1 / %2 words learned").arg(learned).arg(lesson.words.size())
    );

    // Populate the word list — use plain text items (much faster than custom widgets)
    m_wordList->clear();
    for (const WordEntry &e : lesson.words) {
        bool isLearned = m_data->isWordLearned(e.id);

        // Build a lightweight custom widget per word
        QWidget *itemW = new QWidget;
        QHBoxLayout *iLay = new QHBoxLayout(itemW);
        iLay->setContentsMargins(10, 6, 10, 6);
        iLay->setSpacing(8);

        QLabel *statusLbl = new QLabel(isLearned ? "✅" : "⭕");
        statusLbl->setStyleSheet("font-size: 13px; background: transparent;");
        statusLbl->setFixedWidth(20);

        QVBoxLayout *tLay = new QVBoxLayout;
        tLay->setSpacing(1);

        QLabel *wLbl = new QLabel(e.word);
        wLbl->setStyleSheet("font-size: 13px; font-weight: 600; background: transparent;");

        QLabel *tLbl = new QLabel(e.translation);
        tLbl->setStyleSheet("font-size: 11px; color: #7c3aed; background: transparent;");

        tLay->addWidget(wLbl);
        tLay->addWidget(tLbl);

        // Difficulty badge
        auto diffColor = [](const QString &d) -> QString {
            if (d == "easy")   return "#10b981";
            if (d == "medium") return "#f59e0b";
            return "#ef4444";
        };
        QLabel *badge = new QLabel(e.difficulty.left(1).toUpper());
        badge->setStyleSheet(QString(
            "color: white; background: %1; border-radius: 5px;"
            " padding: 2px 6px; font-size: 9px; font-weight: 700;"
        ).arg(diffColor(e.difficulty)));

        iLay->addWidget(statusLbl);
        iLay->addLayout(tLay, 1);
        iLay->addWidget(badge);

        QListWidgetItem *item = new QListWidgetItem;
        item->setSizeHint(QSize(0, 56));
        m_wordList->addItem(item);
        m_wordList->setItemWidget(item, itemW);
    }

    m_detailStack->setCurrentIndex(0);

    bool done = lesson.completed;
    m_markCompleteBtn->setText(done ? "✅  Lesson Already Complete" : "✓  Mark Lesson as Complete");
    m_markCompleteBtn->setEnabled(!done);

    // Switch to lesson detail page
    m_mainStack->setCurrentIndex(1);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Word selection in the lesson detail
// ─────────────────────────────────────────────────────────────────────────────
void LessonsWidget::onWordSelected(int row)
{
    if (row < 0 || row >= m_currentLesson.words.size()) return;
    const WordEntry &e = m_currentLesson.words[row];
    m_detailWord->setText(e.word);
    m_detailTranslation->setText("→  " + e.translation);
    m_detailPronunciation->setText("🔊  " + e.pronunciation);
    m_detailExample->setText(e.example);
    m_detailStack->setCurrentIndex(1);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Mark current lesson as complete
// ─────────────────────────────────────────────────────────────────────────────
void LessonsWidget::onMarkComplete()
{
    m_data->markLessonComplete(QDate::currentDate());
    for (const WordEntry &e : m_currentLesson.words)
        m_data->markWordLearned(e.id);

    m_currentLesson.completed = true;
    m_markCompleteBtn->setText("✅  Lesson Complete");
    m_markCompleteBtn->setEnabled(false);

    // Update progress bar to full
    m_progressBar->setValue(m_currentLesson.words.size());
    m_progressLbl->setText(
        QString("%1 / %1 words learned").arg(m_currentLesson.words.size())
    );
}

// ─────────────────────────────────────────────────────────────────────────────
//  Back button — return to lesson browser
// ─────────────────────────────────────────────────────────────────────────────
void LessonsWidget::onBackToList()
{
    // Rebuild lesson data to reflect any completions made during this session
    buildDailyLesson();
    buildTopicLessons();
    refreshListWidgets();  // uses m_rebuilding guard — no spurious lesson open

    // Return to the browser page
    m_mainStack->setCurrentIndex(0);
}
