#-------------------------------------------------
#
# Project created by QtCreator 2016-08-03T14:42:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PIDOverMachine
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
include(./qextserialport-1.2beta2/src/qextserialport.pri)
