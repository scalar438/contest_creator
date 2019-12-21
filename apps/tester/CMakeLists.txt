add_executable(tester 
	console_utils.cpp
	main.cpp params_reader.cpp 
	run_controller_abstract.cpp
	run_controller_interactive.cpp
	run_controller_simple.cpp
	settings.cpp)

target_link_libraries(tester PRIVATE checklib)
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost COMPONENTS filesystem date_time regex thread REQUIRED MODULE)
target_include_directories(tester PRIVATE ${BOOST_INCLUDEDIR})
target_link_libraries(tester PRIVATE ${Boost_LIBRARIES})

add_definitions("-DOS_WIN32")