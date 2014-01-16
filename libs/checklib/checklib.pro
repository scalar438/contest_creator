QT += core

TEMPLATE = lib
CONFIG += staticlib

HEADERS += \
    checklib_exception.h \
    rp_types.h \
    rp.h \  
    rp_consts.h \
    timer_service.h \
    noexcept.h

SOURCES += \
	rp.cpp

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
LIBS += -lboost_system -lboost_chrono
}
