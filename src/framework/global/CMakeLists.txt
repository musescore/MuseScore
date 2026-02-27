# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_module(muse_global ALIAS muse::global)

include(thirdparty/kors_logger/logger.cmake)
include(thirdparty/kors_profiler/profiler/profiler.cmake)
include(thirdparty/kors_modularity/modularity/modularity.cmake)
include(thirdparty/utfcpp/utfcpp.cmake)

add_subdirectory(async)

target_sources(muse_global PRIVATE
    ${KORS_LOGGER_SRC}
    ${KORS_PROFILER_SRC}

    globaltypes.h
    iapplication.h
    iglobalconfiguration.h
    log.h
    logstream.h
    logger.h
    logremover.cpp
    logremover.h
    profiler.h
    dataformatter.cpp
    dataformatter.h
    stringutils.cpp
    stringutils.h
    ptrutils.h
    realfn.h
    runtime.cpp
    runtime.h
    translation.cpp
    translation.h
    timer.cpp
    timer.h
    progress.h
    utils.cpp
    utils.h
    interpolation.cpp
    interpolation.h
    defer.h
    containers.h
    icryptographichash.h
    allocator.cpp
    allocator.h
    dlib.h
    iprocess.h
    isysteminfo.h
    configreader.cpp
    configreader.h
    uuid.cpp
    uuid.h
    itickerprovider.h
    ticker.cpp
    ticker.h

    ${KORS_MODULARITY_SRC}
    modularity/ioccontext.cpp
    modularity/ioc.h
    modularity/imoduleinterface.h
    modularity/imodulesetup.h

    types/bytearray.cpp
    types/bytearray.h
    types/string.cpp
    types/string.h
    types/datetime.cpp
    types/datetime.h
    types/flags.h
    types/id.cpp
    types/id.h
    types/val.cpp
    types/val.h
    types/ret.cpp
    types/ret.h
    types/retval.h
    types/uri.cpp
    types/uri.h
    types/sharedhashmap.h
    types/sharedmap.h
    types/translatablestring.h
    types/mnemonicstring.cpp
    types/mnemonicstring.h
    types/version.cpp
    types/version.h
    types/number.h
    types/config.h
    types/ratio.h
    types/secs.h
    types/color.cpp
    types/color.h
    types/rgba.h

    io/ioenums.h
    io/path.cpp
    io/path.h
    io/iodevice.cpp
    io/iodevice.h
    io/file.cpp
    io/file.h
    io/filewatcher.cpp
    io/filewatcher.h
    io/buffer.cpp
    io/buffer.h
    io/ifilesystem.h
    io/ioretcodes.h
    io/fileinfo.cpp
    io/fileinfo.h
    io/dir.cpp
    io/dir.h

    io/devtools/allzerosbuffercorruptor.cpp
    io/devtools/allzerosbuffercorruptor.h

    serialization/zipreader.cpp
    serialization/zipreader.h
    serialization/zipwriter.cpp
    serialization/zipwriter.h
    serialization/internal/zipcontainer.cpp
    serialization/internal/zipcontainer.h

    serialization/textstream.cpp
    serialization/textstream.h
    serialization/json.cpp
    serialization/json.h
    serialization/msgpack.h
    serialization/msgpack_forward.h

    serialization/xmlstreamreader.cpp
    serialization/xmlstreamreader.h
    serialization/xmlstreamwriter.cpp
    serialization/xmlstreamwriter.h
    serialization/xmldom.cpp
    serialization/xmldom.h
)
if (MUE_COMPILE_USE_SYSTEM_PUGIXML)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(pugixml REQUIRED IMPORTED_TARGET pugixml)
    target_link_libraries(muse_global PRIVATE PkgConfig::pugixml)
    target_include_directories(muse_global PRIVATE ${pugixml_INCLUDE_DIRS})
else()
    target_sources(muse_global PRIVATE
        thirdparty/pugixml/pugixml.hpp
        thirdparty/pugixml/pugixml.cpp
    )
    target_include_directories(muse_global PRIVATE thirdparty/pugixml)
endif()

