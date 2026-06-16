#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QHash>
#include <QSet>
#include <QDate>
#include "wordentry.h"

class DataManager : public QObject
{
    Q_OBJECT

public:
    explicit DataManager(QObject *parent = nullptr);

    // Vocabulary
    bool loadVocabulary(const QString &filePath = QString());
    const QVector<WordEntry>& getAllWords() const;           // const-ref: no copy
    QVector<WordEntry> getWordsByDifficulty(const QString &difficulty) const;
    QVector<WordEntry> getDailyWords(int count = 10) const;
    QVector<WordEntry> getWordsByCategory(const QString &category) const;
    QVector<WordEntry> searchWords(const QString &query) const;

    // Returns sorted list of unique category strings (built at load time)
    const QStringList& getCategories() const;

    // Load status / error reporting
    bool hasLoadError() const;
    QString lastError() const;

    // Progress
    void markWordLearned(int wordId);
    bool isWordLearned(int wordId) const;
    int getLearnedCount() const;
    int getTotalWords() const;

    // Quiz scores
    void saveQuizScore(int score, int total);
    QVector<QPair<QDate, QPair<int,int>>> getQuizHistory() const;
    int getLastQuizScore() const;
    int getLastQuizTotal() const;

    // Lessons
    void markLessonComplete(const QDate &date);
    bool isDailyLessonComplete() const;
    int getCompletedLessonsCount() const;
    QDate getLastLessonDate() const;
    int getCurrentStreak() const;

    // Persistence
    bool saveProgress(const QString &filePath);
    bool loadProgress(const QString &filePath);

    // Settings
    bool isDarkMode() const;
    void setDarkMode(bool dark);

private:
    QVector<WordEntry> m_words;
    QMap<int, bool>    m_learnedWords;   // wordId -> learned flag

    // O(1) lookup: word index by id — avoids O(N) linear scan in markWordLearned
    QHash<int, int>    m_idToIndex;

    // Pre-built category list (sorted, unique) — built once in loadVocabulary
    QStringList        m_categories;

    QVector<QPair<QDate, QPair<int,int>>> m_quizHistory;
    QVector<QDate>     m_completedLessons;
    QSet<QDate>        m_completedLessonsSet; // fast O(1) lookup
    bool m_darkMode = false;
    QString m_progressFilePath;
    bool m_loadError = false;
    QString m_lastError;

    void ensureProgressPath();
    void rebuildLearnedCache(); // keep m_words[i].learned in sync
};

#endif // DATAMANAGER_H
