
# Common
if (MSVC)
    add_compile_options(/W4)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Target
function(target_no_warning TARGET WNAME)

    # message(STATUS "target_no_warning TARGET: ${TARGET} WNAME: ${WNAME}" )

    set(MSVC_Warning)
    set(GCC_Warning ${WNAME})

    if (WNAME STREQUAL "-Wno-multichar")
        set(MSVC_Warning )
    elseif(WNAME STREQUAL "-Wimplicit-fallthrough=0")

    elseif(WNAME STREQUAL "-Wno-unused-parameter")

    elseif(WNAME STREQUAL "-Wno-unused-variable")

    elseif(WNAME STREQUAL "-Wunused-const-variable=0")

    elseif(WNAME STREQUAL "-Wno-type-limits")

    elseif(WNAME STREQUAL "-Wno-unknown-pragmas")

    elseif(WNAME STREQUAL "-Wfloat-conversion")
        set(MSVC_Warning /wd4244)
    # Only MSVC warnings
    elseif(WNAME STREQUAL "-WMSVC-no-translation-unit-is-empty")
        unset(GCC_Warning)
        set(MSVC_Warning /wd4206)
    else()
        message(WARNING "Unknown warning: ${WNAME}, please add this warning to ${CMAKE_CURRENT_LIST_DIR}/SetupCompileWarnings.cmake")
    endif()

    if (MSVC)
        target_compile_options(${TARGET} PRIVATE ${MSVC_Warning})
    else()
        target_compile_options(${TARGET} PRIVATE ${GCC_Warning})
    endif()

endfunction()

# Temporary solution
# Pavel added the `deprecated` attribute to the old mid event, hoping that the use of deprecated methods will be quickly removed.
# But! This is not yet true, the methods are used.
# Displaying warnings is very annoying for all developers.
# Pavel insists on keep these warnings.
# So we will keep them only for Pavel until he removes the use of obsolete methods.
find_program(GIT_EXECUTABLE git PATHS ENV PATH)
if (GIT_EXECUTABLE)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" config --get user.email
        OUTPUT_VARIABLE git_email
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    if (git_email STREQUAL "p.smokotnin@gmail.com")
        add_definitions(-DSHOW_MIDI_EVENT_DEPRECATED_WARNING)
    endif()

endif (GIT_EXECUTABLE)
