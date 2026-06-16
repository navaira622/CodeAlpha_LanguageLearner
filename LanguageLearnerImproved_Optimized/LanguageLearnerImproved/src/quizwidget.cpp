#include "quizwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedWidget>
#include <QComboBox>
#include <algorithm>
#include <random>

QuizWidget::QuizWidget(DataManager *data, QWidget *parent)
    : QWidget(parent), m_data(data)
{
    m_stack = new QStackedWidget(this);
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0,0,0,0);
    outerLayout->addWidget(m_stack);

    // ── Start page ────────────────────────────────────────────────────
    m_startPage = new QWidget;
    QVBoxLayout *startLayout = new QVBoxLayout(m_startPage);
    startLayout->setAlignment(Qt::AlignCenter);
    startLayout->setSpacing(20);
    startLayout->setContentsMargins(60, 60, 60, 60);

    QLabel *startIcon = new QLabel("✏️");
    startIcon->setStyleSheet("font-size: 64px;");
    startIcon->setAlignment(Qt::AlignCenter);

    QLabel *startTitle = new QLabel("Vocabulary Quiz");
    startTitle->setStyleSheet("font-size: 30px; font-weight: 700;");
    startTitle->setAlignment(Qt::AlignCenter);

    QLabel *startDesc = new QLabel(
        "Test your knowledge with 10 multiple-choice questions.\n"
        "Each question shows a word — choose the correct Spanish translation."
    );
    startDesc->setStyleSheet("font-size: 14px; color: #64748b; line-height: 1.6;");
    startDesc->setAlignment(Qt::AlignCenter);
    startDesc->setWordWrap(true);

    // Settings row
    QHBoxLayout *settingsRow = new QHBoxLayout;
    settingsRow->setAlignment(Qt::AlignCenter);
    settingsRow->setSpacing(12);
    QLabel *diffLbl = new QLabel("Difficulty:");
    diffLbl->setStyleSheet("font-size: 14px;");
    m_difficultyCombo = new QComboBox;
    m_difficultyCombo->addItems({"All Levels", "Easy Only", "Medium Only", "Hard Only"});
    m_difficultyCombo->setMinimumHeight(38);
    m_difficultyCombo->setMinimumWidth(160);
    settingsRow->addWidget(diffLbl);
    settingsRow->addWidget(m_difficultyCombo);

    QPushButton *startBtn = new QPushButton("▶  Start Quiz");
    startBtn->setObjectName("primaryBtn");
    startBtn->setMinimumSize(220, 54);
    startBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "  stop:0 #4f46e5, stop:1 #7c3aed);"
        " color: white; border: none;"
        " border-radius: 14px; font-size: 16px; font-weight: 700; padding: 12px 32px;"
        " border-bottom: 3px solid #be185d; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "  stop:0 #6366f1, stop:1 #8b5cf6); }"
    );
    connect(startBtn, &QPushButton::clicked, this, &QuizWidget::startQuiz);

    startLayout->addWidget(startIcon);
    startLayout->addWidget(startTitle);
    startLayout->addWidget(startDesc);
    startLayout->addLayout(settingsRow);
    startLayout->addSpacing(12);
    startLayout->addWidget(startBtn, 0, Qt::AlignHCenter);

    // ── Quiz page ─────────────────────────────────────────────────────
    m_quizPage = new QWidget;
    QVBoxLayout *quizLayout = new QVBoxLayout(m_quizPage);
    quizLayout->setContentsMargins(60, 40, 60, 40);
    quizLayout->setSpacing(16);

    // Progress row
    QHBoxLayout *progressRow = new QHBoxLayout;
    m_questionCounter = new QLabel;
    m_questionCounter->setStyleSheet("font-size: 14px; font-weight: 600; color: #64748b;");
    progressRow->addWidget(m_questionCounter);
    progressRow->addStretch();
    QLabel *quizTitle = new QLabel("✏️  Quiz");
    quizTitle->setStyleSheet("font-size: 14px; color: #64748b;");
    progressRow->addWidget(quizTitle);
    quizLayout->addLayout(progressRow);

    m_progressBar = new QProgressBar;
    m_progressBar->setRange(0, 10);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(8);
    quizLayout->addWidget(m_progressBar);

    quizLayout->addSpacing(8);

    // Question card
    QWidget *questionCard = new QWidget;
    questionCard->setObjectName("questionCard");
    questionCard->setStyleSheet(
        "QWidget#questionCard {"
        "  border-radius: 16px;"
        "  border: 1px solid #e2e8f0;"
        "}"
    );
    QVBoxLayout *cardLayout = new QVBoxLayout(questionCard);
    cardLayout->setContentsMargins(32, 28, 32, 28);
    cardLayout->setSpacing(8);

    QLabel *questionHdr = new QLabel("What is the Spanish translation of...");
    questionHdr->setStyleSheet("font-size: 12px; color: #94a3b8; font-weight: 600; letter-spacing: 1px; text-transform: uppercase;");

    m_questionLabel = new QLabel;
    m_questionLabel->setStyleSheet("font-size: 32px; font-weight: 700; color: #1e293b;");
    m_questionLabel->setAlignment(Qt::AlignCenter);
    m_questionLabel->setWordWrap(true);
    m_questionLabel->setMinimumHeight(80);

    cardLayout->addWidget(questionHdr, 0, Qt::AlignCenter);
    cardLayout->addWidget(m_questionLabel);
    quizLayout->addWidget(questionCard);

    quizLayout->addSpacing(8);

    // Answer buttons grid
    QGridLayout *answersGrid = new QGridLayout;
    answersGrid->setSpacing(12);
    QString letters[] = {"A", "B", "C", "D"};
    for (int i = 0; i < 4; ++i) {
        m_answerBtns[i] = new QPushButton;
        m_answerBtns[i]->setMinimumHeight(56);
        m_answerBtns[i]->setStyleSheet(
            "QPushButton {"
            "  border: 2px solid #e2e8f0; border-radius: 12px;"
            "  font-size: 14px; font-weight: 500; text-align: left; padding: 0 20px;"
            "}"
            "QPushButton:hover { border-color: #ec4899; }"
        );
        m_answerBtns[i]->setProperty("letterPrefix", letters[i]);
        const int idx = i;
        connect(m_answerBtns[i], &QPushButton::clicked,
                [this, idx]() { onAnswerSelected(idx); });
        answersGrid->addWidget(m_answerBtns[i], i / 2, i % 2);
    }
    quizLayout->addLayout(answersGrid);

    m_feedbackLabel = new QLabel;
    m_feedbackLabel->setAlignment(Qt::AlignCenter);
    m_feedbackLabel->setStyleSheet("font-size: 14px; font-weight: 600; min-height: 28px;");
    quizLayout->addWidget(m_feedbackLabel);

    m_nextBtn = new QPushButton("Next Question →");
    m_nextBtn->setObjectName("primaryBtn");
    m_nextBtn->setMinimumSize(200, 46);
    m_nextBtn->hide();
    connect(m_nextBtn, &QPushButton::clicked, this, &QuizWidget::nextQuestion);
    quizLayout->addWidget(m_nextBtn, 0, Qt::AlignHCenter);
    quizLayout->addStretch();

    // ── Results page ─────────────────────────────────────────────────
    m_resultsPage = new QWidget;
    QVBoxLayout *resLayout = new QVBoxLayout(m_resultsPage);
    resLayout->setAlignment(Qt::AlignCenter);
    resLayout->setSpacing(16);
    resLayout->setContentsMargins(60, 60, 60, 60);

    m_gradeLabel = new QLabel;
    m_gradeLabel->setStyleSheet("font-size: 72px;");
    m_gradeLabel->setAlignment(Qt::AlignCenter);

    QLabel *resultTitle = new QLabel("Quiz Complete!");
    resultTitle->setStyleSheet("font-size: 28px; font-weight: 700;");
    resultTitle->setAlignment(Qt::AlignCenter);

    m_scoreLabel = new QLabel;
    m_scoreLabel->setStyleSheet("font-size: 48px; font-weight: 700; color: #ec4899;");
    m_scoreLabel->setAlignment(Qt::AlignCenter);

    m_scoreDetail = new QLabel;
    m_scoreDetail->setStyleSheet("font-size: 15px; color: #64748b;");
    m_scoreDetail->setAlignment(Qt::AlignCenter);

    QProgressBar *resultBar = new QProgressBar;
    resultBar->setFixedHeight(12);
    resultBar->setTextVisible(false);
    resultBar->setObjectName("resultBar");

    m_retryBtn = new QPushButton("Try Again →");
    m_retryBtn->setObjectName("primaryBtn");
    m_retryBtn->setMinimumSize(200, 52);
    m_retryBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "  stop:0 #4f46e5, stop:1 #7c3aed);"
        " color: white; border: none;"
        " border-radius: 14px; font-size: 16px; font-weight: 700; padding: 12px 32px;"
        " border-bottom: 3px solid #be185d; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "  stop:0 #6366f1, stop:1 #8b5cf6); }"
    );
    connect(m_retryBtn, &QPushButton::clicked, [this]() {
        m_stack->setCurrentWidget(m_startPage);
    });

    resLayout->addWidget(m_gradeLabel);
    resLayout->addWidget(resultTitle);
    resLayout->addWidget(m_scoreLabel);
    resLayout->addWidget(resultBar);
    resLayout->addWidget(m_scoreDetail);
    resLayout->addSpacing(24);
    resLayout->addWidget(m_retryBtn, 0, Qt::AlignHCenter);

    m_stack->addWidget(m_startPage);
    m_stack->addWidget(m_quizPage);
    m_stack->addWidget(m_resultsPage);
    m_stack->setCurrentWidget(m_startPage);
}

