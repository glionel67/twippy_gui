#-------------------------------------------------
#
# Project created by QtCreator 2019-02-25T10:09:50
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = twippy_gui
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11 -DDEBUG

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
