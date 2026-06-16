#include "flashcardwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QFrame>
#include <QEvent>
#include <algorithm>
#include <random>

FlashcardWidget::FlashcardWidget(DataManager *data, QWidget *parent)
    : QWidget(parent), m_data(data)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    // ── Header ────────────────────────────────────────────────────────
    QHBoxLayout *headerRow = new QHBoxLayout;
    QLabel *title = new QLabel("🃏  Flashcards");
    title->setStyleSheet("font-size: 22px; font-weight: 700;");
    headerRow->addWidget(title);
    headerRow->addStretch();

    // Filter
    QLabel *filterLbl = new QLabel("Difficulty:");
    filterLbl->setStyleSheet("color: #64748b; font-size: 13px;");
    headerRow->addWidget(filterLbl);

    m_filterCombo = new QComboBox;
    m_filterCombo->addItems({"All", "Easy", "Medium", "Hard"});
    m_filterCombo->setMinimumHeight(36);
    m_filterCombo->setMinimumWidth(120);
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FlashcardWidget::onFilterChanged);
    headerRow->addWidget(m_filterCombo);

    m_btnShuffle = new QPushButton("🔀  Shuffle");
    m_btnShuffle->setObjectName("secondaryBtn");
    m_btnShuffle->setMinimumHeight(36);
    connect(m_btnShuffle, &QPushButton::clicked, this, &FlashcardWidget::shuffleCards);
    headerRow->addWidget(m_btnShuffle);

    mainLayout->addLayout(headerRow);

    // ── Counter ───────────────────────────────────────────────────────
    m_counterLabel = new QLabel;
    m_counterLabel->setAlignment(Qt::AlignCenter);
    m_counterLabel->setStyleSheet("font-size: 14px; color: #64748b;");
    mainLayout->addWidget(m_counterLabel);

    // ── Card area ─────────────────────────────────────────────────────
    m_cardStack = new QStackedWidget;

    // Front card
    m_frontCard = new QWidget;
    m_frontCard->setObjectName("flashCard");
    m_frontCard->setStyleSheet(
        "QWidget#flashCard {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "    stop:0 #312e81, stop:0.5 #4f46e5, stop:1 #7c3aed);"
        "  border-radius: 24px;"
        "}"
    );
    m_frontCard->setCursor(Qt::PointingHandCursor);
    m_frontCard->installEventFilter(this);

    QVBoxLayout *frontLayout = new QVBoxLayout(m_frontCard);
    frontLayout->setContentsMargins(60, 60, 60, 60);
    frontLayout->setAlignment(Qt::AlignCenter);

    QLabel *frontLabel = new QLabel("ENGLISH");
    frontLabel->setStyleSheet(
        "color: rgba(255,255,255,0.5); font-size: 11px; font-weight: 700;"
        " letter-spacing: 3px; background: transparent;"
    );
    frontLabel->setAlignment(Qt::AlignCenter);

    m_frontWord = new QLabel("Word");
    m_frontWord->setStyleSheet(
        "color: white; font-size: 42px; font-weight: 700; background: transparent;"
    );
    m_frontWord->setAlignment(Qt::AlignCenter);
    m_frontWord->setWordWrap(true);

    m_frontHint = new QLabel("🔊  tap to flip");
    m_frontHint->setStyleSheet(
        "color: rgba(255,255,255,0.4); font-size: 13px; background: transparent;"
    );
    m_frontHint->setAlignment(Qt::AlignCenter);

    frontLayout->addStretch();
    frontLayout->addWidget(frontLabel);
    frontLayout->addSpacing(12);
    frontLayout->addWidget(m_frontWord);
    frontLayout->addStretch();
    frontLayout->addWidget(m_frontHint);

    // Back card
    m_backCard = new QWidget;
    m_backCard->setObjectName("flashCardBack");
    m_backCard->setStyleSheet(
        "QWidget#flashCardBack {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "    stop:0 #581c3e, stop:0.5 #831843, stop:1 #3a1530);"
        "  border-radius: 24px;"
        "}"
    );
    m_backCard->setCursor(Qt::PointingHandCursor);
    m_backCard->installEventFilter(this);

    QVBoxLayout *backLayout = new QVBoxLayout(m_backCard);
    backLayout->setContentsMargins(60, 40, 60, 40);
    backLayout->setAlignment(Qt::AlignCenter);

    QLabel *backLangLabel = new QLabel("SPANISH");
    backLangLabel->setStyleSheet(
        "color: rgba(255,255,255,0.4); font-size: 11px; font-weight: 700;"
        " letter-spacing: 3px; background: transparent;"
    );
    backLangLabel->setAlignment(Qt::AlignCenter);

    m_backWord = new QLabel;
    m_backWord->setStyleSheet(
        "color: #f9a8d4; font-size: 14px; font-weight: 600; background: transparent;"
    );
    m_backWord->setAlignment(Qt::AlignCenter);

    m_backTranslation = new QLabel;
    m_backTranslation->setStyleSheet(
        "color: white; font-size: 38px; font-weight: 700; background: transparent;"
    );
    m_backTranslation->setAlignment(Qt::AlignCenter);
    m_backTranslation->setWordWrap(true);

    QFrame *sepLine = new QFrame(m_backCard);
    sepLine->setFrameShape(QFrame::HLine);
    sepLine->setStyleSheet("color: rgba(255,255,255,0.1);");

    QLabel *exampleLbl = new QLabel("EXAMPLE");
    exampleLbl->setStyleSheet(
        "color: rgba(255,255,255,0.4); font-size: 11px; font-weight: 700;"
        " letter-spacing: 2px; background: transparent;"
    );
    exampleLbl->setAlignment(Qt::AlignCenter);

    m_backExample = new QLabel;
    m_backExample->setStyleSheet(
        "color: rgba(255,255,255,0.7); font-size: 14px; font-style: italic;"
        " line-height: 1.5; background: transparent;"
    );
    m_backExample->setAlignment(Qt::AlignCenter);
    m_backExample->setWordWrap(true);

    QLabel *backHint = new QLabel("tap to flip back");
    backHint->setStyleSheet(
        "color: rgba(255,255,255,0.25); font-size: 12px; background: transparent;"
    );
    backHint->setAlignment(Qt::AlignCenter);

    backLayout->addStretch();
    backLayout->addWidget(backLangLabel);
    backLayout->addSpacing(4);
    backLayout->addWidget(m_backWord);
    backLayout->addSpacing(8);
    backLayout->addWidget(m_backTranslation);
    backLayout->addSpacing(20);
    backLayout->addWidget(sepLine);
    backLayout->addSpacing(12);
    backLayout->addWidget(exampleLbl);
    backLayout->addSpacing(6);
    backLayout->addWidget(m_backExample);
    backLayout->addStretch();
    backLayout->addWidget(backHint);

    m_cardStack->addWidget(m_frontCard);
    m_cardStack->addWidget(m_backCard);
    m_cardStack->setMinimumHeight(340);
    mainLayout->addWidget(m_cardStack, 1);

    // ── Navigation ────────────────────────────────────────────────────
    QHBoxLayout *navRow = new QHBoxLayout;
    navRow->setSpacing(16);

    m_btnPrev = new QPushButton("← Previous");
    m_btnPrev->setObjectName("secondaryBtn");
    m_btnPrev->setMinimumSize(140, 46);
    connect(m_btnPrev, &QPushButton::clicked, this, &FlashcardWidget::prevCard);
    navRow->addWidget(m_btnPrev);

    m_btnFlip = new QPushButton("🔄  Flip Card");
    m_btnFlip->setObjectName("primaryBtn");
    m_btnFlip->setMinimumSize(180, 46);
    connect(m_btnFlip, &QPushButton::clicked, [this]() {
        m_showingBack ? showFront() : showBack();
    });
    navRow->addWidget(m_btnFlip);

    m_btnNext = new QPushButton("Next →");
    m_btnNext->setObjectName("secondaryBtn");
    m_btnNext->setMinimumSize(140, 46);
    connect(m_btnNext, &QPushButton::clicked, this, &FlashcardWidget::nextCard);
    navRow->addWidget(m_btnNext);

    mainLayout->addLayout(navRow);

    onFilterChanged();
}

