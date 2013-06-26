TEMPLATE = lib
CONFIG += staticlib

HEADERS += \
    checklib_exception.h \
    restricted_process.h \
	restricted_process_types.h

win32 {
message("Building for windows")
HEADERS += details/rp_win.h
LIBS += -lpsapi
TARGET = ../checklib
}

unix {
message("Building for linux")
SOURCES += ./details/restproc_linux.cpp
TARGET = checklib
}

QMAKE_CXXFLAGS += --std=c++0x

SOURCES += \
    restricted_process.cpp
