QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = LanguageLearner
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/datamanager.cpp \
    src/vocabularywidget.cpp \
    src/flashcardwidget.cpp \
    src/quizwidget.cpp \
    src/lessonswidget.cpp \
    src/progresswidget.cpp \
    src/homewidget.cpp

RESOURCES += \
    resources.qrc

HEADERS += \
    src/mainwindow.h \
    src/datamanager.h \
    src/vocabularywidget.h \
    src/flashcardwidget.h \
    src/quizwidget.h \
    src/lessonswidget.h \
    src/progresswidget.h \
    src/homewidget.h \
    src/wordentry.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
