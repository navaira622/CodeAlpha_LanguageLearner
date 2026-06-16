#include "mainwindow.h"
#include "homewidget.h"
#include "vocabularywidget.h"
#include "flashcardwidget.h"
#include "quizwidget.h"
#include "lessonswidget.h"
#include "progresswidget.h"
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_data = new DataManager(this);
    m_darkMode = m_data->isDarkMode();
    setupUI();
    applyTheme();
    setWindowTitle("LinguaLearn — English · Spanish");
    resize(1160, 740);
    setMinimumSize(860, 600);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    QVBoxLayout *rootLayout = new QVBoxLayout(m_centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ── Navigation bar ─────────────────────────────────────────────────
    QWidget *navBar = new QWidget;
    navBar->setObjectName("navBar");
    navBar->setFixedHeight(62);

    QHBoxLayout *navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(20, 0, 20, 0);
    navLayout->setSpacing(2);

    QLabel *logo = new QLabel("🌐  LinguaLearn");
    logo->setObjectName("logoLabel");
    navLayout->addWidget(logo);
    navLayout->addSpacing(20);
    navLayout->addStretch();

    struct NavItem { QString icon; QString label; int index; };
    const QVector<NavItem> navItems = {
        {"🏠", "Home",       0},
        {"📖", "Vocabulary",  1},
        {"🃏", "Flashcards",  2},
        {"✏️", "Quiz",        3},
        {"📅", "Lessons",     4},
        {"📊", "Progress",    5},
    };

    auto makeNavBtn = [&](const NavItem &item) -> QPushButton* {
        QPushButton *btn = new QPushButton(item.icon + "  " + item.label);
        btn->setObjectName("navBtn");
        btn->setCheckable(true);
        btn->setFixedHeight(42);
        btn->setMinimumWidth(108);
        int idx = item.index;
        connect(btn, &QPushButton::clicked, [this, idx]() { navigateTo(idx); });
        navLayout->addWidget(btn);
        return btn;
    };

    m_btnHome     = makeNavBtn(navItems[0]);
    m_btnVocab    = makeNavBtn(navItems[1]);
    m_btnFlash    = makeNavBtn(navItems[2]);
    m_btnQuiz     = makeNavBtn(navItems[3]);
    m_btnLessons  = makeNavBtn(navItems[4]);
    m_btnProgress = makeNavBtn(navItems[5]);

    navLayout->addSpacing(12);

    m_btnDark = new QPushButton("🌙");
    m_btnDark->setObjectName("darkBtn");
    m_btnDark->setFixedSize(42, 42);
    m_btnDark->setToolTip("Toggle Dark Mode");
    connect(m_btnDark, &QPushButton::clicked, this, &MainWindow::toggleDarkMode);
    navLayout->addWidget(m_btnDark);

    m_btnHome->setChecked(true);

    // ── Page stack ─────────────────────────────────────────────────────
    m_stack = new QStackedWidget;

    m_home     = new HomeWidget(m_data, this);
    m_vocab    = new VocabularyWidget(m_data, this);
    m_flash    = new FlashcardWidget(m_data, this);
    m_quiz     = new QuizWidget(m_data, this);
    m_lessons  = new LessonsWidget(m_data, this);
    m_progress = new ProgressWidget(m_data, this);

    m_stack->addWidget(m_home);      // 0
    m_stack->addWidget(m_vocab);     // 1
    m_stack->addWidget(m_flash);     // 2
    m_stack->addWidget(m_quiz);      // 3
    m_stack->addWidget(m_lessons);   // 4
    m_stack->addWidget(m_progress);  // 5

    connect(m_home, &HomeWidget::navigateTo, this, &MainWindow::navigateTo);
    connect(m_quiz, &QuizWidget::quizFinished, m_progress, &ProgressWidget::refresh);

    rootLayout->addWidget(navBar);
    rootLayout->addWidget(m_stack, 1);
}

void MainWindow::navigateTo(int index)
{
    // Avoid redundant switch to the same page
    if (m_stack->currentIndex() == index) return;

    m_stack->setCurrentIndex(index);

    const QList<QPushButton*> btns = {
        m_btnHome, m_btnVocab, m_btnFlash,
        m_btnQuiz, m_btnLessons, m_btnProgress
    };
    for (int i = 0; i < btns.size(); ++i)
        btns[i]->setChecked(i == index);

    // Refresh only when actually switching to these pages
    if (index == 5) m_progress->refresh();
    if (index == 1) m_vocab->refresh();
}

void MainWindow::toggleDarkMode()
{
    m_darkMode = !m_darkMode;
    m_data->setDarkMode(m_darkMode);
    applyTheme();
}

