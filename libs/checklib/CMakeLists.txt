add_library(checklib rp.cpp details/rp_win.cpp)
target_include_directories(checklib INTERFACE ..)

cmake_policy(SET CMP0074 NEW)

find_package(Boost REQUIRED)
target_include_directories(checklib PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(checklib INTERFACE ${Boost_LIBRARIES})

add_definitions("-DCHECKLIB_WINDOWS")

set_property(TARGET checklib PROPERTY CXX_STANDARD 17)