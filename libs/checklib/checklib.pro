QT += core

TEMPLATE = lib
CONFIG += staticlib

HEADERS += \
    checklib_exception.h \
    rp_types.h \
    rp.h 

win32 {
HEADERS += details/rp_win.h
SOURCES += details/rp_win.cpp
LIBS += -lpsapi
TARGET = ../checklib
}

unix {
SOURCES += ./details/restproc_linux.cpp
TARGET = checklib
}

QMAKE_CXXFLAGS += --std=c++0x

SOURCES += \
    rp.cpp
