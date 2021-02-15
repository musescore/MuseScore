
# Libraries linked via full path no longer produce linker search paths.
cmake_policy(SET CMP0003 NEW)

# Apparently needed on Mac only (?)
if (APPLE)
    # Issue no warning non-existent target argument to get_target_property()
    # and set the result variable to a -NOTFOUND value rather than issuing a FATAL_ERROR 40
    if(POLICY CMP0045)
        cmake_policy(SET CMP0045 OLD)
    endif(POLICY CMP0045)

    # Silently ignore non-existent dependencies (mops1, mops2)
    if(POLICY CMP0046)
        cmake_policy(SET CMP0046 OLD)
    endif(POLICY CMP0046)
endif (APPLE)

# RPATH settings on macOS do not affect install_name
if(POLICY CMP0068)
      cmake_policy(SET CMP0068 NEW)
endif(POLICY CMP0068)

# Don't process generated source files with AUTOMOC
if(POLICY CMP0071)
      cmake_policy(SET CMP0071 OLD)
endif(POLICY CMP0071)