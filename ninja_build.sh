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
JOBS=4
TARGET=release

MUSESCORE_INSTALL_DIR=${MUSESCORE_INSTALL_DIR:-"../build.install"}
MUSESCORE_INSTALL_SUFFIX=${MUSESCORE_INSTALL_SUFFIX:-""}
MUSESCORE_LABEL=${MUSESCORE_LABEL:-""}
MUSESCORE_BUILD_CONFIG=${MUSESCORE_BUILD_CONFIG:-"dev"}
MUSESCORE_BUILD_CONFIGURE=${DMUSESCORE_BUILD_CONFIGURE:-"app"}
MUSESCORE_BUILD_NUMBER=${MUSESCORE_BUILD_NUMBER:-"12345678"}
MUSESCORE_REVISION=${MUSESCORE_REVISION:-"abc123456"}
MUSESCORE_RUN_LRELEASE=${MUSESCORE_RUN_LRELEASE:-"OFF"}
MUSESCORE_CRASHREPORT_URL=${MUSESCORE_CRASHREPORT_URL:-""}
MUSESCORE_BUILD_CRASHPAD_CLIENT=${MUSESCORE_BUILD_CRASHPAD_CLIENT:-"ON"}
MUSESCORE_DEBUGLEVEL_ENABLED="OFF"
MUSESCORE_VST3_SDK_PATH=${MUSESCORE_VST3_SDK_PATH:-""}
MUSESCORE_DOWNLOAD_SOUNDFONT=${MUSESCORE_DOWNLOAD_SOUNDFONT:-"ON"}
MUSESCORE_BUILD_UNIT_TESTS=${MUSESCORE_BUILD_UNIT_TESTS:-"OFF"}
MUSESCORE_NO_RPATH=${MUSESCORE_NO_RPATH:-"OFF"}
MUSESCORE_YOUTUBE_API_KEY=${MUSESCORE_YOUTUBE_API_KEY:-""} 
MUSESCORE_BUILD_UPDATE_MODULE=${MUSESCORE_BUILD_UPDATE_MODULE:-"ON"}
MUSESCORE_BUILD_SHORTCUTS_MODULE=${MUSESCORE_BUILD_SHORTCUTS_MODULE:-"ON"}
MUSESCORE_BUILD_NETWORK_MODULE=${MUSESCORE_BUILD_NETWORK_MODULE:-"ON"}
MUSESCORE_BUILD_AUDIO_MODULE=${MUSESCORE_BUILD_AUDIO_MODULE:-"ON"}
MUSESCORE_BUILD_VST_MODULE=${MUSESCORE_BUILD_VST_MODULE:-"OFF"}
MUSESCORE_BUILD_LEARN_MODULE=${MUSESCORE_BUILD_LEARN_MODULE:-"ON"}
MUSESCORE_BUILD_WORKSPACE_MODULE=${MUSESCORE_BUILD_WORKSPACE_MODULE:-"ON"}
MUSESCORE_BUILD_CLOUD_MODULE=${MUSESCORE_BUILD_CLOUD_MODULE:-"ON"}
MUSESCORE_BUILD_LANGUAGES_MODULE=${MUSESCORE_BUILD_LANGUAGES_MODULE:-"ON"}
MUSESCORE_BUILD_PLUGINS_MODULE=${MUSESCORE_BUILD_PLUGINS_MODULE:-"ON"}
MUSESCORE_BUILD_PLAYBACK_MODULE=${MUSESCORE_BUILD_PLAYBACK_MODULE:-"ON"}
MUSESCORE_BUILD_PALETTE_MODULE=${MUSESCORE_BUILD_PALETTE_MODULE:-"ON"}
MUSESCORE_BUILD_INSTRUMENTSSCENE_MODULE=${MUSESCORE_BUILD_INSTRUMENTSSCENE_MODULE:-"ON"}
MUSESCORE_BUILD_INSPECTOR_MODULE=${MUSESCORE_BUILD_INSPECTOR_MODULE:-"ON"}
MUSESCORE_BUILD_MULTIINSTANCES_MODULE=${MUSESCORE_BUILD_MULTIINSTANCES_MODULE:-"ON"}
MUSESCORE_BUILD_VIDEOEXPORT_MODULE=${MUSESCORE_BUILD_VIDEOEXPORT_MODULE:-"OFF"}
MUSESCORE_BUILD_IMPORTEXPORT_MODULE=${MUSESCORE_BUILD_IMPORTEXPORT_MODULE:-"ON"}

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
	echo "TODO..."
	exit 0
