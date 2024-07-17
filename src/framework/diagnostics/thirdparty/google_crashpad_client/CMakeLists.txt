if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CRASHPAD_PATH ${CMAKE_CURRENT_LIST_DIR})
set(CHROMIUM_PATH ${CRASHPAD_PATH}/third_party/mini_chromium/mini_chromium)

set(CRASHPAD_INC
    ${CRASHPAD_PATH}
    ${CRASHPAD_PATH}/third_party/mini_chromium/mini_chromium
)

set(CRASHPAD_COMMON_SRC
    ${CHROMIUM_PATH}/base/logging.cc
    ${CHROMIUM_PATH}/base/rand_util.cc
    ${CHROMIUM_PATH}/base/files/file_path.cc
    ${CHROMIUM_PATH}/base/strings/stringprintf.cc

    ${CRASHPAD_PATH}/client/crash_report_database.cc
    ${CRASHPAD_PATH}/client/settings.cc

    ${CRASHPAD_PATH}/util/misc/uuid.cc
    ${CRASHPAD_PATH}/util/misc/metrics.cc

    ${CRASHPAD_PATH}/util/file/file_writer.cc
    ${CRASHPAD_PATH}/util/file/file_reader.cc
)

set(CRASHPAD_COMMON_SRC
    ${CHROMIUM_PATH}/base/logging.cc
    ${CHROMIUM_PATH}/base/rand_util.cc
    ${CHROMIUM_PATH}/base/files/file_path.cc
    ${CHROMIUM_PATH}/base/strings/stringprintf.cc

    ${CRASHPAD_PATH}/client/crash_report_database.cc
    ${CRASHPAD_PATH}/client/settings.cc

    ${CRASHPAD_PATH}/util/misc/uuid.cc
    ${CRASHPAD_PATH}/util/misc/metrics.cc

    ${CRASHPAD_PATH}/util/file/file_writer.cc
    ${CRASHPAD_PATH}/util/file/file_reader.cc
)

if(LINUX)
    set(CRASHPAD_SRC

        ${CHROMIUM_PATH}/base/files/scoped_file.cc
        ${CHROMIUM_PATH}/base/files/file_util_posix.cc
        ${CHROMIUM_PATH}/base/strings/string_number_conversions.cc
        ${CHROMIUM_PATH}/base/posix/safe_strerror.cc

        ${CRASHPAD_PATH}/client/client_argv_handling.cc
        ${CRASHPAD_PATH}/client/crashpad_client_linux.cc
        ${CRASHPAD_PATH}/client/crash_report_database_generic.cc

        ${CRASHPAD_PATH}/util/file/file_io.cc
        ${CRASHPAD_PATH}/util/file/file_io_posix.cc
        ${CRASHPAD_PATH}/util/file/filesystem_posix.cc
        ${CRASHPAD_PATH}/util/file/directory_reader_posix.cc
        ${CRASHPAD_PATH}/util/file/scoped_remove_file.cc

        ${CRASHPAD_PATH}/util/misc/random_string.cc
        ${CRASHPAD_PATH}/util/misc/initialization_state_dcheck.cc

        ${CRASHPAD_PATH}/util/posix/signals.cc
        ${CRASHPAD_PATH}/util/posix/close_multiple.cc
        ${CRASHPAD_PATH}/util/posix/double_fork_and_exec.cc
        ${CRASHPAD_PATH}/util/posix/scoped_mmap.cc
        ${CRASHPAD_PATH}/util/posix/scoped_dir.cc

        ${CRASHPAD_PATH}/util/linux/scoped_pr_set_dumpable.cc
        ${CRASHPAD_PATH}/util/linux/scoped_pr_set_ptracer.cc
        ${CRASHPAD_PATH}/util/linux/exception_handler_client.cc
        ${CRASHPAD_PATH}/util/linux/exception_handler_protocol.cc
        ${CRASHPAD_PATH}/util/linux/socket.cc
        ${CRASHPAD_PATH}/util/linux/ptrace_broker.cc
        ${CRASHPAD_PATH}/util/linux/ptracer.cc
        ${CRASHPAD_PATH}/util/linux/scoped_ptrace_attach.cc
        ${CRASHPAD_PATH}/util/linux/thread_info.cc

        ${CRASHPAD_PATH}/compat/linux/sys/mman_memfd_create.cc
    )

    set(CRASHPAD_DEF
        -DCRASHPAD_LSS_SOURCE_EMBEDDED
    )

    set(CRASHPAD_INC
        ${CRASHPAD_INC}
        ${CRASHPAD_PATH}/compat/linux
    )

    set(CRASHPAD_LINK
        pthread
        dl
    )

