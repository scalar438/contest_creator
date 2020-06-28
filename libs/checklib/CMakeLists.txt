add_library(checklib rp.cpp details/rp_win.cpp details/internal_watcher.cpp)
target_include_directories(checklib INTERFACE ..)

cmake_policy(SET CMP0074 NEW)

find_package(Boost REQUIRED)
target_include_directories(checklib PUBLIC ${Boost_INCLUDE_DIR})
target_link_directories(checklib INTERFACE ${Boost_LIBRARY_DIRS})

add_definitions("-DCHECKLIB_WINDOWS")

set_property(TARGET checklib PROPERTY CXX_STANDARD 17)