#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
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
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

trap 'echo Build failed; exit 1' ERR

if [ $(which nproc) ]; then
    JOBS=$(nproc --all)
else
    JOBS=4
fi
TARGET=release

CMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES:-""}
MUSESCORE_MACOS_DEPS_PATH=${MUSESCORE_MACOS_DEPS_PATH:-""}
MUSESCORE_INSTALL_DIR=${MUSESCORE_INSTALL_DIR:-"../build.install"}
MUSE_APP_INSTALL_SUFFIX=${MUSE_APP_INSTALL_SUFFIX:-""}
MUSESCORE_BUILD_CONFIGURATION=${MUSESCORE_BUILD_CONFIGURATION:-"app"}
MUSE_APP_BUILD_MODE=${MUSE_APP_BUILD_MODE:-"dev"}
MUSESCORE_BUILD_NUMBER=${MUSESCORE_BUILD_NUMBER:-"12345678"}
MUSESCORE_REVISION=${MUSESCORE_REVISION:-"abc123456"}
MUSESCORE_RUN_LRELEASE=${MUSESCORE_RUN_LRELEASE:-"ON"}
MUSESCORE_CRASHREPORT_URL=${MUSESCORE_CRASHREPORT_URL:-""}
MUSESCORE_BUILD_CRASHPAD_CLIENT=${MUSESCORE_BUILD_CRASHPAD_CLIENT:-"ON"}
MUSESCORE_DEBUGLEVEL_ENABLED="OFF"
MUSESCORE_DOWNLOAD_SOUNDFONT=${MUSESCORE_DOWNLOAD_SOUNDFONT:-"ON"}
MUSESCORE_BUILD_UNIT_TESTS=${MUSESCORE_BUILD_UNIT_TESTS:-"OFF"}
MUSESCORE_ENABLE_CODE_COVERAGE=${MUSESCORE_UNIT_TESTS_ENABLE_CODE_COVERAGE:-"OFF"}
MUSESCORE_NO_RPATH=${MUSESCORE_NO_RPATH:-"OFF"}
MUSESCORE_MODULE_UPDATE=${MUSESCORE_MODULE_UPDATEE:-"ON"}
MUSESCORE_BUILD_VST_MODULE=${MUSESCORE_BUILD_VST_MODULE:-"OFF"}
MUSESCORE_BUILD_VIDEOEXPORT_MODULE=${MUSESCORE_BUILD_VIDEOEXPORT_MODULE:-"OFF"}
MUSESCORE_BUILD_WEBSOCKET=${MUSESCORE_BUILD_WEBSOCKET:-"OFF"}
MUSESCORE_BUILD_MULTIMEDIA=${MUSESCORE_BUILD_MULTIMEDIA:-"OFF"}
MUSESCORE_COMPILE_USE_UNITY=${MUSESCORE_COMPILE_USE_UNITY:-"ON"}

SHOW_HELP=0
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -t|--target) TARGET="$2"; shift;;
        -j|--jobs) JOBS="$2"; shift;;
        -h|--help) SHOW_HELP=1;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ $SHOW_HELP -eq 1 ]; then
    echo -e "Usage: ${0}\n" \
        "\t-t, --target <string> [default: ${TARGET}]\n" \
        "\t\tProvided targets: \n" \
        "\t\trelease, debug, relwithdebinfo, install, installrelwithdebinfo, \n" \
        "\t\tinstalldebug, clean, compile_commands, revision, appimage\n" \
        "\t-j, --jobs <number> [default: ${JOBS}]\n" \
        "\t\t Number of parallel compilations jobs\n" \
        "\t-h, --help\n" \
        "\t\t Show this help"
    exit 0
fi

cmake --version
echo "ninja version $(ninja --version)"

