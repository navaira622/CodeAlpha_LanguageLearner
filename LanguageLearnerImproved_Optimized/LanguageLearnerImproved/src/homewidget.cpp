#include "homewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QEvent>
#include <QTime>
#include <QDate>

HomeWidget::HomeWidget(DataManager *data, QWidget *parent)
    : QWidget(parent), m_data(data)
{
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget *content = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(content);
    mainLayout->setContentsMargins(40, 36, 40, 40);
    mainLayout->setSpacing(28);

    // ── Hero ──────────────────────────────────────────────────────────
    QWidget *hero = new QWidget;
    hero->setObjectName("heroCard");
    hero->setStyleSheet(
        "QWidget#heroCard {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "    stop:0 #1e1b4b, stop:0.35 #4f46e5, stop:0.75 #7c3aed, stop:1 #ec4899);"
        "  border-radius: 20px;"
        "}"
    );
    QVBoxLayout *heroLayout = new QVBoxLayout(hero);
    heroLayout->setContentsMargins(40, 34, 40, 34);
    heroLayout->setSpacing(10);

    QDate today = QDate::currentDate();
    int hour = QTime::currentTime().hour();
    QString greeting = hour < 12 ? "¡Buenos días!" : hour < 18 ? "¡Buenas tardes!" : "¡Buenas noches!";

    QLabel *heroGreet = new QLabel(greeting);
    heroGreet->setStyleSheet("color: rgba(255,255,255,0.7); font-size: 15px; font-weight: 500; background: transparent;");

    QLabel *heroTitle = new QLabel("Ready to learn today?");
    heroTitle->setStyleSheet("color: white; font-size: 30px; font-weight: 800; background: transparent;");

    QLabel *heroSub = new QLabel(
        "English → Spanish  ·  " + QString::number(m_data->getTotalWords()) + " words  ·  "
        + today.toString("MMMM d, yyyy")
    );
    heroSub->setStyleSheet("color: rgba(255,255,255,0.8); font-size: 13px; background: transparent;");

    int streak = m_data->getCurrentStreak();
    QString motivation;
    if (streak == 0)        motivation = "✨  Start your first lesson today!";
    else if (streak < 3)    motivation = "🔥  " + QString::number(streak) + " day streak — keep going!";
    else if (streak < 7)    motivation = "🔥  " + QString::number(streak) + " days strong — fantastic!";
    else                    motivation = "🏆  " + QString::number(streak) + " day streak — you're unstoppable!";

    QLabel *motiveLbl = new QLabel(motivation);
    motiveLbl->setStyleSheet(
        "color: white; background: rgba(255,255,255,0.18); border-radius: 12px;"
        " font-size: 14px; font-weight: 600; padding: 10px 18px;"
    );
    motiveLbl->setFixedWidth(420);

    QHBoxLayout *heroButtons = new QHBoxLayout;
    heroButtons->setSpacing(12);

    auto heroBtn = [](const QString &text, const QString &bg, const QString &fg, int nav) -> QPushButton* {
        QPushButton *b = new QPushButton(text);
        b->setStyleSheet(QString(
            "QPushButton { background: %1; color: %2; border: none;"
            " border-radius: 12px; font-size: 14px; font-weight: 700;"
            " padding: 12px 26px; }"
            "QPushButton:hover { opacity: 0.9; }"
        ).arg(bg, fg));
        b->setCursor(Qt::PointingHandCursor);
        Q_UNUSED(nav);
        return b;
    };

    QPushButton *startLessonBtn = heroBtn("📅  Start Lesson", "white", "#4f46e5", 4);
    QPushButton *flashBtn = heroBtn("🃏  Flashcards", "rgba(255,255,255,0.2)", "white", 2);
    QPushButton *quizBtn  = heroBtn("✏️  Quiz Me",     "rgba(255,255,255,0.2)", "white", 3);
    heroButtons->addWidget(startLessonBtn);
    heroButtons->addWidget(flashBtn);
    heroButtons->addWidget(quizBtn);
    heroButtons->addStretch();

    connect(startLessonBtn, &QPushButton::clicked, [this]() { emit navigateTo(4); });
    connect(flashBtn,       &QPushButton::clicked, [this]() { emit navigateTo(2); });
    connect(quizBtn,        &QPushButton::clicked, [this]() { emit navigateTo(3); });

    heroLayout->addWidget(heroGreet);
    heroLayout->addWidget(heroTitle);
    heroLayout->addWidget(heroSub);
    heroLayout->addSpacing(8);
    heroLayout->addWidget(motiveLbl);
    heroLayout->addSpacing(6);
    heroLayout->addLayout(heroButtons);

    mainLayout->addWidget(hero);

    // ── Stats grid ────────────────────────────────────────────────────
    struct StatCard { QString icon; QString value; QString label; QString color; };
    int learned = m_data->getLearnedCount();
    int total   = m_data->getTotalWords();
    int pct     = total > 0 ? (learned * 100 / total) : 0;
    int lastScore = m_data->getLastQuizScore();
    int lastTotal = m_data->getLastQuizTotal();
    QString scoreStr = lastTotal > 0
        ? QString("%1/%2").arg(lastScore).arg(lastTotal)
        : "—";

    QVector<StatCard> stats = {
        {"📖", QString::number(learned), "Words Learned",   "#4f46e5"},
        {"📊", QString::number(pct)+"%",  "Vocabulary",      "#7c3aed"},
        {"🔥", QString::number(streak),  "Day Streak",      "#ec4899"},
        {"✏️", scoreStr,                  "Last Quiz Score", "#06b6d4"},
    };

    QGridLayout *statsGrid = new QGridLayout;
    statsGrid->setSpacing(16);

    for (int i = 0; i < stats.size(); ++i) {
        const auto &s = stats[i];
        QWidget *card = new QWidget;
        card->setStyleSheet(QString(
            "QWidget { background: white; border-radius: 16px;"
            " border: 1.5px solid #e0e7ff; }"
        ));
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(20, 18, 20, 18);
        cl->setSpacing(6);

        QLabel *icon = new QLabel(s.icon);
        icon->setStyleSheet("font-size: 28px; background: transparent; border: none;");

        QLabel *val = new QLabel(s.value);
        val->setStyleSheet(QString(
            "font-size: 32px; font-weight: 800; color: %1; background: transparent; border: none;"
        ).arg(s.color));

        QLabel *lbl = new QLabel(s.label);
        lbl->setStyleSheet("font-size: 12px; color: #64748b; background: transparent; border: none;");

        cl->addWidget(icon);
        cl->addWidget(val);
        cl->addWidget(lbl);

        statsGrid->addWidget(card, 0, i);
    }
    mainLayout->addLayout(statsGrid);

    // ── Quick actions ─────────────────────────────────────────────────
    QLabel *qaTitle = new QLabel("Quick Actions");
    qaTitle->setStyleSheet("font-size: 16px; font-weight: 700;");
    mainLayout->addWidget(qaTitle);

    struct ActionCard { QString icon; QString title; QString desc; QString color; int page; };
    QVector<ActionCard> actions = {
        {"📅", "Daily Lesson",  "Today's 10-word lesson",      "#4f46e5", 4},
        {"📖", "Vocabulary",    "Browse all " + QString::number(total) + " words", "#7c3aed", 1},
        {"🃏", "Flashcards",    "Study with flip cards",         "#ec4899", 2},
        {"✏️", "Quiz",          "Test your knowledge",           "#06b6d4", 3},
        {"📊", "Progress",      "Track your learning",           "#10b981", 5},
    };

    QHBoxLayout *actionsRow = new QHBoxLayout;
    actionsRow->setSpacing(14);

    for (const auto &a : actions) {
        QWidget *card = new QWidget;
        card->setStyleSheet(QString(
            "QWidget { background: white; border-radius: 16px;"
            " border: 1.5px solid #e2e8f0; }"
            "QWidget:hover { border-color: %1; background: #fafafe; }"
        ).arg(a.color));
        card->setCursor(Qt::PointingHandCursor);

        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(18, 16, 18, 16);
        cl->setSpacing(8);

        QLabel *iconLbl = new QLabel(a.icon);
        iconLbl->setStyleSheet(QString(
            "font-size: 28px; background: %1; border-radius: 10px;"
            " padding: 8px; qproperty-alignment: AlignCenter;"
        ).arg(a.color + "20"));
        iconLbl->setFixedSize(52, 52);
        iconLbl->setAlignment(Qt::AlignCenter);

        QLabel *titleLbl = new QLabel(a.title);
        titleLbl->setStyleSheet("font-size: 13px; font-weight: 700; background: transparent; border: none;");

        QLabel *descLbl = new QLabel(a.desc);
        descLbl->setStyleSheet("font-size: 11px; color: #64748b; background: transparent; border: none;");
        descLbl->setWordWrap(true);

        cl->addWidget(iconLbl);
        cl->addWidget(titleLbl);
        cl->addWidget(descLbl);
        cl->addStretch();

        int pg = a.page;
        connect(card, &QWidget::customContextMenuRequested, []{});
        card->installEventFilter(this);
        card->setProperty("navPage", pg);

        actionsRow->addWidget(card, 1);
    }
    mainLayout->addLayout(actionsRow);
    mainLayout->addStretch();

    scroll->setWidget(content);

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(scroll);
}

bool HomeWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QWidget *w = qobject_cast<QWidget*>(obj);
        if (w) {
            QVariant pg = w->property("navPage");
            if (pg.isValid()) {
                emit navigateTo(pg.toInt());
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}
