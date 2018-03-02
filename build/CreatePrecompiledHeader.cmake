macro( precompiled_header includes header_name build_pch)
    if (NOT "${CMAKE_GENERATOR}" STREQUAL "Xcode")
        message(STATUS "Precompiled header generation")
        # Get the compiler flags for this build type
        string( TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" flags_for_build_name )
        set( compile_flags "${CMAKE_CXX_FLAGS} ${${flags_for_build_name}}" )

        # Add all the Qt include directories
        foreach( item ${${includes}} )
            list( APPEND compile_flags "-I${item}" )
        endforeach()

        # Get the list of all build-independent preprocessor definitions
        get_directory_property( defines_global COMPILE_DEFINITIONS )
        list( APPEND defines ${defines_global} )

        # Get the list of all build-dependent preprocessor definitions
        string( TOUPPER "COMPILE_DEFINITIONS_${CMAKE_BUILD_TYPE}" defines_for_build_name )
        get_directory_property( defines_build ${defines_for_build_name} )
        list( APPEND defines ${defines_build} )

        # Add the "-D" prefix to all of them
        foreach( item ${defines} )
            list( APPEND all_define_flags "-D${item}" )
        endforeach()

        list( APPEND compile_flags ${all_define_flags} )

        # Prepare the compile flags var for passing to GCC
        separate_arguments( compile_flags )

        set (PCH_HEADER "${PROJECT_BINARY_DIR}/${header_name}.h")
        set (PCH_INCLUDE "-include ${PCH_HEADER}")

        if( ${build_pch} )
            set (PCH ${PROJECT_BINARY_DIR}/${header_name}.h.gch)
            add_custom_command(
             OUTPUT ${PROJECT_BINARY_DIR}/${header_name}.h.gch
             COMMAND ${CMAKE_CXX_COMPILER} -x c++-header -g  ${compile_flags} -o ${header_name}.h.gch ${header_name}.h
             DEPENDS ${PROJECT_BINARY_DIR}/${header_name}.h
             WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
             VERBATIM
             )
        else ( ${build_pch} )
            message(STATUS "No precompiled header")
        endif( ${build_pch} )
    endif()
endmacro()

# Xcode PCH support. Has to be called *AFTER* the target is created.
# "header_name" - the name of the PCH header, without the extension; "all" or something similar;
#                  note that the source file compiling the header needs to have the same name
macro( xcode_pch target_name header_name )
    if( APPLE )
        set_target_properties(
            ${target_name}
            PROPERTIES
            XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${PROJECT_BINARY_DIR}/${header_name}.h"
            XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES"
        )
    endif()
endmacro()
