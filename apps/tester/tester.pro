TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += main.cpp \
	settings.cpp \
	params_reader.cpp \
	run_controller.cpp \
	console_utils.cpp

HEADERS += \
	io_consts.h \
	settings.h \
	params_reader.h \
	console_utils.h \
	run_controller.h \
	tester_exceptions.h

INCLUDEPATH += ../../libs
win32: INCLUDEPATH += $$(BOOST_INCLUDE_DIR)

LIBS += -L"../../libs/checklib" -lchecklib
win32: LIBS += -lpsapi -L$$(BOOST_LIB_DIR)

unix:QMAKE_CXXFLAGS += --std=c++0x
unix: LIBS += -lboost_system -lboost_filesystem -lboost_thread -lboost_chrono -pthread

PRE_TARGETDEPS += ../../libs/checklib
