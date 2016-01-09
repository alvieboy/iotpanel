#-------------------------------------------------
#
# Project created by QtCreator 2015-11-28T10:10:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = paneleditor
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    PanelFramebuffer.cpp \
    PanelWidgetText.cpp \
    Panel.cpp \
    text.cpp \
    font_10x16.c \
    font_12x16.c          \
    font_16x16.c          \
    font_6x10.c           \
    font_apple4x6.c       \
    font_apple5x7.c       \
    font_apple6x10.c      \
    font.c                \
    font_tom_thumb.c


HEADERS  += mainwindow.h Panel.h font.h text.h PanelFramebuffer.h 

FORMS    += mainwindow.ui
