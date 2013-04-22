TEMPLATE = lib
CONFIG += staticlib
HEADERS += restproc.h

win32 {
message("Building for windows")
SOURCES += ./details/restproc_win.cpp
}

unix {
message("Building for linux")
SOURCES += ./details/restproc_linux.cpp
}

QMAKE_CXXFLAGS += --std=c++0x
