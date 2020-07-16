set(SOURCES process.cpp details/internal_watcher.cpp details/check_stats.cpp)

if (WIN32)
    add_definitions("-DCHECKLIB_WINDOWS")
    set(SOURCES ${SOURCES} details/rp_win.cpp)
elseif (UNIX)
    add_definitions("-DCHECKLIB_UNIX")
    set(SOURCES ${SOURCES} details/rp_linux.cpp)
endif()

add_library(checklib ${SOURCES})
target_include_directories(checklib INTERFACE ..)

cmake_policy(SET CMP0074 NEW)

find_package(Boost REQUIRED)

target_include_directories(checklib PUBLIC ${Boost_INCLUDE_DIR})
target_link_directories(checklib INTERFACE ${Boost_LIBRARY_DIRS})

set_property(TARGET checklib PROPERTY CXX_STANDARD 17)