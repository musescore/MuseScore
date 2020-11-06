
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(LINUX)
    set(BREAKPAD_SRC
        common/convert_UTF.cc
        common/md5.cc
        common/string_conversion.cc
        common/linux/breakpad_getcontext.S
        common/linux/elfutils.cc
        common/linux/file_id.cc
        common/linux/guid_creator.cc
        common/linux/linux_libc_support.cc
        common/linux/memory_mapped_file.cc
        common/linux/safe_readlink.cc
        client/linux/crash_generation/crash_generation_client.cc
        client/linux/dump_writer_common/thread_info.cc
        client/linux/dump_writer_common/ucontext_reader.cc
        client/linux/handler/exception_handler.cc
        client/linux/handler/minidump_descriptor.cc
        client/linux/log/log.cc
        client/linux/microdump_writer/microdump_writer.cc
        client/linux/minidump_writer/linux_dumper.cc
        client/linux/minidump_writer/linux_ptrace_dumper.cc
        client/linux/minidump_writer/minidump_writer.cc
        client/minidump_file_writer.cc
    )

    enable_language(C ASM)
    set_source_files_properties(common/linux/breakpad_getcontext.S PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")

endif(LINUX)

if(WIN32)
    set(BREAKPAD_SRC
        common/string_conversion.cc
        common/convert_UTF.cc
        common/md5.cc
        common/windows/guid_string.cc
        client/windows/handler/exception_handler.cc
        client/windows/crash_generation/minidump_generator.cc
        client/windows/crash_generation/client_info.cc
        client/windows/crash_generation/crash_generation_client.cc
    )
endif(WIN32)

if(APPLE)
    set(BREAKPAD_SRC
        common/md5.cc
        common/string_conversion.cc
        common/convert_UTF.cc

        client/minidump_file_writer.cc

        client/mac/handler/exception_handler.h
        client/mac/handler/exception_handler.cc
        client/mac/handler/minidump_generator.cc
        client/mac/handler/dynamic_images.cc
        client/mac/handler/breakpad_nlist_64.cc

        client/mac/crash_generation/crash_generation_client.cc

        common/mac/string_utilities.cc
        common/mac/file_id.cc
        common/mac/macho_id.cc
        common/mac/macho_reader.cc
        common/mac/macho_utilities.cc
        common/mac/macho_walker.cc
        common/mac/MachIPC.mm
        common/mac/bootstrap_compat.cc
    )
endif(APPLE)


include_directories(.)
add_definitions(-D__STDC_FORMAT_MACROS)
# ADD_DEFINITIONS( -fPIC )
add_library(gbreakpad STATIC ${BREAKPAD_SRC} )

if(NOT MSVC)
    target_link_libraries(gbreakpad pthread)
endif()

if(WIN32 AND NOT MSVC)
    target_link_libraries(gbreakpad "mingwex")
endif()
