QT += core

TEMPLATE = lib
CONFIG += staticlib

HEADERS += \
    checklib_exception.h \
    interactive_processes.h \
    rp_types.h \
    rp.h \
    details/rp_win.h

win32 {
HEADERS += $$(BOOST_INCLUDE_DIR)
SOURCES +=
LIBS += -lpsapi
TARGET = ../checklib
}

unix {
SOURCES += ./details/restproc_linux.cpp
TARGET = checklib
}

QMAKE_CXXFLAGS += --std=c++0x

SOURCES += \
    interactive_processes.cpp \
    rp.cpp \
    details/rp_win.cpp