function do_build() {
    BUILD_TYPE=$1

    cmake .. -GNinja \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DCMAKE_OSX_ARCHITECTURES="${CMAKE_OSX_ARCHITECTURES}" \
        -DMUE_COMPILE_MACOS_PRECOMPILED_DEPS_PATH="${MUSESCORE_MACOS_DEPS_PATH}" \
        -DCMAKE_INSTALL_PREFIX="${MUSESCORE_INSTALL_DIR}" \
        -DMUSE_APP_INSTALL_SUFFIX="${MUSE_APP_INSTALL_SUFFIX}" \
        -DMUSESCORE_BUILD_CONFIGURATION="${MUSESCORE_BUILD_CONFIGURATION}" \
        -DMUSE_APP_BUILD_MODE="${MUSE_APP_BUILD_MODE}" \
        -DCMAKE_BUILD_NUMBER="${MUSESCORE_BUILD_NUMBER}" \
        -DMUSESCORE_REVISION="${MUSESCORE_REVISION}" \
        -DMUE_RUN_LRELEASE="${MUSESCORE_RUN_LRELEASE}" \
        -DMUE_BUILD_VIDEOEXPORT_MODULE="${MUSESCORE_BUILD_VIDEOEXPORT_MODULE}" \
        -DMUSE_MODULE_UPDATE="${MUSESCORE_MODULE_UPDATE}" \
        -DMUE_DOWNLOAD_SOUNDFONT="${MUSESCORE_DOWNLOAD_SOUNDFONT}" \
        -DMUSE_ENABLE_UNIT_TESTS="${MUSESCORE_BUILD_UNIT_TESTS}" \
        -DMUSE_ENABLE_UNIT_TESTS_CODE_COVERAGE="${MUSESCORE_UNIT_TESTS_ENABLE_CODE_COVERAGE}" \
        -DMUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT="${MUSESCORE_BUILD_CRASHPAD_CLIENT}" \
        -DMUSE_MODULE_DIAGNOSTICS_CRASHREPORT_URL="${MUSESCORE_CRASHREPORT_URL}" \
        -DMUSE_MODULE_GLOBAL_LOGGER_DEBUGLEVEL="${MUSESCORE_DEBUGLEVEL_ENABLED}" \
        -DMUSE_MODULE_VST="${MUSESCORE_BUILD_VST_MODULE}" \
        -DMUSE_MODULE_NETWORK_WEBSOCKET="${MUSESCORE_BUILD_WEBSOCKET}" \
        -DMUSE_MODULE_MULTIMEDIA="${MUSESCORE_BUILD_MULTIMEDIA}" \
        -DCMAKE_SKIP_RPATH="${MUSESCORE_NO_RPATH}" \
        -DMUSE_COMPILE_USE_UNITY="${MUSESCORE_COMPILE_USE_UNITY}"

    ninja -j $JOBS
}

