cmake_minimum_required(VERSION 3.10)
project(contests-creator VERSION 1.0.3)

#add_subdirectory(apps)
add_subdirectory(libs)

option(TESTING "Build and install tests")
if(${TESTING})
   enable_testing()
   #add_subdirectory(tests)
endif()