void QuizWidget::startQuiz()
{
    buildQuestions();

    if (m_questions.isEmpty()) {
        m_questionCounter->setText("No questions available");
        m_questionLabel->setText("⚠️  Vocabulary data is empty or failed to load.\nPlease restart the app.");
        for (int i = 0; i < 4; ++i) {
            m_answerBtns[i]->setText("");
            m_answerBtns[i]->setEnabled(false);
        }
        m_progressBar->setValue(0);
        m_nextBtn->hide();
        m_feedbackLabel->clear();
        m_stack->setCurrentWidget(m_quizPage);
        return;
    }

    m_currentQ = 0;
    m_score    = 0;
    m_progressBar->setValue(0);
    m_nextBtn->hide();
    m_feedbackLabel->clear();
    showQuestion(0);
    m_stack->setCurrentWidget(m_quizPage);
}

void QuizWidget::buildQuestions()
{
    QString diff = m_difficultyCombo->currentText();
    QVector<WordEntry> pool;
    if (diff == "Easy Only")   pool = m_data->getWordsByDifficulty("easy");
    else if (diff == "Medium Only") pool = m_data->getWordsByDifficulty("medium");
    else if (diff == "Hard Only")   pool = m_data->getWordsByDifficulty("hard");
    else pool = m_data->getAllWords();

    if (pool.isEmpty()) pool = m_data->getAllWords();

    std::mt19937 rng(std::random_device{}());
    std::shuffle(pool.begin(), pool.end(), rng);

    int qCount = qMin(10, pool.size());
    m_questions.clear();
    m_progressBar->setMaximum(qCount);

    for (int i = 0; i < qCount; ++i) {
        QuizQuestion q;
        q.word = pool[i];

        // Build 4 choices (1 correct + 3 random distractors)
        QVector<QString> choices;
        choices.append(q.word.translation);

        QVector<WordEntry> distractors = pool;
        distractors.removeAt(i);
        std::shuffle(distractors.begin(), distractors.end(), rng);

        for (int j = 0; j < 3 && j < distractors.size(); ++j)
            choices.append(distractors[j].translation);

        std::shuffle(choices.begin(), choices.end(), rng);

        q.options = choices;
        q.correctIndex = choices.indexOf(q.word.translation);
        m_questions.append(q);
    }
}