case $TARGET in
    release)
        mkdir -p build.release
        cd build.release
        do_build Release
        ;;

    debug)
        mkdir -p build.debug
        cd build.debug
        do_build Debug
        ;;

    relwithdebinfo)
        mkdir -p build.release
        cd build.release
        do_build RelWithDebInfo
        ;;

    install)
        mkdir -p build.release
        cd build.release
        do_build Release
        ninja install
        ;;

    installrelwithdebinfo)
        mkdir -p build.release
        cd build.release
        do_build RelWithDebInfo
        ninja install
        ;;

    installdebug)
        mkdir -p build.debug
        cd build.debug
        do_build Debug
        ninja install
        ;;

    clean)
        rm -rf build.debug build.release
        ;;

    compile_commands)
        # Generate compile_commands.json file (https://clang.llvm.org/docs/JSONCompilationDatabase.html)
        mkdir -p build.tooldata
        cd build.tooldata
        cmake .. -GNinja \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
            -DMUSE_COMPILE_USE_UNITY=OFF \
            -DCMAKE_BUILD_TYPE="Debug" \
            -DCMAKE_OSX_ARCHITECTURES="${CMAKE_OSX_ARCHITECTURES}" \
            -DMUE_COMPILE_MACOS_PRECOMPILED_DEPS_PATH="${MUSESCORE_MACOS_DEPS_PATH}" \
            -DCMAKE_INSTALL_PREFIX="${MUSESCORE_INSTALL_DIR}" \
            -DMUSE_APP_INSTALL_SUFFIX="${MUSE_APP_INSTALL_SUFFIX}" \
            -DMUSESCORE_BUILD_CONFIGURATION="${MUSESCORE_BUILD_CONFIGURATION}" \
            -DMUSE_APP_BUILD_MODE="${MUSE_APP_BUILD_MODE}" \
            -DCMAKE_BUILD_NUMBER="${MUSESCORE_BUILD_NUMBER}" \
            -DMUSESCORE_REVISION="${MUSESCORE_REVISION}" \
            -DMUE_RUN_LRELEASE="${MUSESCORE_RUN_LRELEASE}" \
            -DMUE_BUILD_VIDEOEXPORT_MODULE="${MUSESCORE_BUILD_VIDEOEXPORT_MODULE}" \
            -DMUSE_MODULE_UPDATE="${MUSESCORE_MODULE_UPDATE}" \
            -DMUE_DOWNLOAD_SOUNDFONT="${MUSESCORE_DOWNLOAD_SOUNDFONT}" \
            -DMUSE_ENABLE_UNIT_TESTS="${MUSESCORE_BUILD_UNIT_TESTS}" \
            -DMUSE_ENABLE_UNIT_TESTS_CODE_COVERAGE="${MUSESCORE_UNIT_TESTS_ENABLE_CODE_COVERAGE}" \
            -DMUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT="${MUSESCORE_BUILD_CRASHPAD_CLIENT}" \
            -DMUSE_MODULE_DIAGNOSTICS_CRASHREPORT_URL="${MUSESCORE_CRASHREPORT_URL}" \
            -DMUSE_MODULE_GLOBAL_LOGGER_DEBUGLEVEL="${MUSESCORE_DEBUGLEVEL_ENABLED}" \
            -DMUSE_MODULE_VST="${MUSESCORE_BUILD_VST_MODULE}" \
            -DCMAKE_SKIP_RPATH="${MUSESCORE_NO_RPATH}"
        ;;

    revision)
        git rev-parse --short=7 HEAD | tr -d '\n' >local_build_revision.env
        ;;

    appimage)
        MUSESCORE_INSTALL_DIR=../MuseScore
        MUSE_APP_INSTALL_SUFFIX="4portable${MUSE_APP_INSTALL_SUFFIX}" # e.g. "4portable" or "4portablenightly"
        MUSESCORE_NO_RPATH=ON

        mkdir -p build.release
        cd build.release
        do_build RELEASE
        ninja install

        build_dir="$(pwd)"
        install_dir="$(cat $build_dir/PREFIX.txt)"
        cd $install_dir

        ln -sf . usr # we installed into the root of our AppImage but some tools expect a "usr" subdirectory
        mscore="mscore${MUSE_APP_INSTALL_SUFFIX}"
        desktop="org.musescore.MuseScore${MUSE_APP_INSTALL_SUFFIX}.desktop"
        icon="${mscore}.png"
        mani="install_manifest.txt"
        cp "share/applications/${desktop}" "${desktop}"
        cp "share/icons/hicolor/128x128/apps/${icon}" "${icon}"
        sed <"$build_dir/${mani}" >"${mani}" -rn 's/.*(share\/)(applications|icons|man|metainfo|mime)(.*)/\1\2\3/p'
        ;;

    appimagedebug)
        MUSESCORE_INSTALL_DIR=../MuseScore
        MUSE_APP_INSTALL_SUFFIX="4portable${MUSE_APP_INSTALL_SUFFIX}" # e.g. "4portable" or "4portablenightly"
        MUSESCORE_NO_RPATH=ON

        mkdir -p build.debug
        cd build.debug
        do_build Debug
        ninja install

        build_dir="$(pwd)"
        install_dir="$(cat $build_dir/PREFIX.txt)"
        cd $install_dir

        ln -sf . usr # we installed into the root of our AppImage but some tools expect a "usr" subdirectory
        mscore="mscore${MUSE_APP_INSTALL_SUFFIX}"
        desktop="org.musescore.MuseScore${MUSE_APP_INSTALL_SUFFIX}.desktop"
        icon="${mscore}.png"
        mani="install_manifest.txt"
        cp "share/applications/${desktop}" "${desktop}"
        cp "share/icons/hicolor/128x128/apps/${icon}" "${icon}"
        sed <"$build_dir/${mani}" >"${mani}" -rn 's/.*(share\/)(applications|icons|man|metainfo|mime)(.*)/\1\2\3/p'
        ;;

    *)
        echo "Unknown target: $TARGET"
        exit 1
        ;;
esac
