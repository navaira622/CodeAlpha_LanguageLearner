#ifndef HOMEWIDGET_H
#define HOMEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QDate>
#include <QTime>
#include "datamanager.h"

class HomeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HomeWidget(DataManager *data, QWidget *parent = nullptr);

signals:
    void navigateTo(int index);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    DataManager *m_data;
};

#endif // HOMEWIDGET_H