void MainWindow::applyTheme()
{
    m_btnDark->setText(m_darkMode ? "☀️" : "🌙");

    if (m_darkMode) {
        qApp->setStyleSheet(R"(
            QMainWindow, QWidget {
                background-color: #0f172a;
                color: #e2e8f0;
                font-family: "Segoe UI", "Helvetica Neue", Arial, sans-serif;
            }
            QWidget#navBar {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #0f172a, stop:0.5 #1e1b4b, stop:1 #1a0e2e);
                border-bottom: 2px solid #312e81;
            }
            QLabel#logoLabel {
                font-size: 18px; font-weight: 800; color: #a5b4fc;
                letter-spacing: 1px; background: transparent;
            }
            QPushButton#navBtn {
                background: transparent; color: #94a3b8;
                border: none; border-radius: 10px;
                font-size: 13px; font-weight: 500;
                padding: 6px 14px;
            }
            QPushButton#navBtn:hover {
                background: rgba(99,102,241,0.15); color: #a5b4fc;
            }
            QPushButton#navBtn:checked {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #4f46e5, stop:1 #7c3aed);
                color: #ffffff; font-weight: 700;
                border-bottom: 2px solid #6366f1;
            }
            QPushButton#darkBtn {
                background: #1e1b4b; border: 1px solid #312e81;
                border-radius: 10px; font-size: 17px; color: #e2e8f0;
            }
            QPushButton#darkBtn:hover { background: #312e81; }

            QPushButton#primaryBtn {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #4f46e5, stop:1 #7c3aed);
                color: white; border: none; border-radius: 10px;
                font-size: 14px; font-weight: 700; padding: 10px 24px;
                border-bottom: 2px solid #3730a3;
            }
            QPushButton#primaryBtn:hover {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #6366f1, stop:1 #8b5cf6);
            }
            QPushButton#primaryBtn:pressed { background: #3730a3; border-bottom: none; }

            QPushButton#secondaryBtn {
                background: #1e1b4b; color: #a5b4fc;
                border: 1px solid #312e81; border-radius: 10px;
                font-size: 14px; padding: 8px 20px; font-weight: 600;
            }
            QPushButton#secondaryBtn:hover { background: #312e81; color: #c7d2fe; }

            QLineEdit {
                background: #1e293b; border: 1.5px solid #334155;
                border-radius: 8px; padding: 8px 12px;
                color: #e2e8f0; font-size: 13px;
            }
            QLineEdit:focus { border: 2px solid #6366f1; background: #1e1b4b; }

            QListWidget, QTableWidget {
                background: #1e293b; border: 1px solid #334155;
                border-radius: 12px; color: #e2e8f0;
                gridline-color: #1e293b;
                alternate-background-color: #1a2035;
                outline: none;
            }
            QListWidget::item { padding: 8px; border-bottom: 1px solid #1e293b; border-radius: 6px; }
            QTableWidget::item { padding: 8px; }
            QListWidget::item:selected, QTableWidget::item:selected {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #4f46e5, stop:1 #7c3aed);
                color: white; border-radius: 6px;
            }
            QListWidget::item:hover, QTableWidget::item:hover {
                background: rgba(99,102,241,0.12);
            }
            QHeaderView::section {
                background: #1e293b; color: #818cf8;
                border: none; border-bottom: 2px solid #334155;
                padding: 10px 12px;
                font-weight: 700; font-size: 11px; letter-spacing: 1px;
            }
            QScrollBar:vertical {
                background: #1e293b; width: 7px; border-radius: 4px;
            }
            QScrollBar::handle:vertical {
                background: #475569; border-radius: 4px; min-height: 30px;
            }
            QScrollBar::handle:vertical:hover { background: #6366f1; }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
            QProgressBar {
                background: #1e293b; border: none; border-radius: 6px;
                text-align: center; color: white; font-size: 11px;
            }
            QProgressBar::chunk {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #4f46e5, stop:1 #7c3aed);
                border-radius: 6px;
            }
            QComboBox {
                background: #1e293b; border: 1.5px solid #334155;
                border-radius: 8px; padding: 6px 12px; color: #e2e8f0;
                min-height: 32px;
            }
            QComboBox:hover { border-color: #6366f1; }
            QComboBox::drop-down { border: none; width: 24px; }
            QComboBox::down-arrow { image: none; }
            QComboBox QAbstractItemView {
                background: #1e293b; color: #e2e8f0;
                border: 1px solid #334155; border-radius: 8px;
                selection-background-color: #4f46e5;
            }
            QSplitter::handle { background: #334155; }
            QFrame[frameShape="4"], QFrame[frameShape="5"] { color: #334155; }
            QTabWidget::pane { border: none; }
            QTabBar::tab {
                font-size: 13px; font-weight: 600; padding: 10px 20px;
                background: transparent; color: #64748b; border: none;
                border-radius: 8px; margin: 2px;
            }
            QTabBar::tab:selected {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #4f46e5,stop:1 #7c3aed);
                color: white;
            }
        )");
    } else {
        qApp->setStyleSheet(R"(
            QMainWindow, QWidget {
                background-color: #f8fafc;
                color: #1e293b;
                font-family: "Segoe UI", "Helvetica Neue", Arial, sans-serif;
            }
            QWidget#navBar {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #ffffff, stop:0.6 #fafafe, stop:1 #f0f4ff);
                border-bottom: 2px solid #e0e7ff;
            }
            QLabel#logoLabel {
                font-size: 18px; font-weight: 800;
                background: transparent;
                color: #4f46e5;
                letter-spacing: 1px;
            }
            QPushButton#navBtn {
                background: transparent; color: #64748b;
                border: none; border-radius: 10px;
                font-size: 13px; font-weight: 500;
                padding: 6px 14px;
            }
            QPushButton#navBtn:hover {
                background: #eef2ff; color: #4f46e5;
            }
            QPushButton#navBtn:checked {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #4f46e5, stop:1 #7c3aed);
                color: #ffffff; font-weight: 700;
                border-bottom: 2px solid #3730a3;
            }
            QPushButton#darkBtn {
                background: #eef2ff; border: 1px solid #e0e7ff;
                border-radius: 10px; font-size: 17px;
            }
            QPushButton#darkBtn:hover { background: #e0e7ff; border-color: #818cf8; }

            QPushButton#primaryBtn {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #4f46e5, stop:1 #7c3aed);
                color: white; border: none; border-radius: 10px;
                font-size: 14px; font-weight: 700; padding: 10px 24px;
                border-bottom: 2px solid #3730a3;
            }
            QPushButton#primaryBtn:hover {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #6366f1, stop:1 #8b5cf6);
            }
            QPushButton#primaryBtn:pressed { background: #3730a3; border-bottom: none; }
            QPushButton#primaryBtn:disabled {
                background: #d1fae5; color: #065f46; border-bottom: none;
            }

            QPushButton#secondaryBtn {
                background: white; color: #4f46e5;
                border: 1.5px solid #c7d2fe; border-radius: 10px;
                font-size: 14px; padding: 8px 20px; font-weight: 600;
            }
            QPushButton#secondaryBtn:hover {
                background: #eef2ff; border-color: #818cf8; color: #4338ca;
            }

            QLineEdit {
                background: white; border: 1.5px solid #e2e8f0;
                border-radius: 8px; padding: 8px 12px;
                color: #1e293b; font-size: 13px;
            }
            QLineEdit:focus { border: 2px solid #6366f1; background: #fafafe; }

            QListWidget, QTableWidget {
                background: white; border: 1px solid #e2e8f0;
                border-radius: 12px; color: #1e293b;
                gridline-color: #f1f5f9;
                alternate-background-color: #f8fafc;
                outline: none;
            }
            QListWidget::item {
                padding: 8px; border-bottom: 1px solid #f1f5f9; border-radius: 6px;
            }
            QTableWidget::item { padding: 8px; }
            QListWidget::item:selected, QTableWidget::item:selected {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #4f46e5, stop:1 #7c3aed);
                color: white; border-radius: 6px;
            }
            QListWidget::item:hover, QTableWidget::item:hover {
                background: #eef2ff;
            }
            QHeaderView::section {
                background: #f8fafc; color: #6366f1;
                border: none; border-bottom: 2px solid #e0e7ff;
                padding: 10px 12px;
                font-weight: 700; font-size: 11px; letter-spacing: 1px;
            }
            QScrollBar:vertical {
                background: #f1f5f9; width: 7px; border-radius: 4px;
            }
            QScrollBar::handle:vertical {
                background: #cbd5e1; border-radius: 4px; min-height: 30px;
            }
            QScrollBar::handle:vertical:hover { background: #6366f1; }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
            QProgressBar {
                background: #e0e7ff; border: none; border-radius: 6px;
                text-align: center; color: white; font-size: 11px;
            }
            QProgressBar::chunk {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #4f46e5, stop:1 #7c3aed);
                border-radius: 6px;
            }
            QComboBox {
                background: white; border: 1.5px solid #e2e8f0;
                border-radius: 8px; padding: 6px 12px; color: #1e293b;
                min-height: 32px;
            }
            QComboBox:hover { border-color: #6366f1; }
            QComboBox::drop-down { border: none; width: 24px; }
            QComboBox::down-arrow { image: none; }
            QComboBox QAbstractItemView {
                background: white; color: #1e293b;
                border: 1px solid #e0e7ff; border-radius: 8px;
                selection-background-color: #4f46e5; selection-color: white;
            }
            QSplitter::handle { background: #e2e8f0; }
            QFrame[frameShape="4"], QFrame[frameShape="5"] { color: #e2e8f0; }
            QTabWidget::pane { border: none; }
            QTabBar::tab {
                font-size: 13px; font-weight: 600; padding: 10px 20px;
                background: transparent; color: #64748b; border: none;
                border-radius: 8px; margin: 2px;
            }
            QTabBar::tab:selected {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #4f46e5,stop:1 #7c3aed);
                color: white;
            }
            QTabBar::tab:hover:!selected { background: #eef2ff; color: #4f46e5; }
        )");
    }
}
