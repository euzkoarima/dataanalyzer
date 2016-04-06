#-------------------------------------------------
#
# Project created by QtCreator 2015-11-24T21:15:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

TARGET = DataAnalyzer
TEMPLATE = app


SOURCES += main.cpp\
        dataanalyzer.cpp \
    test.cpp \
    measurement.cpp \
    testmodel.cpp \
    qcustomplot.cpp

HEADERS  += dataanalyzer.h \
    test.h \
    measurement.h \
    testmodel.h \
    qcustomplot.h

TRANSLATIONS = translations/DataAnalyzer_es.ts

FORMS    += dataanalyzer.ui
RESOURCES += dataanalyzer.qrc
