#-------------------------------------------------
#
# Project created by QtCreator 2016-04-26T20:37:47
#
#-------------------------------------------------

QT       += core gui network webkit webkitwidgets testlib

DEFINES += QUAZIP_STATIC

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QNizikano
TEMPLATE = app


SOURCES += main.cpp\
        nizikano.cpp \
    nizikanopage.cpp \
    aspectwebview.cpp

HEADERS  += nizikano.h \
    nizikanopage.h \
    aspectwebview.h

FORMS    += nizikano.ui

LIBS     += -lz -lsfml-audio -lsfml-system

INCLUDEPATH += /usr/include/SFML/

include(quazip/quazip.pri)