endif(LINUX)

if(WIN32)
    set(CRASHPAD_SRC

        ${CHROMIUM_PATH}/base/synchronization/lock.cc
        ${CHROMIUM_PATH}/base/synchronization/lock_impl_win.cc
        ${CHROMIUM_PATH}/base/process/memory.cc
        ${CHROMIUM_PATH}/base/strings/utf_string_conversions.cc
        ${CHROMIUM_PATH}/base/strings/utf_string_conversion_utils.cc
        ${CHROMIUM_PATH}/base/strings/string_util_win.cc
        ${CHROMIUM_PATH}/base/scoped_clear_last_error_win.cc
        ${CHROMIUM_PATH}/base/third_party/icu/icu_utf.cc

        ${CRASHPAD_PATH}/client/crashpad_client_win.cc
        ${CRASHPAD_PATH}/client/crash_report_database_win.cc

        ${CRASHPAD_PATH}/util/win/scoped_process_suspend.cc
        ${CRASHPAD_PATH}/util/win/registration_protocol_win.cc
        ${CRASHPAD_PATH}/util/win/nt_internals.cc
        ${CRASHPAD_PATH}/util/win/scoped_local_alloc.cc
        ${CRASHPAD_PATH}/util/win/process_info.cc
        ${CRASHPAD_PATH}/util/win/get_function.cc
        ${CRASHPAD_PATH}/util/win/ntstatus_logging.cc
        ${CRASHPAD_PATH}/util/win/handle.cc
        ${CRASHPAD_PATH}/util/win/loader_lock.cc
        ${CRASHPAD_PATH}/util/win/initial_client_data.cc
        ${CRASHPAD_PATH}/util/win/command_line.cc
        ${CRASHPAD_PATH}/util/win/scoped_handle.cc
        ${CRASHPAD_PATH}/util/win/critical_section_with_debug_info.cc

        ${CRASHPAD_PATH}/util/stdlib/aligned_allocator.cc
        ${CRASHPAD_PATH}/util/stdlib/string_number_conversion.cc

        ${CRASHPAD_PATH}/util/string/split_string.cc

        ${CRASHPAD_PATH}/util/file/file_io.cc
        ${CRASHPAD_PATH}/util/file/file_io_win.cc
        ${CRASHPAD_PATH}/util/file/filesystem_win.cc
        ${CRASHPAD_PATH}/util/file/directory_reader_win.cc
        ${CRASHPAD_PATH}/util/file/scoped_remove_file.cc

        ${CRASHPAD_PATH}/util/misc/random_string.cc
        ${CRASHPAD_PATH}/util/misc/initialization_state_dcheck.cc
        ${CRASHPAD_PATH}/util/misc/time_win.cc

        ${CRASHPAD_PATH}/util/misc/capture_context_win.asm
    )

    enable_language(ASM_MASM)

    set(CRASHPAD_DEF
        -DOS_WIN
        -DNOMINMAX
        -DUNICODE
        -DWIN32_LEAN_AND_MEAN
        -DWIN32
    )

    set(CRASHPAD_INC
        ${CRASHPAD_INC}
        ${CRASHPAD_PATH}/compat/win
    )
endif(WIN32)

