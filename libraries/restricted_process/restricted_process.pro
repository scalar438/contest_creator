TEMPLATE = lib

HEADERS = restproc.h \

win32 {
warning("Building for windows")
SOURCES += ./details/restproc_win.cpp
}

unix {
warning("Building for linux")
SOURCES += ./details/restproc_linux.cpp
}

SOURCES = restproc.cpp
