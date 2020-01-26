add_executable(tester 
	console_utils.cpp
	main.cpp params_reader.cpp 
	run_controller_abstract.cpp
	run_controller_interactive.cpp
	run_controller_simple.cpp
	settings.cpp)

install(TARGETS tester)

cmake_policy(SET CMP0074 NEW)

target_link_libraries(tester PRIVATE checklib)
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost)

target_include_directories(tester PRIVATE ${Boost_INCLUDE_DIR})
target_link_directories(tester PRIVATE ${Boost_LIBRARY_DIRS})


add_definitions("-DOS_WIN32")