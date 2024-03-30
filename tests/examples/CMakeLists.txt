add_executable(pArgsExitCode args_exitcode.cpp)
add_executable(pArgsOut args_out.cpp)
add_executable(pIL IL.cpp)
add_executable(pML ML.cpp)
add_executable(pRE RE.cpp)
add_executable(pStderr_out stderr_out.cpp)
add_executable(pTL TL.cpp)
add_executable(pSum sum.cpp)

install(TARGETS 
   pArgsOut pArgsExitCode pIL pML pRE pStderr_out pSum pTL
   DESTINATION "test_script/examples/")