add_subdirectory(examples)

add_executable(test_runner test_runner.cpp)
target_link_libraries(test_runner PRIVATE checklib)

install(TARGETS test_runner DESTINATION "./test_script")
install(FILES run.py DESTINATION "./test_script")