bool FlashcardWidget::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::MouseButtonPress) {
        if (obj == m_frontCard) showBack();
        else if (obj == m_backCard) showFront();
    }
    return QWidget::eventFilter(obj, ev);
}

void FlashcardWidget::onFilterChanged()
{
    QString filter = m_filterCombo->currentText();
    if (filter == "All")
        m_cards = m_data->getAllWords();
    else
        m_cards = m_data->getWordsByDifficulty(filter.toLower());

    m_currentIndex = 0;
    m_showingBack = false;
    updateCard();
}

void FlashcardWidget::shuffleCards()
{
    std::mt19937 rng(std::random_device{}());
    std::shuffle(m_cards.begin(), m_cards.end(), rng);
    m_currentIndex = 0;
    showFront();
}

void FlashcardWidget::showFront()
{
    m_showingBack = false;
    m_cardStack->setCurrentIndex(0);
    m_btnFlip->setText("🔄  Flip Card");
}

void FlashcardWidget::showBack()
{
    m_showingBack = true;
    m_cardStack->setCurrentIndex(1);
    m_btnFlip->setText("🔄  Show Word");
}

void FlashcardWidget::nextCard()
{
    if (m_cards.isEmpty()) return;
    m_currentIndex = (m_currentIndex + 1) % m_cards.size();
    showFront();
    updateCard();
}

void FlashcardWidget::prevCard()
{
    if (m_cards.isEmpty()) return;
    m_currentIndex = (m_currentIndex - 1 + m_cards.size()) % m_cards.size();
    showFront();
    updateCard();
}

void FlashcardWidget::updateCard()
{
    if (m_cards.isEmpty()) {
        m_frontWord->setText("No vocabulary loaded");
        m_frontHint->setText("⚠️  Check that vocabulary.json is available, then restart the app.");
        m_backWord->clear();
        m_backTranslation->setText("No data");
        m_backExample->setText("Vocabulary could not be loaded.");
        m_counterLabel->setText("0 / 0");
        updateNavButtons();
        return;
    }

    const WordEntry &e = m_cards[m_currentIndex];
    m_frontWord->setText(e.word);
    m_frontHint->setText("🔊  " + e.pronunciation + "  ·  tap to flip");
    m_backWord->setText(e.word);
    m_backTranslation->setText(e.translation);
    m_backExample->setText(e.example);

    m_counterLabel->setText(
        QString("Card %1 of %2").arg(m_currentIndex + 1).arg(m_cards.size())
    );
    updateNavButtons();
}

void FlashcardWidget::updateNavButtons()
{
    m_btnPrev->setEnabled(!m_cards.isEmpty());
    m_btnNext->setEnabled(!m_cards.isEmpty());
}
