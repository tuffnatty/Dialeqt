QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Dialeqt
TEMPLATE = app


SOURCES += main.cpp\
        dialeqt.cpp \
    dialogcreatenewdictionary.cpp \
    dictglobalattributes.cpp \
    dicttabscontainer.cpp

HEADERS  += dialeqt.h \
    dialogcreatenewdictionary.h \
    dictglobalattributes.h \
    dicttabscontainer.h

FORMS    += dialeqt.ui \
    dialogcreatenewdictionary.ui