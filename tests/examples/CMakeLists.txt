add_subdirectory(args_exitcode)
add_subdirectory(args_out)
add_subdirectory(IL)
add_subdirectory(ML)
add_subdirectory(RE)
add_subdirectory(stderr_out)
add_subdirectory(sum)
add_subdirectory(TL)

set_target_properties(pArgsOut      PROPERTIES OUTPUT_NAME "args_out")
set_target_properties(pArgsExitCode PROPERTIES OUTPUT_NAME "args_exit_code")
set_target_properties(pIL           PROPERTIES OUTPUT_NAME "il")
set_target_properties(pML           PROPERTIES OUTPUT_NAME "ml")
set_target_properties(pRE           PROPERTIES OUTPUT_NAME "re")
set_target_properties(pStderr_out   PROPERTIES OUTPUT_NAME "stderr_out")
set_target_properties(pSum          PROPERTIES OUTPUT_NAME "sum")
set_target_properties(pTL           PROPERTIES OUTPUT_NAME "tl")

install(TARGETS 
   pArgsOut pArgsExitCode pIL pML pRE pStderr_out pSum pTL
   DESTINATION "examples/")
    