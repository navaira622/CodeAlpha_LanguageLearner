#include "datamanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <algorithm>
#include <random>

// ─────────────────────────────────────────────────────────────────────────────
DataManager::DataManager(QObject *parent) : QObject(parent)
{
    ensureProgressPath();
    // Load progress first so learned flags are known before vocabulary is parsed
    loadProgress(m_progressFilePath);

    if (!loadVocabulary()) {
        qWarning() << "[DataManager] Failed to load vocabulary:" << m_lastError;
    } else {
        qDebug() << "[DataManager] Loaded" << m_words.size() << "words.";
    }
}

void DataManager::ensureProgressPath()
{
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appData);
    m_progressFilePath = appData + "/progress.json";
}

// ── Vocabulary loading ───────────────────────────────────────────────────────

static bool parseVocabularyJson(const QByteArray &data,
                                 QVector<WordEntry> &outWords,
                                 QHash<int,int> &outIdToIndex,
                                 QStringList &outCategories,
                                 const QMap<int,bool> &learnedWords,
                                 QString &error)
{
    QJsonParseError pe;
    QJsonDocument doc = QJsonDocument::fromJson(data, &pe);
    if (doc.isNull()) {
        error = "JSON parse error: " + pe.errorString();
        return false;
    }

    QJsonArray words = doc.object().value("words").toArray();
    if (words.isEmpty()) {
        error = "JSON file contains no 'words' array or it is empty.";
        return false;
    }

    outWords.clear();
    outWords.reserve(words.size());
    outIdToIndex.clear();
    outIdToIndex.reserve(words.size());

    QSet<QString> categorySet;

    for (const QJsonValue &v : words) {
        QJsonObject obj = v.toObject();
        WordEntry e;
        e.id            = obj.value("id").toInt();
        e.word          = obj.value("word").toString();
        e.translation   = obj.value("translation").toString();
        e.example       = obj.value("example").toString();
        e.pronunciation = obj.value("pronunciation").toString();
        e.difficulty    = obj.value("difficulty").toString();
        e.category      = obj.value("category").toString();
        e.learned       = learnedWords.value(e.id, false);

        // Pre-compute lowercase strings once — avoids repeated toLower() during search
        e.wordLower        = e.word.toLower();
        e.translationLower = e.translation.toLower();
        e.categoryLower    = e.category.toLower();

        const int idx = outWords.size();
        outIdToIndex.insert(e.id, idx);
        outWords.append(std::move(e));

        categorySet.insert(outWords.last().category);
    }

    // Build sorted category list
    outCategories = categorySet.values();
    std::sort(outCategories.begin(), outCategories.end());

    return true;
}

bool DataManager::loadVocabulary(const QString &filePath)
{
    m_loadError = false;
    m_lastError.clear();

    QStringList candidates;
    if (!filePath.isEmpty()) candidates << filePath;
    candidates << ":/data/vocabulary.json";
    const QString appDir = QCoreApplication::applicationDirPath();
    candidates << appDir + "/data/vocabulary.json"
               << appDir + "/../data/vocabulary.json"
               << appDir + "/../../data/vocabulary.json"
               << appDir + "/../../../data/vocabulary.json"
               << "data/vocabulary.json"
               << "../data/vocabulary.json";

    for (const QString &path : candidates) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) continue;
        QByteArray data = file.readAll();
        QString parseErr;
        QVector<WordEntry> words;
        QHash<int,int>     idToIndex;
        QStringList        categories;
        if (parseVocabularyJson(data, words, idToIndex, categories, m_learnedWords, parseErr)) {
            m_words      = std::move(words);
            m_idToIndex  = std::move(idToIndex);
            m_categories = std::move(categories);
            qDebug() << "[DataManager] Vocabulary loaded from:" << path
                     << "(" << m_words.size() << "words)";
            return true;
        }
        qWarning() << "[DataManager] Parse failed for" << path << ":" << parseErr;
        m_lastError = parseErr;
    }

    if (m_lastError.isEmpty())
        m_lastError = "Could not find vocabulary.json in any known location.";
    m_loadError = true;
    m_words.clear();
    m_idToIndex.clear();
    m_categories.clear();
    return false;
}

bool DataManager::hasLoadError() const { return m_loadError; }
QString DataManager::lastError() const { return m_lastError; }

// ── Vocabulary queries — return const-ref or filtered copies ─────────────────

const QVector<WordEntry>& DataManager::getAllWords() const { return m_words; }

const QStringList& DataManager::getCategories() const { return m_categories; }

QVector<WordEntry> DataManager::getWordsByDifficulty(const QString &difficulty) const
{
    QVector<WordEntry> result;
    for (const WordEntry &e : m_words)
        if (e.difficulty == difficulty) result.append(e);
    return result;
}

QVector<WordEntry> DataManager::getWordsByCategory(const QString &category) const
{
    QVector<WordEntry> result;
    for (const WordEntry &e : m_words)
        if (e.category == category) result.append(e);
    return result;
}

QVector<WordEntry> DataManager::getDailyWords(int count) const
{
    if (m_words.isEmpty()) return {};
    QDate today = QDate::currentDate();
    int seed = today.year() * 10000 + today.month() * 100 + today.day();
    QVector<WordEntry> all = m_words;  // intentional copy for shuffle
    std::mt19937 rng(static_cast<unsigned>(seed));
    std::shuffle(all.begin(), all.end(), rng);
    return all.mid(0, qMin(count, all.size()));
}

