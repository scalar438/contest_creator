TEMPLATE = lib
CONFIG += staticlib
HEADERS += \
    checklib_exception.h \
    restricted_process.h

win32 {
message("Building for windows")
SOURCES += ./details/restproc_win.cpp
}

unix {
message("Building for linux")
SOURCES += ./details/restproc_linux.cpp
}

QMAKE_CXXFLAGS += --std=c++0x
