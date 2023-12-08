#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
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

if [ $(which nproc) ]; then
    JOBS=$(nproc --all)
else
    JOBS=4
fi
TARGET=release

MUSESCORE_INSTALL_DIR=${MUSESCORE_INSTALL_DIR:-"../build.install"}
MUSESCORE_INSTALL_SUFFIX=${MUSESCORE_INSTALL_SUFFIX:-""}
MUSESCORE_BUILD_CONFIGURATION=${MUSESCORE_BUILD_CONFIGURATION:-"app"}
MUSESCORE_BUILD_MODE=${MUSESCORE_BUILD_MODE:-"dev"}
MUSESCORE_BUILD_NUMBER=${MUSESCORE_BUILD_NUMBER:-"12345678"}
MUSESCORE_REVISION=${MUSESCORE_REVISION:-"abc123456"}
MUSESCORE_RUN_LRELEASE=${MUSESCORE_RUN_LRELEASE:-"ON"}
MUSESCORE_CRASHREPORT_URL=${MUSESCORE_CRASHREPORT_URL:-""}
MUSESCORE_BUILD_CRASHPAD_CLIENT=${MUSESCORE_BUILD_CRASHPAD_CLIENT:-"ON"}
MUSESCORE_DEBUGLEVEL_ENABLED="OFF"
MUSESCORE_VST3_SDK_PATH=${MUSESCORE_VST3_SDK_PATH:-""}
MUSESCORE_DOWNLOAD_SOUNDFONT=${MUSESCORE_DOWNLOAD_SOUNDFONT:-"ON"}
MUSESCORE_BUILD_UNIT_TESTS=${MUSESCORE_BUILD_UNIT_TESTS:-"OFF"}
MUSESCORE_NO_RPATH=${MUSESCORE_NO_RPATH:-"OFF"}
MUSESCORE_YOUTUBE_API_KEY=${MUSESCORE_YOUTUBE_API_KEY:-""} 
MUSESCORE_BUILD_UPDATE_MODULE=${MUSESCORE_BUILD_UPDATE_MODULE:-"ON"}
MUSESCORE_BUILD_VST_MODULE=${MUSESCORE_BUILD_VST_MODULE:-"OFF"}
MUSESCORE_BUILD_VIDEOEXPORT_MODULE=${MUSESCORE_BUILD_VIDEOEXPORT_MODULE:-"OFF"}

SHOW_HELP=0
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -t|--target) TARGET="$2"; shift;;
        -j|--jobs) JOBS="$2"; shift;;
        -h|--help) SHOW_HELP=1; shift;;
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
	 "\t-h, --help\n"\
	 "\t\t Show this help"
    exit 0
fi

cmake --version
echo "ninja version $(ninja --version)"



