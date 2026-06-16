#include "progresswidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QScrollArea>
#include <QFrame>
#include <QFont>

ProgressWidget::ProgressWidget(DataManager *data, QWidget *parent)
    : QWidget(parent), m_data(data)
{
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget *content = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(content);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(24);

    // ── Header ────────────────────────────────────────────────────────
    QLabel *title = new QLabel("📊  Progress");
    title->setStyleSheet("font-size: 22px; font-weight: 700;");
    mainLayout->addWidget(title);

    // ── Vocabulary progress ────────────────────────────────────────────
    QWidget *vocabCard = new QWidget;
    vocabCard->setObjectName("progressCard");
    vocabCard->setStyleSheet(
        "QWidget#progressCard { border-radius: 16px;"
        " border: 1px solid #e2e8f0; }"
    );
    QVBoxLayout *vcLayout = new QVBoxLayout(vocabCard);
    vcLayout->setContentsMargins(24, 20, 24, 20);
    vcLayout->setSpacing(12);

    QLabel *vcTitle = new QLabel("Vocabulary Learned");
    vcTitle->setStyleSheet("font-size: 16px; font-weight: 700;");

    QHBoxLayout *vcRow = new QHBoxLayout;
    m_learnedLabel = new QLabel;
    m_learnedLabel->setStyleSheet("font-size: 38px; font-weight: 800; color: #4f46e5;");
    m_learnedPct = new QLabel;
    m_learnedPct->setStyleSheet("font-size: 14px; color: #64748b;");
    vcRow->addWidget(m_learnedLabel);
    vcRow->addSpacing(8);
    vcRow->addWidget(m_learnedPct, 0, Qt::AlignBottom);
    vcRow->addStretch();

    m_learnedBar = new QProgressBar;
    m_learnedBar->setFixedHeight(16);
    m_learnedBar->setTextVisible(false);
    m_learnedBar->setStyleSheet(
        "QProgressBar { background: #fce7f3; border: none; border-radius: 8px; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "  stop:0 #ec4899, stop:1 #fb7185); border-radius: 8px; }"
    );

    vcLayout->addWidget(vcTitle);
    vcLayout->addLayout(vcRow);
    vcLayout->addWidget(m_learnedBar);
    mainLayout->addWidget(vocabCard);

    // ── Stats grid ─────────────────────────────────────────────────────
    QGridLayout *statsGrid = new QGridLayout;
    statsGrid->setSpacing(16);

    auto makeStatCard = [&](const QString &icon, const QString &label,
                             QLabel *&valLabel, const QString &color) -> QWidget* {
        QWidget *card = new QWidget;
        card->setObjectName("statCard");
        card->setStyleSheet(
            "QWidget#statCard { border-radius: 14px;"
            " border: 1px solid #e2e8f0; }"
        );
        card->setMinimumHeight(110);
        QVBoxLayout *l = new QVBoxLayout(card);
        l->setContentsMargins(20, 16, 20, 16);
        l->setSpacing(6);

        QLabel *iconLbl = new QLabel(icon);
        iconLbl->setStyleSheet(QString(
            "font-size: 26px; background: %1; border-radius: 10px;"
            " padding: 4px 8px; color: white;"
        ).arg(color));
        iconLbl->setFixedSize(44, 44);
        iconLbl->setAlignment(Qt::AlignCenter);

        valLabel = new QLabel("0");
        valLabel->setStyleSheet("font-size: 28px; font-weight: 700;");

        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet("font-size: 12px; color: #64748b;");

        l->addWidget(iconLbl);
        l->addWidget(valLabel);
        l->addWidget(lbl);
        return card;
    };

    statsGrid->addWidget(makeStatCard("🔥", "Day Streak",
                                      m_streakLabel, "#ec4899"), 0, 0);
    statsGrid->addWidget(makeStatCard("📅", "Lessons Completed",
                                      m_lessonsLabel, "#a855f7"), 0, 1);
    statsGrid->addWidget(makeStatCard("✏️", "Quizzes Taken",
                                      m_quizLabel, "#f59e0b"), 0, 2);
    statsGrid->addWidget(makeStatCard("🏆", "Best Quiz Score",
                                      m_bestQuizLabel, "#ef4444"), 0, 3);

    mainLayout->addLayout(statsGrid);

    // ── Quiz history ───────────────────────────────────────────────────
    QWidget *histCard = new QWidget;
    histCard->setObjectName("histCard");
    histCard->setStyleSheet(
        "QWidget#histCard { border-radius: 16px;"
        " border: 1px solid #e2e8f0; }"
    );
    QVBoxLayout *histLayout = new QVBoxLayout(histCard);
    histLayout->setContentsMargins(24, 20, 24, 20);
    histLayout->setSpacing(12);

    QLabel *histTitle = new QLabel("Quiz History");
    histTitle->setStyleSheet("font-size: 16px; font-weight: 700;");
    histLayout->addWidget(histTitle);

    m_historyTable = new QTableWidget;
    m_historyTable->setColumnCount(4);
    m_historyTable->setHorizontalHeaderLabels({"Date", "Score", "Total", "Percentage"});
    m_historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_historyTable->verticalHeader()->setVisible(false);
    m_historyTable->setShowGrid(false);
    m_historyTable->setMinimumHeight(180);
    histLayout->addWidget(m_historyTable);

    mainLayout->addWidget(histCard);
    mainLayout->addStretch();

    scroll->setWidget(content);

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(scroll);

    refresh();
}

