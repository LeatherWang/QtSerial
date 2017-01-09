#-------------------------------------------------
#
# Project created by QtCreator 2016-07-10T17:09:00
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LeatherCOM
TEMPLATE = app


SOURCES += main.cpp\
        mainform.cpp \
    widget.cpp \
    hled.cpp

HEADERS  += mainform.h \
    widget.h \
    hled.h

FORMS    += \
    widget.ui \
    mainform.ui
include(./qextserialport-1.2beta2/src/qextserialport.pri)

RESOURCES += \
    image.qrc
