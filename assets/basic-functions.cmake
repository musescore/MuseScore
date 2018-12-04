function(required_program # ensure that a build dependency is installed
  PATHV # location of the program returned in this variable
  DESCRIPTION # build fails with this error message if program is missing
  COMMAND # the name of the program on the command line
  # ARGN any alternative names for the command
  )
  set(CMAKE_FIND_APPBUNDLE "NEVER") # macOS: don't search for .app bundles
  find_program(${PATHV} NAMES ${COMMAND} ${ARGN} DOC "${DESCRIPTION}")
  if(NOT ${PATHV} OR NOT EXISTS "${${PATHV}}")
    # Fail the build, but use SEND_ERROR instead of FATAL_ERROR to allow other
    # checks to complete (show all missing dependencies, not just the first)
    message(SEND_ERROR "Program not found: ${COMMAND} - ${DESCRIPTION}")
  endif(NOT ${PATHV} OR NOT EXISTS "${${PATHV}}")
endfunction(required_program)

function(required_variable # ensure that a required variable has been set
  NAMEV # the name of the variable
  DEFAULT_VALUE # a value to store in the variable if it was not already set
  )
  if(NOT DEFINED "${NAMEV}")
    if(STRICT_VARIABLES)
      # Fail the build, but use SEND_ERROR instead of FATAL_ERROR to allow other
      # checks to complete (show all missing dependencies, not just the first)
      message(SEND_ERROR "Variable not defined: ${NAMEV}")
    else(STRICT_VARIABLES)
      message(STATUS "Using default value for ${NAMEV}: \"${DEFAULT_VALUE}\"")
      set("${NAMEV}" "${DEFAULT_VALUE}" PARENT_SCOPE)
    endif(STRICT_VARIABLES)
  endif(NOT DEFINED "${NAMEV}")
endfunction(required_variable)

function(assert # ensure a condition is met (CI Test)
  STATEMENT # the first part of the condition statement
  # ARGN any more parts of the condition statement
  )
  if(NOT (${STATEMENT} ${ARGN}))
    set(ASSERTION "") # empty string ("unset()" exposes cached values)
    string(REPLACE ";" " " ASSERTION "${STATEMENT} ${ARGN}")
    message(FATAL_ERROR "Assertion failed: ${ASSERTION}\n"
      "please fix the code responsible to allow the build to proceed")
  endif(NOT (${STATEMENT} ${ARGN}))
endfunction(assert)

function(min_index # find the smallest value in a list of numbers
  MINV # minimum value returned in this variable
  IDXV # index of its first occurance returned in this variable
  # ARGN remaining arguments are items in the list
  )
  set(MIN_IDX 0) # start by assuming first item in list is the smallest
  list(GET ARGN ${MIN_IDX} MIN_VAL) # store its value for comparison
  set(IDX 0) # keep track of where we are in the list
  foreach(VAL ${ARGN})
    if(VAL LESS MIN_VAL)
      set(MIN_VAL ${VAL})
      set(MIN_IDX ${IDX})
    endif(VAL LESS MIN_VAL)
    math(EXPR IDX "${IDX}+1") # increment IDX
  endforeach(VAL)
  set("${MINV}" "${MIN_VAL}" PARENT_SCOPE)
  set("${IDXV}" "${MIN_IDX}" PARENT_SCOPE)
endfunction(min_index)

set(TEST_LIST 7 -2 20 -5 0 3 8)
min_index(TEST_MIN TEST_IDX ${TEST_LIST})
assert(TEST_MIN EQUAL -5 AND TEST_IDX EQUAL 3)

function(sort_integer_list # sort list of integers into ascending order
  LISTV # list variable to be sorted
  )
  set(LIST "${${LISTV}}")
  list(LENGTH LIST LEN)
  if(LEN GREATER 1)
    min_index(MIN IDX ${LIST}) # find smallest value
    list(REMOVE_AT LIST ${IDX}) # remove smallest value
    sort_integer_list(LIST) # sort the list that remains (recursion)
    set(${LISTV} ${MIN} ${LIST} PARENT_SCOPE)
  endif(LEN GREATER 1)
endfunction(sort_integer_list)

sort_integer_list(TEST_LIST)
set(TEST_SORT -5 -2 0 3 7 8 20)
assert(TEST_LIST STREQUAL TEST_SORT)

function(assert_ascending # ensure a list of integers is in ascending order
  LISTV # list variable to be checked
  )
  set(LIST "${${LISTV}}")
  set(SORTED "${LIST}")
  sort_integer_list(SORTED)
  if(NOT LIST STREQUAL SORTED)
    message(FATAL_ERROR "Assertion failed: ${LISTV} in ascending order.\n"
      "Please sort the list into ascending order to allow build to proceed.")
  endif(NOT LIST STREQUAL SORTED)
endfunction(assert_ascending)

function(assert_alphabetical # ensure list of strings is in alphabetical order
  LISTV # list variable to be checked
  )
  set(LIST "${${LISTV}}")
  set(SORTED "${LIST}")
  list(SORT SORTED)
  if(NOT LIST STREQUAL SORTED)
    message(FATAL_ERROR "Assertion failed: ${LISTV} in alphabetical order.\n"
      "Please sort the list into alphabetical order to allow build to proceed.")
  endif(NOT LIST STREQUAL SORTED)
endfunction(assert_alphabetical)