void ProgressWidget::refresh()
{
    int learned = m_data->getLearnedCount();
    int total   = m_data->getTotalWords();
    int pct     = total > 0 ? (learned * 100 / total) : 0;

    m_learnedLabel->setText(QString("%1 / %2").arg(learned).arg(total));
    m_learnedPct->setText(QString("(%1% complete)").arg(pct));
    m_learnedBar->setRange(0, qMax(1, total));
    m_learnedBar->setValue(learned);

    m_streakLabel->setText(QString::number(m_data->getCurrentStreak()));
    m_lessonsLabel->setText(QString::number(m_data->getCompletedLessonsCount()));
    m_quizLabel->setText(QString::number(m_data->getQuizHistory().size()));

    // Best quiz
    int best = 0;
    for (const auto &entry : m_data->getQuizHistory()) {
        int t = entry.second.second;
        if (t > 0) best = qMax(best, entry.second.first * 100 / t);
    }
    m_bestQuizLabel->setText(QString::number(best) + "%");

    // History table
    auto history = m_data->getQuizHistory();
    m_historyTable->setRowCount(history.size());

    for (int i = 0; i < history.size(); ++i) {
        const auto &entry = history[history.size() - 1 - i]; // newest first
        int s = entry.second.first;
        int t = entry.second.second;
        int p = t > 0 ? (s * 100 / t) : 0;

        QTableWidgetItem *dateItem = new QTableWidgetItem(entry.first.toString("MMM d, yyyy"));
        m_historyTable->setItem(i, 0, dateItem);

        QTableWidgetItem *scoreItem = new QTableWidgetItem(QString::number(s));
        scoreItem->setTextAlignment(Qt::AlignCenter);
        m_historyTable->setItem(i, 1, scoreItem);

        QTableWidgetItem *totalItem = new QTableWidgetItem(QString::number(t));
        totalItem->setTextAlignment(Qt::AlignCenter);
        m_historyTable->setItem(i, 2, totalItem);

        QTableWidgetItem *pctItem = new QTableWidgetItem(QString("%1%").arg(p));
        pctItem->setTextAlignment(Qt::AlignCenter);
        if (p >= 80) pctItem->setForeground(QColor("#10b981"));
        else if (p >= 60) pctItem->setForeground(QColor("#f59e0b"));
        else pctItem->setForeground(QColor("#ef4444"));
        pctItem->setFont(QFont("", -1, QFont::Bold));
        m_historyTable->setItem(i, 3, pctItem);

        m_historyTable->setRowHeight(i, 44);
    }

    if (history.isEmpty()) {
        m_historyTable->setRowCount(1);
        QTableWidgetItem *emptyItem = new QTableWidgetItem("No quiz history yet — take your first quiz!");
        emptyItem->setForeground(QColor("#94a3b8"));
        m_historyTable->setItem(0, 0, emptyItem);
        m_historyTable->setSpan(0, 0, 1, 4);
    }
}
