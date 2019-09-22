add_executable(tester 
	console_utils.cpp
	main.cpp params_reader.cpp 
	run_controller_abstract.cpp
	run_controller_interactive.cpp
	run_controller_simple.cpp
	settings.cpp)

target_link_libraries(tester PRIVATE checklib)
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost COMPONENTS filesystem date_time regex thread REQUIRED)
target_link_libraries(tester PRIVATE ${Boost_LIBRARIES})
target_link_libraries(tester PRIVATE "Psapi.lib")

add_definitions("-DOS_WIN32")
add_definitions("-DBOOST_ALL_NO_LIB")