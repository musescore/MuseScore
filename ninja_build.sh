#!/usr/bin/env bash

JOBS=4
TARGET=release

MUSESCORE_INSTALL_DIR=${MUSESCORE_INSTALL_DIR:-"../build.install"}
MUSESCORE_INSTALL_SUFFIX=${MUSESCORE_INSTALL_SUFFIX:-""}
MUSESCORE_LABEL=${MUSESCORE_LABEL:-""}
MUSESCORE_BUILD_CONFIG=${MUSESCORE_BUILD_CONFIG:-"dev"}
MUSESCORE_BUILD_NUMBER=${MUSESCORE_BUILD_NUMBER:-"12345678"}
MUSESCORE_REVISION=${MUSESCORE_REVISION:-"abc123456"}
MUSESCORE_TELEMETRY_ID=${MUSESCORE_TELEMETRY_ID:-""}
MUSESCORE_CRASHREPORT_URL=${MUSESCORE_CRASHREPORT_URL:-""}
MUSESCORE_BUILD_JACK=${MUSESCORE_BUILD_JACK:-"OFF"}
MUSESCORE_BUILD_WEBENGINE=${MUSESCORE_BUILD_WEBENGINE:-"OFF"}
MUSESCORE_BUILD_VST=${MUSESCORE_BUILD_VST:-"OFF"}
MUSESCORE_VST3_SDK_PATH=${MUSESCORE_VST3_SDK_PATH:-""}
MUSESCORE_DOWNLOAD_SOUNDFONT=${MUSESCORE_DOWNLOAD_SOUNDFONT:-"ON"}
MUSESCORE_BUILD_UNIT_TESTS=${MUSESCORE_BUILD_UNIT_TESTS:-"OFF"}
MUSESCORE_NO_RPATH=${MUSESCORE_NO_RPATH:-"OFF"}

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

    cmake -GNinja \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DCMAKE_INSTALL_PREFIX="${MUSESCORE_INSTALL_DIR}" \
        -DMSCORE_INSTALL_SUFFIX="${MUSESCORE_INSTALL_SUFFIX}" \
        -DMUSESCORE_LABEL="${MUSESCORE_LABEL}" \
        -DMUSESCORE_BUILD_CONFIG="${MUSESCORE_BUILD_CONFIG}" \
        -DCMAKE_BUILD_NUMBER="${MUSESCORE_BUILD_NUMBER}" \
        -DMUSESCORE_REVISION="${MUSESCORE_REVISION}" \
        -DTELEMETRY_TRACK_ID="${MUSESCORE_TELEMETRY_ID}" \
        -DCRASH_REPORT_URL="${MUSESCORE_CRASHREPORT_URL}" \
        -DBUILD_JACK="${MUSESCORE_BUILD_JACK}" \
        -DBUILD_WEBENGINE="${MUSESCORE_BUILD_WEBENGINE}" \
        -DBUILD_VST="${MUSESCORE_BUILD_VST}" \
        -DVST3_SDK_PATH="${MUSESCORE_VST3_SDK_PATH}" \
        -DDOWNLOAD_SOUNDFONT="${MUSESCORE_DOWNLOAD_SOUNDFONT}" \
        -DBUILD_UNIT_TESTS="${MUSESCORE_BUILD_UNIT_TESTS}" \
        -DCMAKE_SKIP_RPATH="${MUSESCORE_NO_RPATH}" ..

    ninja -j $JOBS 
}


case $TARGET in

    release)
        mkdir -p build.release
        cd build.release
        do_build Release
        ;; 

    debug)
        mkdir -p build.release
        cd build.release
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
        MUSESCORE_INSTALL_DIR=MuseScore 
        MUSESCORE_INSTALL_SUFFIX=-portable 
        MUSESCORE_LABEL="Portable AppImage" 
        MUSESCORE_NO_RPATH=ON 

        mkdir -p build.release
        cd build.release
        do_build RELEASE
        ninja install

        build_dir="$(pwd)" 
        install_dir="$(cat $build_dir/PREFIX.txt)" 
        cd $install_dir

        [ -L usr ] || ln -s . usr && mscore="mscore-portable" 
        dsktp="${mscore}.desktop" 
        icon="${mscore}.svg" 
        mani="install_manifest.txt" 
        cp "share/applications/${dsktp}" "${dsktp}" 
        cp "share/icons/hicolor/scalable/apps/${icon}" "${icon}" 
        <"$build_dir/${mani}" >"${mani}" sed -rn 's/.*(share\/)(man|mime|icons|applications)(.*)/\1\2\3/p'

        ;;     

    *)
        echo "Unknown target: $TARGET";
        exit 1;
        ;;
esac
