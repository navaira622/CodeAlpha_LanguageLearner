#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "datamanager.h"

class HomeWidget;
class VocabularyWidget;
class FlashcardWidget;
class QuizWidget;
class LessonsWidget;
class ProgressWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void navigateTo(int index);
    void toggleDarkMode();

private:
    void setupUI();
    void applyTheme();

    QWidget        *m_centralWidget;
    QStackedWidget *m_stack;
    DataManager    *m_data;

    HomeWidget       *m_home;
    VocabularyWidget *m_vocab;
    FlashcardWidget  *m_flash;
    QuizWidget       *m_quiz;
    LessonsWidget    *m_lessons;
    ProgressWidget   *m_progress;

    // Nav bar buttons
    QPushButton *m_btnHome;
    QPushButton *m_btnVocab;
    QPushButton *m_btnFlash;
    QPushButton *m_btnQuiz;
    QPushButton *m_btnLessons;
    QPushButton *m_btnProgress;
    QPushButton *m_btnDark;

    bool m_darkMode = false;
};

#endif // MAINWINDOW_H
