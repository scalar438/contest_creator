TEMPLATE = app
CONFIG += qt
QT -= gui
QT += testlib

SOURCES += run_once_program.cpp \
   test.cpp

HEADERS += test.h

INCLUDEPATH += ../../libs
win32: INCLUDEPATH += $$(BOOST_INCLUDE_DIR)

LIBS += -L"../../libs/checklib" -lchecklib

win32: LIBS += -lpsapi -L$$(BOOST_LIB_DIR)

PRE_TARGETDEPS += ../../libs/checklib

unix:QMAKE_CXXFLAGS += --std=c++0x
unix: LIBS += -lboost_system -lboost_filesystem
TARGET = runOnceProgram

DESTDIR = ../
