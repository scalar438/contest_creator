set(CMAKE_AUTOMOC ON)
add_executable(testRP main.cpp test.cpp test.h)

find_package(Qt5 COMPONENTS Core Test REQUIRED)
target_link_libraries(testRP PRIVATE ${Qt5Test_LIBRARIES})
target_include_directories(testRP PRIVATE ${Qt5Test_INCLUDE_DIRS})

target_link_libraries(testRP PRIVATE checklib)

set(Boost_USE_STATIC_LIBS ON)

find_package(Boost COMPONENTS filesystem date_time regex thread REQUIRED)
target_link_libraries(testRP PRIVATE ${Boost_LIBRARIES})
