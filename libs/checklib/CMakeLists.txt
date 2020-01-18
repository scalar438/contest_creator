add_library(checklib rp.cpp details/rp_win.cpp)
target_include_directories(checklib INTERFACE ..)

find_package(Boost REQUIRED COMPONENTS filesystem date_time regex thread MODULE)
target_include_directories(checklib PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(checklib INTERFACE ${Boost_LIBRARIES})

add_definitions("-DCHECKLIB_WINDOWS")