void QuizWidget::showQuestion(int index)
{
    if (index >= m_questions.size()) {
        showResults();
        return;
    }

    const QuizQuestion &q = m_questions[index];
    m_questionCounter->setText(
        QString("Question %1 of %2").arg(index + 1).arg(m_questions.size())
    );
    m_questionLabel->setText(q.word.word);
    m_feedbackLabel->clear();
    m_answered = -1;
    m_nextBtn->hide();

    QString letters[] = {"A", "B", "C", "D"};
    for (int i = 0; i < 4 && i < q.options.size(); ++i) {
        m_answerBtns[i]->setText(
            QString("  %1.  %2").arg(letters[i], q.options[i])
        );
        m_answerBtns[i]->setEnabled(true);
        m_answerBtns[i]->setStyleSheet(
            "QPushButton {"
            "  border: 2px solid #e2e8f0; border-radius: 12px;"
            "  font-size: 14px; font-weight: 500; text-align: left; padding: 0 20px;"
            "}"
            "QPushButton:hover { border-color: #ec4899; }"
        );
    }
}

void QuizWidget::onAnswerSelected(int index)
{
    if (m_answered >= 0) return; // already answered
    m_answered = index;

    const QuizQuestion &q = m_questions[m_currentQ];
    bool correct = (index == q.correctIndex);
    if (correct) m_score++;

    // Style buttons
    for (int i = 0; i < 4; ++i) {
        m_answerBtns[i]->setEnabled(false);
        if (i == q.correctIndex) {
            m_answerBtns[i]->setStyleSheet(
                "QPushButton {"
                "  background: #d1fae5; color: #065f46;"
                "  border: 2px solid #10b981; border-radius: 12px;"
                "  font-size: 14px; font-weight: 600; text-align: left; padding: 0 20px;"
                "}"
            );
        } else if (i == index && !correct) {
            m_answerBtns[i]->setStyleSheet(
                "QPushButton {"
                "  background: #fee2e2; color: #991b1b;"
                "  border: 2px solid #ef4444; border-radius: 12px;"
                "  font-size: 14px; font-weight: 500; text-align: left; padding: 0 20px;"
                "}"
            );
        } else {
            m_answerBtns[i]->setStyleSheet(
                "QPushButton {"
                "  color: #94a3b8;"
                "  border: 2px solid #e2e8f0; border-radius: 12px;"
                "  font-size: 14px; text-align: left; padding: 0 20px;"
                "}"
            );
        }
    }

    if (correct) {
        m_feedbackLabel->setText("✅  Correct! Great job!");
        m_feedbackLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #10b981;");
    } else {
        m_feedbackLabel->setText(
            QString("❌  Incorrect. The answer was: %1").arg(q.word.translation)
        );
        m_feedbackLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #ef4444;");
    }

    m_progressBar->setValue(m_currentQ + 1);

    bool isLast = (m_currentQ == m_questions.size() - 1);
    m_nextBtn->setText(isLast ? "See Results 🏆" : "Next Question →");
    m_nextBtn->show();
}