if (MUE_COMPILE_USE_SYSTEM_UTF8CPP)
    find_package(utf8cpp REQUIRED CONFIG)
    get_target_property(UTFCPP_INC utf8::cpp INTERFACE_INCLUDE_DIRECTORIES)
    # UTFCPP_INC is .../include/utf8cpp, and we want .../include so that
    # #include <utfcpp/utf8.h> resolves correctly
    get_filename_component(UTFCPP_INC_PARENT "${UTFCPP_INC}" DIRECTORY)
    message(STATUS "Using system utf8cpp from ${UTFCPP_INC_PARENT} (${UTFCPP_INC})")
    target_include_directories(muse_global PRIVATE ${UTFCPP_INC_PARENT})
    target_compile_definitions(muse_global PRIVATE SYSTEM_UTFCPP)
else()
    target_sources(muse_global PRIVATE ${UTFCPP_SRC})
endif()

if (MUSE_THREADS_SUPPORT)
    target_sources(muse_global PRIVATE
        concurrency/taskscheduler.h
        concurrency/concurrent.h
        concurrency/threadutils.h
        concurrency/ringqueue.h
        concurrency/rpcqueue.h
    )
endif()

if (GLOBAL_NO_INTERNAL)
    target_compile_definitions(muse_global PRIVATE GLOBAL_NO_INTERNAL)
else()
    target_sources(muse_global PRIVATE
        globalmodule.cpp
        globalmodule.h
        settings.cpp
        settings.h

        api/apitypes.h
        api/iapiregister.h
        api/iapiengine.h
        api/internal/apiregister.cpp
        api/internal/apiregister.h
        api/apiobject.cpp
        api/apiobject.h
        api/jsretval.h
        api/apiutils.h
        api/logapi.cpp
        api/logapi.h
        api/processapi.cpp
        api/processapi.h
        api/filesystemapi.cpp
        api/filesystemapi.h

        internal/baseapplication.cpp
        internal/baseapplication.h
        internal/globalconfiguration.cpp
        internal/globalconfiguration.h
        internal/cryptographichash.cpp
        internal/cryptographichash.h
        internal/process.cpp
        internal/process.h
        internal/systeminfo.cpp
        internal/systeminfo.h
        internal/tickerprovider.cpp
        internal/tickerprovider.h
    )

    include(GetPlatformInfo)

    if (OS_IS_WASM)
        target_sources(muse_global PRIVATE
            io/internal/memfilesystem.cpp
            io/internal/memfilesystem.h
        )
    else()
        target_sources(muse_global PRIVATE
            io/internal/filesystem.cpp
            io/internal/filesystem.h
        )
    endif()

    if (OS_IS_WIN)
        target_sources(muse_global PRIVATE
            platform/win/async.h
            platform/win/waitabletimer.h
        )
        target_link_libraries(muse_global PRIVATE
            WindowsApp # needed for C++/WinRT
            Winmm # needed for timeBeginPeriod
        )
    endif()
endif()

include(GetCompilerInfo)
set(Z_LIB )
if (CC_IS_MSVC)
    include(FindZlibStatic)
    target_link_libraries(muse_global PRIVATE zlibstat)
    target_include_directories(muse_global PRIVATE ${DEPENDENCIES_INC}/zlib)
elseif (CC_IS_EMCC)
    #zlib included in main linker
else ()
    target_link_libraries(muse_global PRIVATE z)
endif ()

target_include_directories(muse_global PRIVATE thirdparty/pugixml)

if (MUSE_QT_SUPPORT)
    # These are needed by so many modules, that we make them public here,
    # so that other modules get them transitively.
    target_link_libraries(muse_global PUBLIC Qt::Core Qt::Gui)

    # These are needed by the PCH; if we don't make them public, modules
    # that don't use them won't be able to use the PCH with some compilers.
    target_link_libraries(muse_global PUBLIC Qt::Quick Qt::Widgets)
endif()

target_link_libraries(muse_global PRIVATE ${CMAKE_DL_LIBS})

if (MUSE_ENABLE_CUSTOM_ALLOCATOR)
    target_compile_definitions(muse_global PRIVATE MUSE_ENABLE_CUSTOM_ALLOCATOR)
endif()

if (MUSE_MODULE_GLOBAL_LOGGER_DEBUGLEVEL)
    target_compile_definitions(muse_global PRIVATE MUSE_MODULE_GLOBAL_LOGGER_DEBUGLEVEL)
endif()

if (MUSE_MODULE_GLOBAL_MULTI_IOC)
    target_compile_definitions(muse_global PUBLIC MUSE_MODULE_GLOBAL_MULTI_IOC)
endif()

if (MUSE_MODULE_GLOBAL_TESTS)
    add_subdirectory(tests)
endif()