fi

cmake --version
echo "ninja version $(ninja --version)"



function do_build() {

    BUILD_TYPE=$1

    cmake .. -GNinja \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DCMAKE_INSTALL_PREFIX="${MUSESCORE_INSTALL_DIR}" \
        -DMSCORE_INSTALL_SUFFIX="${MUSESCORE_INSTALL_SUFFIX}" \
        -DMUSESCORE_LABEL="${MUSESCORE_LABEL}" \
        -DMUSESCORE_BUILD_CONFIG="${MUSESCORE_BUILD_CONFIG}" \
        -DMUE_BUILD_CONFIGURE="${MUSESCORE_BUILD_CONFIGURE}" \
        -DCMAKE_BUILD_NUMBER="${MUSESCORE_BUILD_NUMBER}" \
        -DMUSESCORE_REVISION="${MUSESCORE_REVISION}" \
        -DMUE_RUN_LRELEASE="${MUSESCORE_RUN_LRELEASE}" \
        -DMUE_BUILD_AUDIO_MODULE="${MUSESCORE_BUILD_AUDIO_MODULE}" \
        -DMUE_BUILD_NETWORK_MODULE="${MUSESCORE_BUILD_NETWORK_MODULE}" \
        -DMUE_BUILD_SHORTCUTS_MODULE="${MUSESCORE_BUILD_SHORTCUTS_MODULE}" \
        -DMUE_BUILD_VST_MODULE="${MUSESCORE_BUILD_VST_MODULE}" \
        -DMUE_BUILD_CLOUD_MODULE="${MUSESCORE_BUILD_CLOUD_MODULE}" \
        -DMUE_BUILD_IMPORTEXPORT_MODULE="${MUSESCORE_BUILD_IMPORTEXPORT_MODULE}" \
        -DMUE_BUILD_VIDEOEXPORT_MODULE="${MUSESCORE_BUILD_VIDEOEXPORT_MODULE}" \
        -DMUE_BUILD_INSPECTOR_MODULE="${MUSESCORE_BUILD_INSPECTOR_MODULE}" \
        -DMUE_BUILD_INSTRUMENTSSCENE_MODULE="${MUSESCORE_BUILD_INSTRUMENTSSCENE_MODULE}" \
        -DMUE_BUILD_LANGUAGES_MODULE="${MUSESCORE_BUILD_LANGUAGES_MODULE}" \
        -DMUE_BUILD_LEARN_MODULE="${MUSESCORE_BUILD_LEARN_MODULE}" \
        -DMUE_LEARN_YOUTUBE_API_KEY="${MUSESCORE_YOUTUBE_API_KEY}" \
        -DMUE_BUILD_MULTIINSTANCES_MODULE="${MUSESCORE_BUILD_MULTIINSTANCES_MODULE}" \
        -DMUE_BUILD_PALETTE_MODULE="${MUSESCORE_BUILD_PALETTE_MODULE}" \
        -DMUE_BUILD_PLAYBACK_MODULE="${MUSESCORE_BUILD_PLAYBACK_MODULE}" \
        -DMUE_BUILD_PLUGINS_MODULE="${MUSESCORE_BUILD_PLUGINS_MODULE}" \
        -DMUE_BUILD_UPDATE_MODULE="${MUSESCORE_BUILD_UPDATE_MODULE}" \
        -DMUE_BUILD_WORKSPACE_MODULE="${MUSESCORE_BUILD_WORKSPACE_MODULE}" \
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

    revision)
	    git rev-parse --short=7 HEAD | tr -d '\n' > local_build_revision.env
        ;;

    appimage)
        MUSESCORE_INSTALL_DIR=../MuseScore 
        MUSESCORE_INSTALL_SUFFIX="4portable${MUSESCORE_INSTALL_SUFFIX}" # e.g. "4portable" or "4portablenightly"
        MUSESCORE_LABEL="Portable AppImage" 
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
        icon="${mscore}.svg" 
        mani="install_manifest.txt" 
        cp "share/applications/${desktop}" "${desktop}"
        cp "share/icons/hicolor/scalable/apps/${icon}" "${icon}" 
        <"$build_dir/${mani}" >"${mani}" sed -rn 's/.*(share\/)(applications|icons|man|metainfo|mime)(.*)/\1\2\3/p'

        ;;     

    *)
        echo "Unknown target: $TARGET";
        exit 1;
        ;;
esac
