#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QTableWidget>
#include "datamanager.h"

class ProgressWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProgressWidget(DataManager *data, QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    DataManager  *m_data;

    QLabel       *m_learnedLabel;
    QLabel       *m_learnedPct;
    QProgressBar *m_learnedBar;

    QLabel       *m_streakLabel;
    QLabel       *m_lessonsLabel;
    QLabel       *m_quizLabel;
    QLabel       *m_bestQuizLabel;

    QTableWidget *m_historyTable;
};

#endif // PROGRESSWIDGET_H
