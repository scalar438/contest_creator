add_executable(tester 
	console_utils.cpp
	main.cpp params_reader.cpp 
	run_controller_abstract.cpp
	run_controller_interactive.cpp
	run_controller_simple.cpp
	settings.cpp)

install(TARGETS tester RUNTIME DESTINATION bin)

cmake_policy(SET CMP0074 NEW)

target_link_libraries(tester PRIVATE checklib)
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost)

if (WIN32)
	add_definitions("-DOS_WIN32")
elseif(UNIX)
	# TODO: check this option after removing boost dependency
	target_link_libraries(tester PRIVATE boost_system boost_thread pthread boost_chrono boost_filesystem)
endif()