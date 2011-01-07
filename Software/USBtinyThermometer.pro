#-------------------------------------------------
#
# Project created by QtCreator 2011-01-07T17:06:23
#
#-------------------------------------------------

QT       -= gui

TARGET = USBtinyThermometer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lusb

SOURCES += main.cpp \
    opendevice.cpp \

HEADERS += opendevice.h \
    ../Firmware/usbtiny.h \
    ../Firmware/commands.h \
    version.h \