void QuizWidget::nextQuestion()
{
    m_currentQ++;
    if (m_currentQ >= m_questions.size()) {
        showResults();
    } else {
        showQuestion(m_currentQ);
    }
}

void QuizWidget::showResults()
{
    int total = m_questions.size();
    m_data->saveQuizScore(m_score, total);
    emit quizFinished();

    int pct = total > 0 ? (m_score * 100 / total) : 0;

    QString grade;
    QString detail;
    if (pct >= 90) { grade = "🏆"; detail = "Outstanding performance!"; }
    else if (pct >= 70) { grade = "⭐"; detail = "Great job — keep it up!"; }
    else if (pct >= 50) { grade = "📚"; detail = "Good effort — review the missed words."; }
    else { grade = "💪"; detail = "Keep practicing — you're learning!"; }

    m_gradeLabel->setText(grade);
    m_scoreLabel->setText(QString("%1 / %2").arg(m_score).arg(total));
    m_scoreDetail->setText(
        QString("%1% correct  ·  %2").arg(pct).arg(detail)
    );

    // Update result bar
    QProgressBar *rb = m_resultsPage->findChild<QProgressBar*>("resultBar");
    if (rb) {
        rb->setRange(0, 100);
        rb->setValue(pct);
    }

    m_stack->setCurrentWidget(m_resultsPage);
}
