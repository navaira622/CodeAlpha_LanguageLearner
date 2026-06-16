#ifndef QUIZWIDGET_H
#define QUIZWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QStackedWidget>
#include <QComboBox>
#include <QVector>
#include "datamanager.h"

struct QuizQuestion {
    WordEntry word;
    QVector<QString> options; // 4 choices
    int correctIndex;
};

class QuizWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QuizWidget(DataManager *data, QWidget *parent = nullptr);

signals:
    void quizFinished();

private slots:
    void startQuiz();
    void onAnswerSelected(int index);
    void nextQuestion();
    void showResults();

private:
    void buildQuestions();
    void showQuestion(int index);
    void setAnswerButton(int btnIndex, const QString &text, bool correct,
                         bool wrong, bool neutral);

    DataManager  *m_data;
    QVector<QuizQuestion> m_questions;
    int m_currentQ  = 0;
    int m_score     = 0;
    int m_answered  = -1; // which button was pressed

    // Pages
    QWidget *m_startPage;
    QWidget *m_quizPage;
    QWidget *m_resultsPage;

    QStackedWidget *m_stack;

    // Quiz page widgets
    QProgressBar *m_progressBar;
    QLabel       *m_questionCounter;
    QLabel       *m_questionLabel;
    QPushButton  *m_answerBtns[4];
    QPushButton  *m_nextBtn;
    QLabel       *m_feedbackLabel;

    // Results page widgets
    QLabel       *m_scoreLabel;
    QLabel       *m_gradeLabel;
    QLabel       *m_scoreDetail;
    QPushButton  *m_retryBtn;

    // Settings
    QComboBox    *m_difficultyCombo;
};

#endif // QUIZWIDGET_H
