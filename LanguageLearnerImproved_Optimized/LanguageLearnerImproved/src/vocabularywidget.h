#ifndef VOCABULARYWIDGET_H
#define VOCABULARYWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVector>
#include "datamanager.h"
#include "wordentry.h"

class VocabularyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VocabularyWidget(DataManager *data, QWidget *parent = nullptr);
    void refresh();

private slots:
    void onSearch(const QString &query);
    void onFilterChanged();
    void onMarkLearned();
    void onTableSelectionChanged();
    void applySearchDebounced(); // fires after typing pause

private:
    // ── Core filter + populate ───────────────────────────────────────────────
    void applyFilters();
    void populateTable(const QVector<const WordEntry*> &words);
    void updateRow(int row, const WordEntry &e);

    // ── Detail panel ─────────────────────────────────────────────────────────
    void showDetail(const WordEntry &entry);
    void hideDetail();

    // ── Data ─────────────────────────────────────────────────────────────────
    DataManager  *m_data;

    // Stores raw pointers into m_data's word vector — no copies.
    // Invalidated only when DataManager reloads vocabulary (which never happens
    // at runtime), or after markWordLearned() which calls refresh().
    QVector<const WordEntry*> m_currentWords;

    // ── Widgets ──────────────────────────────────────────────────────────────
    QTableWidget *m_table;
    QLineEdit    *m_searchBar;
    QComboBox    *m_filterCombo;
    QComboBox    *m_categoryCombo;
    QWidget      *m_detailPanel;
    QLabel       *m_detailWord;
    QLabel       *m_detailTranslation;
    QLabel       *m_detailExample;
    QLabel       *m_detailPronunciation;
    QLabel       *m_detailDifficulty;
    QLabel       *m_detailPlaceholder;
    QPushButton  *m_markLearnedBtn;
    QLabel       *m_countLabel;

    // Cached list of all detail-panel widgets (avoids findChild loops)
    QList<QWidget*> m_detailWidgets;

    // ── Search debounce ──────────────────────────────────────────────────────
    QTimer  *m_searchTimer;

    // ── Row-recycling state ──────────────────────────────────────────────────
    // We keep the table's row count stable between calls when possible so Qt
    // doesn't destroy and recreate QTableWidgetItems on every filter change.
    int m_tableRowCapacity = 0; // current setRowCount value
};

#endif // VOCABULARYWIDGET_H
