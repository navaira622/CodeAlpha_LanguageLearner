#ifndef FLASHCARDWIDGET_H
#define FLASHCARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QComboBox>
#include "datamanager.h"

class FlashcardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FlashcardWidget(DataManager *data, QWidget *parent = nullptr);

private slots:
    void showFront();
    void showBack();
    void nextCard();
    void prevCard();
    void shuffleCards();
    void onFilterChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    void updateCard();
    void updateNavButtons();

    DataManager  *m_data;
    QVector<WordEntry> m_cards;
    int m_currentIndex = 0;
    bool m_showingBack = false;

    QStackedWidget *m_cardStack;
    QWidget        *m_frontCard;
    QWidget        *m_backCard;

    QLabel  *m_frontWord;
    QLabel  *m_frontHint;
    QLabel  *m_backWord;
    QLabel  *m_backTranslation;
    QLabel  *m_backExample;

    QPushButton *m_btnPrev;
    QPushButton *m_btnFlip;
    QPushButton *m_btnNext;
    QPushButton *m_btnShuffle;
    QLabel      *m_counterLabel;
    QComboBox   *m_filterCombo;
};

#endif // FLASHCARDWIDGET_H
