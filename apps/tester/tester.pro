TEMPLATE = app
CONFIG += qt console
QT -= gui

SOURCES += \
	main.cpp \
	consoleUtils.cpp \
	ParamsReader.cpp \
    Runner.cpp \
    RunController.cpp

HEADERS += \
    consoleUtils.h \
    ParamsReader.h \
    Runner.h \
    RunController.h

INCLUDEPATH += ../../libs
win32: INCLUDEPATH += $$(BOOST_INCLUDE_DIR)

LIBS += -L"../../libs/checklib" -lchecklib
win32: LIBS += -lpsapi -L$$(BOOST_LIB_DIR)

unix:QMAKE_CXXFLAGS += --std=c++0x
unix: LIBS += -lboost_system -lboost_filesystem -lboost_thread -lboost_chrono
