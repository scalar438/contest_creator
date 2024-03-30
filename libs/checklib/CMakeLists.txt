set(SOURCES impl/process.cpp impl/internal_watcher.cpp impl/check_stats.cpp)

if (WIN32)
    add_definitions("-DCHECKLIB_WINDOWS")
    set(SOURCES ${SOURCES} impl/rp_win.cpp)
  #  add_compile_definitions(
elseif (UNIX)
    add_definitions("-DCHECKLIB_UNIX")
    set(SOURCES ${SOURCES} impl/rp_linux.cpp)
    
    find_package(Boost REQUIRED)
endif()

add_library(checklib ${SOURCES})
target_link_options(checklib INTERFACE -lboost_chrono -lboost_thread)

target_include_directories(checklib INTERFACE "include")
target_include_directories(checklib PRIVATE "include/checklib")

cmake_policy(SET CMP0074 NEW)