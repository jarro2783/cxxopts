# Check the exact exit status as well as the diagnostic. A crash must not count
# as a successful rejection of invalid input.
foreach(argument --number=bad --unknown)
    execute_process(COMMAND "${PROGRAM}" "${argument}"
        RESULT_VARIABLE result ERROR_VARIABLE diagnostic)
    if(NOT "${result}" STREQUAL "1")
        message(FATAL_ERROR "${argument}: expected exit 1, got ${result}")
    endif()
    if("${argument}" STREQUAL "--number=bad")
        set(expected "Argument 'bad' failed to parse")
    else()
        set(expected "Option 'unknown' does not exist")
    endif()
    string(FIND "${diagnostic}" "${expected}" match)
    if(match EQUAL -1)
        message(FATAL_ERROR "${argument}: missing diagnostic: ${diagnostic}")
    endif()
endforeach()
