add_library(checklib rp.cpp details/rp_win.cpp)
target_include_directories(checklib INTERFACE ..)

find_package(Boost COMPONENTS system filesystem date_time regex thread REQUIRED)
target_include_directories(checklib PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(checklib INTERFACE ${Boost_LIBRARIES})
target_link_libraries(checklib INTERFACE "Psapi.lib")


add_definitions("-DCHECKLIB_WINDOWS")
