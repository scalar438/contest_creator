TEMPLATE = lib
CONFIG += staticlib

HEADERS += qchecklib.h

SOURCES += qchecklib.cpp

INCLUDEPATH += ..

QMAKE_CXXFLAGS += --std=c++0x