if(APPLE)
    set(CRASHPAD_SRC
        ${CHROMIUM_PATH}/base/files/scoped_file.cc
        ${CHROMIUM_PATH}/base/files/file_util_posix.cc
        ${CHROMIUM_PATH}/base/posix/safe_strerror.cc
        ${CHROMIUM_PATH}/base/mac/mach_logging.cc
        ${CHROMIUM_PATH}/base/mac/scoped_mach_port.cc
        ${CHROMIUM_PATH}/base/mac/scoped_nsautorelease_pool.mm
        ${CHROMIUM_PATH}/base/mac/scoped_mach_vm.cc
        ${CHROMIUM_PATH}/base/mac/foundation_util.mm
        ${CHROMIUM_PATH}/base/strings/string_number_conversions.cc
        ${CHROMIUM_PATH}/base/strings/sys_string_conversions_mac.mm

        ${CRASHPAD_PATH}/client/crashpad_client_mac.cc
        ${CRASHPAD_PATH}/client/crash_report_database_mac.mm

        ${CRASHPAD_PATH}/util/posix/double_fork_and_exec.cc
        ${CRASHPAD_PATH}/util/posix/close_multiple.cc
        ${CRASHPAD_PATH}/util/posix/scoped_dir.cc

        ${CRASHPAD_PATH}/util/file/file_io.cc
        ${CRASHPAD_PATH}/util/file/file_io_posix.cc
        ${CRASHPAD_PATH}/util/file/scoped_remove_file.cc
        ${CRASHPAD_PATH}/util/file/filesystem_posix.cc
        ${CRASHPAD_PATH}/util/file/directory_reader_posix.cc

        ${CRASHPAD_PATH}/util/misc/clock_mac.cc
        ${CRASHPAD_PATH}/util/misc/clock_posix.cc
        ${CRASHPAD_PATH}/util/misc/initialization_state_dcheck.cc
        ${CRASHPAD_PATH}/util/misc/random_string.cc

        ${CRASHPAD_PATH}/util/mac/xattr.cc
        ${CRASHPAD_PATH}/util/mac/mac_util.cc
        ${CRASHPAD_PATH}/util/mac/sysctl.cc

        ${CRASHPAD_PATH}/util/mach/bootstrap.cc
        ${CRASHPAD_PATH}/util/mach/notify_server.cc
        ${CRASHPAD_PATH}/util/mach/mach_message.cc
        ${CRASHPAD_PATH}/util/mach/child_port_handshake.cc
        ${CRASHPAD_PATH}/util/mach/child_port_server.cc
        ${CRASHPAD_PATH}/util/mach/exception_ports.cc
        ${CRASHPAD_PATH}/util/mach/composite_mach_message_server.cc
        ${CRASHPAD_PATH}/util/mach/mach_message_server.cc
        ${CRASHPAD_PATH}/util/mach/mach_extensions.cc

        ${CRASHPAD_PATH}/gen/util/mach/child_portServer.c
        ${CRASHPAD_PATH}/gen/util/mach/child_portUser.c
        ${CRASHPAD_PATH}/gen/util/mach/excServer.c
        ${CRASHPAD_PATH}/gen/util/mach/excUser.c
        ${CRASHPAD_PATH}/gen/util/mach/mach_excServer.c
        ${CRASHPAD_PATH}/gen/util/mach/mach_excUser.c
        ${CRASHPAD_PATH}/gen/util/mach/notifyServer.c
        ${CRASHPAD_PATH}/gen/util/mach/notifyUser.c
    )

    set(CRASHPAD_INC
        ${CRASHPAD_PATH}/gen
        ${CRASHPAD_INC}
    )

    find_library(ApplicationServicesLib ApplicationServices)
    find_library(CoreFoundationLib CoreFoundation)
    find_library(FoundationLib Foundation)
    find_library(IOKitLib IOKit)
    find_library(SecurityLib Security)

    set(CRASHPAD_LINK
        ${ApplicationServicesLib}
        ${CoreFoundationLib}
        ${FoundationLib}
        ${SecurityLib}
        ${IOKitLib}
        pthread
        bsm
    )

endif(APPLE)

add_library(gcrashpad STATIC
    ${CRASHPAD_COMMON_SRC}
    ${CRASHPAD_SRC}
)

target_compile_definitions(gcrashpad PRIVATE ${CRASHPAD_DEF} )
target_include_directories(gcrashpad PRIVATE ${CRASHPAD_INC} )
target_link_libraries(gcrashpad ${CRASHPAD_LINK} )
if (NOT MSVC)
    set_target_properties(gcrashpad PROPERTIES COMPILE_FLAGS "-fPIC")
endif (NOT MSVC)

#target_no_warning(gcrashpad -Wno-conversion)
target_no_warning(gcrashpad -Wno-multichar)
target_no_warning(gcrashpad -Wno-unused-parameter)
target_no_warning(gcrashpad -Wno-deprecated)
target_no_warning(gcrashpad -Wno-attributes)
target_no_warning(gcrashpad -Wno-restrict)
target_no_warning(gcrashpad -WMSVC-no-nonstandard-extension-used)
