add_executable(lcmp lcmp.cpp)
target_include_directories(lcmp PRIVATE ../)

install(TARGETS lcmp RUNTIME DESTINATION bin)