TEMPLATE = lib
CONFIG += staticlib
HEADERS += \
    checklib_exception.h \
    restricted_process.h



win32 {
message("Building for windows")
SOURCES += ./details/restproc_win.cpp
LIBS += -lpsapi
TARGET = ../checklib
}

unix {
message("Building for linux")
SOURCES += ./details/restproc_linux.cpp
TARGET = checklib
}

QMAKE_CXXFLAGS += --std=c++0x
