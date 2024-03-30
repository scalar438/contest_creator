set(SOURCES impl/process.cpp impl/internal_watcher.cpp impl/check_stats.cpp)

if (WIN32)
    add_definitions("-DCHECKLIB_WINDOWS")
    set(SOURCES ${SOURCES} impl/rp_win.cpp)
  #  add_compile_definitions(
elseif (UNIX)
    add_definitions("-DCHECKLIB_UNIX")
    set(SOURCES ${SOURCES} impl/rp_linux.cpp)
    
    find_package(Boost REQUIRED)

    #target_include_directories(checklib PUBLIC ${Boost_INCLUDE_DIR})
    #target_link_directories(checklib INTERFACE ${Boost_LIBRARY_DIRS})
endif()

add_library(checklib ${SOURCES})
target_include_directories(checklib INTERFACE "include")
target_include_directories(checklib PRIVATE "include/checklib")

cmake_policy(SET CMP0074 NEW)