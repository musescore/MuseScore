if( NOT test_cmd )
   message( FATAL_ERROR "test_cmd not defined" )
endif( NOT test_cmd )

if( NOT test_output )
   message( FATAL_ERROR "test_output not defined" )
endif( NOT test_output )

if( NOT expected_output )
   message( FATAL_ERROR "expected_output not defined" )
endif( NOT expected_output )

separate_arguments( test_args )

execute_process(
   COMMAND ${test_cmd} ${test_args}
   RESULT_VARIABLE test_not_successful
)

if( test_not_successful )
    message( FATAL_ERROR "${test_cmd} ${test_args} returned error ${test_not_successful}!" )
endif( test_not_successful )

execute_process(
   COMMAND ${CMAKE_COMMAND} -E compare_files ${expected_output} ${test_output}
   RESULT_VARIABLE compare_not_successful
)

if( compare_not_successful )
   message( SEND_ERROR "${test_output} does not match ${expected_output}!" )
endif( compare_not_successful )
