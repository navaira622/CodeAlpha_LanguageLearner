#include <QApplication>
#include <QDir>
#include <QCoreApplication>
#include <QFont>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("LinguaLearn");
    a.setOrganizationName("LinguaLearn");
    a.setOrganizationDomain("linguallearn.app");

    // Set app font
    QFont appFont("Segoe UI", 10);
    appFont.setStyleStrategy(QFont::PreferAntialias);
    a.setFont(appFont);

    MainWindow w;
    w.show();

    return a.exec();
}
