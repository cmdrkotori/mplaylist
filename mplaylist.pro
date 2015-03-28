#-------------------------------------------------
#
# Project created by QtCreator 2015-03-04T21:43:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mplaylist
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    window.cpp \
    storage.cpp

HEADERS  += widget.h \
    window.h \
    storage.h

FORMS    += widget.ui \
    window.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    README.rst \
    LICENSE
