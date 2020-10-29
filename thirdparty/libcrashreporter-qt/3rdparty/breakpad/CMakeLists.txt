# PROJECT(breakpad)
# cmake_minimum_required(VERSION 2.8)
#
# set( CMAKE_MODULE_PATH
#      ${CMAKE_MODULE_PATH}
#      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
# )
#
# if( CMAKE_COMPILER_IS_GNUCXX )
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++98")
# endif()
#
# if(${CMAKE_BUILD_TYPE} MATCHES "Release")
#   add_definitions(-DNDEBUG)
# endif(${CMAKE_BUILD_TYPE} MATCHES "Release")

if(UNIX)
    list(APPEND breakpadSources
        client/minidump_file_writer.cc
        common/convert_UTF.c
        common/string_conversion.cc
    )

    if(APPLE)
        list(APPEND breakpadSources
            client/mac/crash_generation/crash_generation_client.cc
            client/mac/crash_generation/crash_generation_server.cc
            client/mac/handler/breakpad_nlist_64.cc
            client/mac/handler/dynamic_images.cc
            client/mac/handler/exception_handler.cc
            client/mac/handler/minidump_generator.cc
            client/mac/handler/protected_memory_allocator.cc
            common/mac/file_id.cc
            common/mac/macho_id.cc
            common/mac/macho_reader.cc
            common/mac/macho_utilities.cc
            common/mac/macho_walker.cc
            common/mac/string_utilities.cc
            common/md5.cc
            common/mac/dump_syms.mm
            common/mac/MachIPC.mm
            common/dwarf/dwarf2reader.cc
            common/mac/bootstrap_compat.cc
            common/dwarf_cfi_to_module.cc
        )
    else()
        list(APPEND breakpadSources
            client/linux/crash_generation/crash_generation_client.cc
            client/linux/handler/exception_handler.cc
            client/linux/handler/minidump_descriptor.cc
            client/linux/log/log.cc
            client/linux/minidump_writer/linux_dumper.cc
            client/linux/minidump_writer/linux_ptrace_dumper.cc
            client/linux/minidump_writer/minidump_writer.cc
            common/linux/elfutils.cc
            common/linux/file_id.cc
            common/linux/guid_creator.cc
            common/linux/linux_libc_support.cc
            common/linux/memory_mapped_file.cc
            common/linux/safe_readlink.cc
        )
    endif()
endif()

if(WIN32)
    add_definitions( -DUNICODE -fshort-wchar )
    list(APPEND breakpadSources
        client/windows/handler/exception_handler.cc
        client/windows/crash_generation/crash_generation_client.cc
        common/windows/guid_string.cc
    )
endif()


include_directories(.)
# ADD_DEFINITIONS( -fPIC )
add_library(crashreporter-breakpad STATIC ${breakpadSources} )

if(NOT MSVC)
    target_link_libraries(crashreporter-breakpad pthread)
endif()

if(WIN32 AND NOT MSVC)
    target_link_libraries(crashreporter-breakpad "mingwex")
endif()
