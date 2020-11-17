if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CRASHPAD_PATH ${CMAKE_CURRENT_LIST_DIR})
set(CHROMIUM_PATH ${CRASHPAD_PATH}/third_party/mini_chromium/mini_chromium)

set(CRASHPAD_INC
    ${CRASHPAD_PATH}
    ${CRASHPAD_PATH}/third_party/mini_chromium/mini_chromium
)

if(LINUX)
    set(CRASHPAD_SRC
        ${CHROMIUM_PATH}/base/files/file_path.cc
        ${CHROMIUM_PATH}/base/files/scoped_file.cc
        ${CHROMIUM_PATH}/base/logging.cc
        ${CHROMIUM_PATH}/base/posix/safe_strerror.cc
        ${CHROMIUM_PATH}/base/strings/stringprintf.cc
        ${CHROMIUM_PATH}/base/strings/string_number_conversions.cc
        ${CHROMIUM_PATH}/base/rand_util.cc
        ${CHROMIUM_PATH}/base/files/file_util_posix.cc

        ${CRASHPAD_PATH}/client/client_argv_handling.cc
        ${CRASHPAD_PATH}/client/crashpad_client_linux.cc
        ${CRASHPAD_PATH}/client/crash_report_database.cc
        ${CRASHPAD_PATH}/client/crash_report_database_generic.cc
        ${CRASHPAD_PATH}/client/settings.cc

        ${CRASHPAD_PATH}/util/linux/scoped_pr_set_dumpable.cc
        ${CRASHPAD_PATH}/util/posix/signals.cc
        ${CRASHPAD_PATH}/util/linux/scoped_pr_set_ptracer.cc
        ${CRASHPAD_PATH}/util/linux/exception_handler_client.cc
        ${CRASHPAD_PATH}/util/linux/exception_handler_protocol.cc
        ${CRASHPAD_PATH}/util/linux/socket.cc
        ${CRASHPAD_PATH}/util/posix/double_fork_and_exec.cc
        ${CRASHPAD_PATH}/util/file/filesystem_posix.cc
        ${CRASHPAD_PATH}/util/posix/scoped_mmap.cc
        ${CRASHPAD_PATH}/util/file/file_io.cc
        ${CRASHPAD_PATH}/util/linux/ptrace_broker.cc
        ${CRASHPAD_PATH}/util/posix/close_multiple.cc
        ${CRASHPAD_PATH}/util/file/file_io_posix.cc
        ${CRASHPAD_PATH}/util/linux/ptracer.cc
        ${CRASHPAD_PATH}/util/linux/scoped_ptrace_attach.cc
        ${CRASHPAD_PATH}/util/file/directory_reader_posix.cc
        ${CRASHPAD_PATH}/util/linux/thread_info.cc
        ${CRASHPAD_PATH}/util/misc/random_string.cc
        ${CRASHPAD_PATH}/util/misc/initialization_state_dcheck.cc
        ${CRASHPAD_PATH}/util/posix/scoped_dir.cc
        ${CRASHPAD_PATH}/util/misc/uuid.cc
        ${CRASHPAD_PATH}/util/file/file_writer.cc
        ${CRASHPAD_PATH}/util/file/file_reader.cc
        ${CRASHPAD_PATH}/util/file/scoped_remove_file.cc
        ${CRASHPAD_PATH}/util/misc/metrics.cc
    )

    set(CRASHPAD_DEF
        -DCRASHPAD_LSS_SOURCE_EMBEDDED
    )

endif(LINUX)

if(WIN32)

endif(WIN32)

if(APPLE)

endif(APPLE)

add_library(gcrashpad STATIC ${BREAKPAD_SRC} )

target_compile_definitions(gcrashpad PUBLIC
    ${CRASHPAD_DEF}
)

target_include_directories(gcrashpad PUBLIC
    ${CRASHPAD_INC}
)

if(NOT MSVC)
    target_link_libraries(gcrashpad pthread)
endif()