function do_build() {

    BUILD_TYPE=$1

    cmake .. -GNinja \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DCMAKE_INSTALL_PREFIX="${MUSESCORE_INSTALL_DIR}" \
        -DMUSESCORE_INSTALL_SUFFIX="${MUSESCORE_INSTALL_SUFFIX}" \
        -DMUSESCORE_BUILD_CONFIGURATION="${MUSESCORE_BUILD_CONFIGURATION}" \
        -DMUSESCORE_BUILD_MODE="${MUSESCORE_BUILD_MODE}" \
        -DCMAKE_BUILD_NUMBER="${MUSESCORE_BUILD_NUMBER}" \
        -DMUSESCORE_REVISION="${MUSESCORE_REVISION}" \
        -DMUE_RUN_LRELEASE="${MUSESCORE_RUN_LRELEASE}" \
        -DMUE_BUILD_VST_MODULE="${MUSESCORE_BUILD_VST_MODULE}" \
        -DMUE_BUILD_VIDEOEXPORT_MODULE="${MUSESCORE_BUILD_VIDEOEXPORT_MODULE}" \
        -DMUE_LEARN_YOUTUBE_API_KEY="${MUSESCORE_YOUTUBE_API_KEY}" \
        -DMUE_BUILD_UPDATE_MODULE="${MUSESCORE_BUILD_UPDATE_MODULE}" \
        -DMUE_DOWNLOAD_SOUNDFONT="${MUSESCORE_DOWNLOAD_SOUNDFONT}" \
        -DMUE_BUILD_UNIT_TESTS="${MUSESCORE_BUILD_UNIT_TESTS}" \
        -DMUE_BUILD_CRASHPAD_CLIENT="${MUSESCORE_BUILD_CRASHPAD_CLIENT}" \
        -DMUE_CRASH_REPORT_URL="${MUSESCORE_CRASHREPORT_URL}" \
        -DMUE_LOGGER_DEBUGLEVEL_ENABLED="${MUSESCORE_DEBUGLEVEL_ENABLED}" \
        -DVST3_SDK_PATH="${MUSESCORE_VST3_SDK_PATH}" \
        -DCMAKE_SKIP_RPATH="${MUSESCORE_NO_RPATH}" \


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
            -DMUE_COMPILE_USE_UNITY=OFF \
            -DCMAKE_BUILD_TYPE="Debug" \
            -DCMAKE_INSTALL_PREFIX="${MUSESCORE_INSTALL_DIR}" \
            -DMUSESCORE_INSTALL_SUFFIX="${MUSESCORE_INSTALL_SUFFIX}" \
            -DMUSESCORE_BUILD_CONFIGURATION="${MUSESCORE_BUILD_CONFIGURATION}" \
            -DMUSESCORE_BUILD_MODE="${MUSESCORE_BUILD_MODE}" \
            -DCMAKE_BUILD_NUMBER="${MUSESCORE_BUILD_NUMBER}" \
            -DMUSESCORE_REVISION="${MUSESCORE_REVISION}" \
            -DMUE_RUN_LRELEASE="${MUSESCORE_RUN_LRELEASE}" \
            -DMUE_BUILD_VST_MODULE="${MUSESCORE_BUILD_VST_MODULE}" \
            -DMUE_BUILD_VIDEOEXPORT_MODULE="${MUSESCORE_BUILD_VIDEOEXPORT_MODULE}" \
            -DMUE_LEARN_YOUTUBE_API_KEY="${MUSESCORE_YOUTUBE_API_KEY}" \
            -DMUE_BUILD_UPDATE_MODULE="${MUSESCORE_BUILD_UPDATE_MODULE}" \
            -DMUE_DOWNLOAD_SOUNDFONT="${MUSESCORE_DOWNLOAD_SOUNDFONT}" \
            -DMUE_BUILD_UNIT_TESTS="${MUSESCORE_BUILD_UNIT_TESTS}" \
            -DMUE_BUILD_CRASHPAD_CLIENT="${MUSESCORE_BUILD_CRASHPAD_CLIENT}" \
            -DMUE_CRASH_REPORT_URL="${MUSESCORE_CRASHREPORT_URL}" \
            -DMUE_LOGGER_DEBUGLEVEL_ENABLED="${MUSESCORE_DEBUGLEVEL_ENABLED}" \
            -DVST3_SDK_PATH="${MUSESCORE_VST3_SDK_PATH}" \
            -DCMAKE_SKIP_RPATH="${MUSESCORE_NO_RPATH}" \


        ;;

    revision)
	    git rev-parse --short=7 HEAD | tr -d '\n' > local_build_revision.env
        ;;

    appimage)
        MUSESCORE_INSTALL_DIR=../MuseScore 
        MUSESCORE_INSTALL_SUFFIX="4portable${MUSESCORE_INSTALL_SUFFIX}" # e.g. "4portable" or "4portablenightly"
        MUSESCORE_NO_RPATH=ON 

        mkdir -p build.release
        cd build.release
        do_build RELEASE
        ninja install

        build_dir="$(pwd)" 
        install_dir="$(cat $build_dir/PREFIX.txt)" 
        cd $install_dir

        ln -sf . usr # we installed into the root of our AppImage but some tools expect a "usr" subdirectory
        mscore="mscore${MUSESCORE_INSTALL_SUFFIX}"
        desktop="org.musescore.MuseScore${MUSESCORE_INSTALL_SUFFIX}.desktop"
        icon="${mscore}.png"
        mani="install_manifest.txt" 
        cp "share/applications/${desktop}" "${desktop}"
        cp "share/icons/hicolor/128x128/apps/${icon}" "${icon}"
        <"$build_dir/${mani}" >"${mani}" sed -rn 's/.*(share\/)(applications|icons|man|metainfo|mime)(.*)/\1\2\3/p'

        ;;     

    *)
        echo "Unknown target: $TARGET";
        exit 1;
        ;;
esac
