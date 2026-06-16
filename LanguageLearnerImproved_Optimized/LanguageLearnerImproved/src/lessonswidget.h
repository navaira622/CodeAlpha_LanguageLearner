#ifndef LESSONSWIDGET_H
#define LESSONSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QStackedWidget>
#include <QTabWidget>
#include <QScrollArea>
#include <QProgressBar>
#include "datamanager.h"

struct LessonInfo {
    QString id;
    QString title;
    QString icon;
    QString description;
    QString category;        // empty = "daily"
    QVector<WordEntry> words;
    bool completed = false;
};

class LessonsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LessonsWidget(DataManager *data, QWidget *parent = nullptr);

private slots:
    void onDailyLessonClicked(int row);
    void onTopicLessonClicked(int row);
    void onWordSelected(int row);
    void onMarkComplete();
    void onBackToList();

private:
    void buildDailyLesson();
    void buildTopicLessons();
    void refreshListWidgets();
    void showLesson(const LessonInfo &lesson);

    DataManager *m_data;

    // Main pages
    QStackedWidget   *m_mainStack;   // 0 = browser, 1 = detail

    // ── Page 0: lesson browser ───────────────────────────────────────
    QTabWidget       *m_tabWidget;
    QListWidget      *m_dailyList;
    QListWidget      *m_topicList;

    // ── Page 1: lesson detail ────────────────────────────────────────
    QLabel           *m_lessonTitle;
    QLabel           *m_lessonIcon;
    QLabel           *m_lessonDesc;
    QLabel           *m_progressLbl;
    QProgressBar     *m_progressBar;
    QListWidget      *m_wordList;
    QStackedWidget   *m_detailStack;  // 0 = hint, 1 = word detail
    QLabel           *m_detailWord;
    QLabel           *m_detailTranslation;
    QLabel           *m_detailPronunciation;
    QLabel           *m_detailExample;
    QPushButton      *m_markCompleteBtn;
    QPushButton      *m_backBtn;

    QVector<LessonInfo> m_dailyLessons;
    QVector<LessonInfo> m_topicLessons;
    LessonInfo          m_currentLesson;

    // Flag to suppress spurious currentRowChanged during list rebuild
    bool m_rebuilding = false;
};

#endif // LESSONSWIDGET_H
