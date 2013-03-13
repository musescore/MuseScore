macro( precompiled_header includes header_name )
    if( ${CMAKE_COMPILER_IS_GNUCXX})
        message(STATUS "precompiled header generation")
        # Get the compiler flags for this build type
        string( TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" flags_for_build_name )
        set( compile_flags ${${flags_for_build_name}} )

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

        add_custom_command(
         OUTPUT ${PROJECT_BINARY_DIR}/${header_name}.h.gch
         COMMAND ${CMAKE_CXX_COMPILER}
           -x c++-header -g  ${compile_flags} -o ${header_name}.h.gch ${header_name}.h
         DEPENDS ${PROJECT_BINARY_DIR}/${header_name}.h
         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
         VERBATIM
         )
    endif()
endmacro()