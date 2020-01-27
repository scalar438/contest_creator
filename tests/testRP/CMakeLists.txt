add_executable(testRP main.cpp)

target_link_libraries(testRP PRIVATE checklib)

install(TARGETS testRP)