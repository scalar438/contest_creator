TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += main.cpp \
	settings.cpp \
	params_reader.cpp \
	run_controller_abstract.cpp \
	run_controller_simple.cpp \
	run_controller_interactive.cpp \
	console_utils.cpp

HEADERS += \
	io_consts.h \
	settings.h \
	params_reader.h \
	console_utils.h \
	run_controller_abstract.h \
	run_controller_simple.h \
	run_controller_interactive.h \
	tester_exceptions.h

INCLUDEPATH += ../../libs
win32: INCLUDEPATH += $$(BOOST_INCLUDE_DIR)

LIBS += -L"../../libs/checklib" -lchecklib
win32: LIBS += -lpsapi -L$$(BOOST_LIB_DIR)
win32: DEFINES += OS_WIN32

unix:QMAKE_CXXFLAGS += --std=c++0x
unix: LIBS += -lboost_system -lboost_filesystem -lboost_thread -lboost_chrono -pthread

PRE_TARGETDEPS += ../../libs/checklib
