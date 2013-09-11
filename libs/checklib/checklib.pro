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
INCLUDEPATH += $$(BOOST_INCLUDE_DIR)
LIBS += -L$$(BOOST_LIB_DIR)
}

unix {
HEADERS += details/rp_linux.h
SOURCES += details/rp_linux.cpp
TARGET = checklib
QMAKE_CXXFLAGS += --std=c++0x
LIBS += -lboost_system
}

SOURCES += \
    rp.cpp