QVector<WordEntry> DataManager::searchWords(const QString &query) const
{
    // Use pre-lowercased fields — no per-call toLower() allocations
    const QString q = query.toLower();
    QVector<WordEntry> result;
    result.reserve(m_words.size() / 4); // reasonable starting reservation
    for (const WordEntry &e : m_words) {
        if (e.wordLower.contains(q) ||
            e.translationLower.contains(q) ||
            e.categoryLower.contains(q))
            result.append(e);
    }
    return result;
}

// ── Progress ─────────────────────────────────────────────────────────────────

void DataManager::markWordLearned(int wordId)
{
    m_learnedWords[wordId] = true;

    // O(1) lookup via id-to-index map — no more O(N) linear scan
    auto it = m_idToIndex.find(wordId);
    if (it != m_idToIndex.end()) {
        const int idx = it.value();
        if (idx >= 0 && idx < m_words.size())
            m_words[idx].learned = true;
    }

    saveProgress(m_progressFilePath);
}

bool DataManager::isWordLearned(int wordId) const
{
    return m_learnedWords.value(wordId, false);
}

int DataManager::getLearnedCount() const
{
    int count = 0;
    for (auto it = m_learnedWords.constBegin(); it != m_learnedWords.constEnd(); ++it)
        if (it.value()) ++count;
    return count;
}

int DataManager::getTotalWords() const { return m_words.size(); }

// ── Quiz scores ───────────────────────────────────────────────────────────────

void DataManager::saveQuizScore(int score, int total)
{
    m_quizHistory.append({QDate::currentDate(), {score, total}});
    saveProgress(m_progressFilePath);
}

QVector<QPair<QDate, QPair<int,int>>> DataManager::getQuizHistory() const
{ return m_quizHistory; }

int DataManager::getLastQuizScore() const
{ return m_quizHistory.isEmpty() ? 0 : m_quizHistory.last().second.first; }

int DataManager::getLastQuizTotal() const
{ return m_quizHistory.isEmpty() ? 0 : m_quizHistory.last().second.second; }

// ── Lessons ───────────────────────────────────────────────────────────────────

void DataManager::markLessonComplete(const QDate &date)
{
    if (!m_completedLessonsSet.contains(date)) {
        m_completedLessons.append(date);
        m_completedLessonsSet.insert(date);
    }
    saveProgress(m_progressFilePath);
}

bool DataManager::isDailyLessonComplete() const
{
    return m_completedLessonsSet.contains(QDate::currentDate());
}

int DataManager::getCompletedLessonsCount() const { return m_completedLessons.size(); }

QDate DataManager::getLastLessonDate() const
{
    if (m_completedLessons.isEmpty()) return QDate();
    return *std::max_element(m_completedLessons.begin(), m_completedLessons.end());
}

int DataManager::getCurrentStreak() const
{
    if (m_completedLessonsSet.isEmpty()) return 0;

    QDate cursor = QDate::currentDate();
    // Streak still counts if yesterday was the last completed day
    if (!m_completedLessonsSet.contains(cursor)) {
        cursor = cursor.addDays(-1);
        if (!m_completedLessonsSet.contains(cursor))
            return 0;
    }

    int streak = 0;
    while (m_completedLessonsSet.contains(cursor)) {
        ++streak;
        cursor = cursor.addDays(-1);
    }
    return streak;
}

// ── Persistence ───────────────────────────────────────────────────────────────

bool DataManager::saveProgress(const QString &filePath)
{
    QJsonObject root;

    QJsonArray learnedArr;
    for (auto it = m_learnedWords.constBegin(); it != m_learnedWords.constEnd(); ++it)
        if (it.value()) learnedArr.append(it.key());
    root["learned_words"] = learnedArr;

    QJsonArray quizArr;
    for (const auto &entry : m_quizHistory) {
        QJsonObject o;
        o["date"]  = entry.first.toString(Qt::ISODate);
        o["score"] = entry.second.first;
        o["total"] = entry.second.second;
        quizArr.append(o);
    }
    root["quiz_history"] = quizArr;

    QJsonArray lessonsArr;
    for (const QDate &d : m_completedLessons)
        lessonsArr.append(d.toString(Qt::ISODate));
    root["completed_lessons"] = lessonsArr;

    root["dark_mode"] = m_darkMode;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    return true;
}

bool DataManager::loadProgress(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) return false;
    QJsonObject root = doc.object();

    m_learnedWords.clear();
    for (const QJsonValue &v : root["learned_words"].toArray())
        m_learnedWords[v.toInt()] = true;

    m_quizHistory.clear();
    for (const QJsonValue &v : root["quiz_history"].toArray()) {
        QJsonObject o = v.toObject();
        m_quizHistory.append({
            QDate::fromString(o["date"].toString(), Qt::ISODate),
            {o["score"].toInt(), o["total"].toInt()}
        });
    }

    m_completedLessons.clear();
    m_completedLessonsSet.clear();
    for (const QJsonValue &v : root["completed_lessons"].toArray()) {
        QDate d = QDate::fromString(v.toString(), Qt::ISODate);
        m_completedLessons.append(d);
        m_completedLessonsSet.insert(d);
    }

    m_darkMode = root["dark_mode"].toBool(false);
    return true;
}

void DataManager::rebuildLearnedCache()
{
    for (WordEntry &e : m_words)
        e.learned = m_learnedWords.value(e.id, false);
}

bool DataManager::isDarkMode() const { return m_darkMode; }
void DataManager::setDarkMode(bool dark)
{
    m_darkMode = dark;
    saveProgress(m_progressFilePath);
}
