
if (NOT MINGW AND NOT MSVC AND NOT APPLE)
    #### PACKAGING for Linux and BSD based systems (more in mscore/CMakeLists.txt) ####
    #
    #     set library search path for runtime linker to load the same
    #     qt libraries as we used at compile time
    #
    string(TOUPPER "mscore${MSCORE_INSTALL_SUFFIX}" MAN_MSCORE_UPPER) # Command name shown in uppercase in man pages by convention
    if (${MSCORE_INSTALL_SUFFIX} MATCHES "portable") # Note: "-portable-anything" would match
      # Build portable AppImage as per https://github.com/probonopd/AppImageKit
      add_subdirectory(build/Linux+BSD/portable)
      if (NOT DEFINED ARCH)
        execute_process(COMMAND arch OUTPUT_VARIABLE ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)# Get architecture (strip trailing newline)
      endif (NOT DEFINED ARCH)
      get_filename_component(PORTABLE_INSTALL_PATH ${CMAKE_INSTALL_PREFIX} PATH)# Get path (dirname)
      get_filename_component(PORTABLE_INSTALL_NAME ${CMAKE_INSTALL_PREFIX} NAME)# Strip path (basename)
      if (NOT MSCORE_UNSTABLE)
        set(PORTABLE_INSTALL_NAME "${PORTABLE_INSTALL_NAME}-${MUSESCORE_VERSION_FULL}") # Append version info.
      endif (NOT MSCORE_UNSTABLE)
      set(PORTABLE_INSTALL_NAME "${PORTABLE_INSTALL_NAME}-${ARCH}") # Append system architecture.
      set(CMAKE_INSTALL_PREFIX ${PORTABLE_INSTALL_PATH}/${PORTABLE_INSTALL_NAME}.AppDir) # E.g. "MuseScore-X.Y.Z-x86_64.AppDir"
      execute_process(COMMAND echo ${CMAKE_INSTALL_PREFIX} OUTPUT_FILE PREFIX.txt)
      # Prepare portable scripts:
      configure_file(build/Linux+BSD/portable/AppRun.in          AppRun         @ONLY)
      configure_file(build/Linux+BSD/portable/portable-utils.in  portable-utils @ONLY)
      install(PROGRAMS ${PROJECT_BINARY_DIR}/AppRun     DESTINATION .   COMPONENT portable)
      install(PROGRAMS ${PROJECT_BINARY_DIR}/portable-utils
                       build/Linux+BSD/portable/ldd-recursive
                       build/rm-empty-dirs              DESTINATION bin COMPONENT portable)
      install(FILES    build/Linux+BSD/portable/qt.conf DESTINATION bin COMPONENT portable)
    else (${MSCORE_INSTALL_SUFFIX} MATCHES "portable")
      set(MAN_PORTABLE .\\\") # comment out lines in man page that are only relevant to the portable version
    endif (${MSCORE_INSTALL_SUFFIX} MATCHES "portable")
    # Install desktop file (perform variable substitution first)
    configure_file(build/Linux+BSD/mscore.desktop.in mscore${MSCORE_INSTALL_SUFFIX}.desktop)
    install( FILES ${PROJECT_BINARY_DIR}/mscore${MSCORE_INSTALL_SUFFIX}.desktop DESTINATION share/applications)
    # Install appdata file (perform variable substitution first)
    configure_file(build/Linux+BSD/org.musescore.MuseScore.appdata.xml.in org.musescore.MuseScore${MSCORE_INSTALL_SUFFIX}.appdata.xml)
    install( FILES ${PROJECT_BINARY_DIR}/org.musescore.MuseScore${MSCORE_INSTALL_SUFFIX}.appdata.xml DESTINATION share/metainfo)
    # Substitute variables within man pages
    set(MAN_NAME mscore)
    set(MAN_ALIAS musescore)
    set(MAN_EXTENSION .1)
    set(MAN_FULL_NAME ${MAN_NAME}${MSCORE_INSTALL_SUFFIX}${MAN_EXTENSION})
    set(MAN_FULL_ALIAS ${MAN_ALIAS}${MSCORE_INSTALL_SUFFIX}${MAN_EXTENSION})
    set(MAN_TARGET ${PROJECT_SOURCE_DIR}/build/Linux+BSD/${MAN_NAME}${MAN_EXTENSION}.in)
    set(MAN_BUILD ${PROJECT_BINARY_DIR}/${MAN_FULL_NAME})
    configure_file(${MAN_TARGET} ${MAN_BUILD})
    # Compress man pages if gzip is installed (don't on OpenBSD)
    # Note: Compressing man pages is normal on Linux but not OpenBSD
    find_program( GZIP_EXECUTABLE gzip DOC "A tool for compressing manpages (optional)." )
    if (GZIP_EXECUTABLE AND NOT CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
        message(STATUS "Found 'gzip'. Man pages will be compressed.")
        set(MAN_TARGET ${MAN_BUILD})
        set(MAN_EXTENSION ${MAN_EXTENSION}.gz)
        set(MAN_FULL_NAME ${MAN_NAME}${MSCORE_INSTALL_SUFFIX}${MAN_EXTENSION})
        set(MAN_FULL_ALIAS ${MAN_ALIAS}${MSCORE_INSTALL_SUFFIX}${MAN_EXTENSION})
        set(MAN_BUILD ${PROJECT_BINARY_DIR}/${MAN_FULL_NAME})
        add_custom_command(
            OUTPUT ${MAN_BUILD}
            DEPENDS ${MAN_TARGET}
            COMMAND ${GZIP_EXECUTABLE} -9 < ${MAN_TARGET} > ${MAN_BUILD}
            )
        add_custom_target(manpages ALL
            DEPENDS ${MAN_BUILD}
            COMMAND echo "Man pages have been compressed ready for installation."
            VERBATIM
            )
    else (GZIP_EXECUTABLE AND NOT CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
        if (CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
            message(STATUS "System is OpenBSD: Man pages will not be compressed.")
        else (CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
            message(STATUS "'gzip' not found (it is optional). Man pages will not be compressed.")
        endif (CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
        add_custom_target(manpages ALL
            COMMAND echo "Man pages will be installed uncompressed."
            VERBATIM
            )
    endif (GZIP_EXECUTABLE AND NOT CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
    # Install man pages in either compressed or uncompressed form
    install( FILES ${MAN_BUILD} DESTINATION share/man/man1 COMPONENT doc)
    # Create symlink alias for man pages so `man musescore` = `man mscore`
    find_program( LN_EXECUTABLE ln DOC "A tool for creating symbolic link aliases (optional)." )
    if (LN_EXECUTABLE)
        message(STATUS "Found 'ln'. Symlink aliases will be created for MuseScore executable and the man pages.")
        add_custom_command(
            TARGET manpages
            COMMAND echo "Creating symlink alias for man pages."
            COMMAND ${LN_EXECUTABLE} -sf "${MAN_FULL_NAME}" "${MAN_FULL_ALIAS}"
            COMMAND echo 'Symlink alias: ${MAN_FULL_ALIAS} -> ${MAN_FULL_NAME}'
            )
        install( FILES ${PROJECT_BINARY_DIR}/${MAN_FULL_ALIAS} DESTINATION share/man/man1 COMPONENT doc)
     else (LN_EXECUTABLE)
         message(STATUS "'ln' not found (it is optional). No symlink aliases will be created.")
     endif (LN_EXECUTABLE)
    # Add .MSCZ and .MSCX to MIME database (informs system that filetypes .MSCZ & .MSCX are MuseScore files)
    configure_file(build/Linux+BSD/musescore.xml.in musescore${MSCORE_INSTALL_SUFFIX}.xml)
    install( FILES ${PROJECT_BINARY_DIR}/musescore${MSCORE_INSTALL_SUFFIX}.xml DESTINATION share/mime/packages COMPONENT doc)
    # Note: Must now run "update-mime-database" to apply changes.
endif (NOT MINGW AND NOT MSVC AND NOT APPLE)
