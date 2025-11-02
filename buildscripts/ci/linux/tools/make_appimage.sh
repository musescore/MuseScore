#!/usr/bin/env bash

trap 'code=$?; echo "error: make_appimage.sh: command \`$BASH_COMMAND\` exited with code $code." >&2; exit 1' ERR

INSTALL_DIR="$1" # MuseScore was installed here
APPIMAGE_NAME="$2" # name for AppImage file (created outside $INSTALL_DIR)
PACKARCH="$3" # architecture (x86_64, aarch64, armv7l)

if [ -z "$INSTALL_DIR" ]; then echo "error: not set INSTALL_DIR"; exit 1; fi
if [ -z "$APPIMAGE_NAME" ]; then echo "error: not set APPIMAGE_NAME"; exit 1; fi
if [ -z "$PACKARCH" ]; then 
  PACKARCH="x86_64"
elif [ "$PACKARCH" == "armv7l" ]; then
  PACKARCH="armhf"
fi

HERE="$(cd "$(dirname "$0")" && pwd)"
ORIGIN_DIR=${PWD}
BUILD_TOOLS=$HOME/build_tools

mkdir -p $BUILD_TOOLS

##########################################################################
# INSTALL APPIMAGETOOL AND LINUXDEPLOY
##########################################################################

if [ "$PACKARCH" == "armhf" ]; then
  # In a Docker container, AppImages cannot run normally because of problems with FUSE.
  export APPIMAGE_EXTRACT_AND_RUN=1
fi

function download_github_release()
{
  local -r repo_slug="$1" release_tag="$2" file="$3"
  if [[ "${release_tag}" == "latest" ]]; then
    local -r url="https://github.com/${repo_slug}/releases/latest/download/${file}"
  else
    local -r url="https://github.com/${repo_slug}/releases/download/${release_tag}/${file}"
  fi

  echo "try download: ${url}"
  
  # use curl instead of wget which fails on armhf
  curl "${url}" -O -L
  chmod +x "${file}"
  echo "downloaded: ${file}"
}

function download_appimage_release()
{
  local -r github_repo_slug="$1" binary_name="$2" tag="$3"
  local -r appimage="${binary_name}-${PACKARCH}.AppImage"
  download_github_release "${github_repo_slug}" "${tag}" "${appimage}"
  mv "${appimage}" "${binary_name}"
}

if [[ ! -d $BUILD_TOOLS/appimagetool ]]; then
  mkdir $BUILD_TOOLS/appimagetool
  cd $BUILD_TOOLS/appimagetool
  download_appimage_release AppImage/appimagetool appimagetool continuous
  cd $ORIGIN_DIR
fi
export PATH="$BUILD_TOOLS/appimagetool:$PATH"
appimagetool --version

function download_linuxdeploy_component()
{
  download_appimage_release "linuxdeploy/$1" "$1" continuous
}

if [[ ! -f $BUILD_TOOLS/linuxdeploy/linuxdeploy ]]; then
  mkdir -p $BUILD_TOOLS/linuxdeploy
  cd $BUILD_TOOLS/linuxdeploy
  download_linuxdeploy_component linuxdeploy
  cd $ORIGIN_DIR
fi
if [[ ! -f $BUILD_TOOLS/linuxdeploy/linuxdeploy-plugin-qt ]]; then
  mkdir -p $BUILD_TOOLS/linuxdeploy
  cd $BUILD_TOOLS/linuxdeploy
  download_linuxdeploy_component linuxdeploy-plugin-qt
  cd $ORIGIN_DIR
fi
export PATH="$BUILD_TOOLS/linuxdeploy:$PATH"
linuxdeploy --list-plugins

##########################################################################
# BUNDLE DEPENDENCIES INTO APPDIR
##########################################################################

cd "$(dirname "${INSTALL_DIR}")"
appdir="$(basename "${INSTALL_DIR}")" # directory that will become the AppImage

# Prevent linuxdeploy setting RUNPATH in binaries that shouldn't have it
mv "${appdir}/bin/findlib" "${appdir}/../findlib"

# Remove Qt plugins for MySQL and PostgreSQL to prevent
# linuxdeploy-plugin-qt from failing due to missing dependencies.
# SQLite plugin alone should be enough for our AppImage.
# rm -f ${QT_ROOT_DIR}/plugins/sqldrivers/libqsql{mysql,psql}.so
qt_sql_drivers_path="${QT_ROOT_DIR}/plugins/sqldrivers"
qt_sql_drivers_tmp="/tmp/qtsqldrivers"
mkdir -p "$qt_sql_drivers_tmp"
[ -f "${qt_sql_drivers_path}/libqsqlmysql.so" ] && mv "${qt_sql_drivers_path}/libqsqlmysql.so" "${qt_sql_drivers_tmp}/libqsqlmysql.so"
[ -f "${qt_sql_drivers_path}/libqsqlpsql.so" ] && mv "${qt_sql_drivers_path}/libqsqlpsql.so" "${qt_sql_drivers_tmp}/libqsqlpsql.so"

# Semicolon-separated list of platforms to deploy in addition to `libqxcb.so`.
# Used by linuxdeploy-plugin-qt.
export EXTRA_PLATFORM_PLUGINS="libqoffscreen.so;libqwayland.so"

# Colon-separated list of root directories containing QML files.
# Needed for linuxdeploy-plugin-qt to scan for QML imports.
# Qml files can be in different directories, the qmlimportscanner will go through everything recursively.
export QML_SOURCES_PATHS=./

linuxdeploy --appdir "${appdir}" # adds all shared library dependencies
linuxdeploy-plugin-qt --appdir "${appdir}" # adds all Qt dependencies

# The system must be used
if [ -f ${appdir}/lib/libglib-2.0.so.0 ]; then
  rm -f ${appdir}/lib/libglib-2.0.so.0 
fi

unset QML_SOURCES_PATHS EXTRA_PLATFORM_PLUGINS

# Return the moved libraries back
[ -f "${qt_sql_drivers_tmp}/libqsqlmysql.so" ] && mv "${qt_sql_drivers_tmp}/libqsqlmysql.so" "${qt_sql_drivers_path}/libqsqlmysql.so"
[ -f "${qt_sql_drivers_tmp}/libqsqlpsql.so" ] && mv "${qt_sql_drivers_tmp}/libqsqlpsql.so" "${qt_sql_drivers_path}/libqsqlpsql.so"

# Put the non-RUNPATH binaries back
mv "${appdir}/../findlib" "${appdir}/bin/findlib"

##########################################################################
# BUNDLE REMAINING DEPENDENCIES MANUALLY
##########################################################################

function find_library()
{
  # Print full path to a library or return exit status 1 if not found
  "${appdir}/bin/findlib" "$@"
}

# UNWANTED FILES
# linuxdeploy or linuxdeploy-plugin-qt may have added some files or folders
# that we don't want. List them here using paths relative to AppDir root.
# Report new additions at https://github.com/linuxdeploy/linuxdeploy/issues
# or https://github.com/linuxdeploy/linuxdeploy-plugin-qt/issues for Qt libs.
unwanted_files=(
  # https://github.com/musescore/MuseScore/issues/24068#issuecomment-2297823192
  lib/libwayland-client.so.0
)

for file in "${unwanted_files[@]}"; do
  rm -rf "${appdir}/${file}"
done

# ADDITIONAL QT COMPONENTS
# linuxdeploy-plugin-qt may have missed some Qt files or folders that we need.
# List them here using paths relative to the Qt root directory. Report new
# additions at https://github.com/linuxdeploy/linuxdeploy-plugin-qt/issues
additional_qt_components=(
  plugins/printsupport/libcupsprintersupport.so

  # At an unknown point in time, the libqgtk3 plugin stopped being deployed
  plugins/platformthemes/libqgtk3.so

  # Wayland support (run with MU_QT_QPA_PLATFORM=wayland to use)
  plugins/wayland-decoration-client
  plugins/wayland-graphics-integration-client
  plugins/wayland-shell-integration
)

for file in "${additional_qt_components[@]}"; do
  if [ -f "${appdir}/${file}" ]; then
    echo "Warning: ${file} was already deployed. Skipping."
    continue
  fi
  mkdir -p "${appdir}/$(dirname "${file}")"
  cp -Lr "${QT_ROOT_DIR}/${file}" "${appdir}/${file}"
done

# ADDITIONAL LIBRARIES
# linuxdeploy may have missed some libraries that we need
# Report new additions at https://github.com/linuxdeploy/linuxdeploy/issues
if [[ "$PACKARCH" == "x86_64" ]]; then
  additional_libraries=(
    libssl.so.1.1    # OpenSSL (for Save Online)
    libcrypto.so.1.1 # OpenSSL (for Save Online)
  )
else
  additional_libraries=()
fi

for lib in "${additional_libraries[@]}"; do
  if [ -f "${appdir}/lib/${lib}" ]; then
    echo "Warning: ${lib} was already deployed. Skipping."
    continue
  fi
  full_path="$(find_library "${lib}")"
  cp -L "${full_path}" "${appdir}/lib/${lib}"
done

# FALLBACK LIBRARIES
# These get bundled in the AppImage, but are only loaded if the user does not
# already have a version of the library installed on their system. This is
# helpful in cases where it is necessary to use a system library in order for
# a particular feature to work properly, but where the program would crash at
# startup if the library was not found. The fallback library may not provide
# the full functionality of the system version, but it does avoid the crash.
# Report new additions at https://github.com/linuxdeploy/linuxdeploy/issues
fallback_libraries=(
  libjack.so.0 # https://github.com/LMMS/lmms/pull/3958
  libOpenGL.so.0 # https://bugreports.qt.io/browse/QTBUG-89754
  libpipewire-0.3.so.0
)

for fb_lib in "${fallback_libraries[@]}"; do
  full_path="$(find_library "$fb_lib")"
  new_path="${appdir}/fallback/${fb_lib}"
  mkdir -p "${new_path}" # directory has the same name as the library
  cp -L "${full_path}" "${new_path}/${fb_lib}"
  # Use the AppRun script to check at runtime whether the user has a copy of
  # this library. If not then add our copy's directory to $LD_LIBRARY_PATH.
done

# pipewire is still packaged by linuxdeploy, 
# so we need to remove it as it is in the fallback mechanism
if [[ -f "${appdir}/lib/libpipewire-0.3.so.0" ]]; then
  rm -f "${appdir}/lib/libpipewire-0.3.so.0"
fi

# APPIMAGEUPDATETOOL
# Bundled uncompressed, to avoid creating a double layer of compression
# (AppImage inside AppImage).
if [[ "${UPDATE_INFORMATION}" ]]; then
  if [[ ! -d $BUILD_TOOLS/appimageupdatetool ]]; then
    mkdir $BUILD_TOOLS/appimageupdatetool
    cd $BUILD_TOOLS/appimageupdatetool
    download_appimage_release AppImageCommunity/AppImageUpdate appimageupdatetool continuous
    cd $ORIGIN_DIR
  fi

  export PATH="$BUILD_TOOLS/appimageupdatetool:$PATH"
  appimageupdatetool --version

  # Extract appimageupdatetool
  appimageupdatetool --appimage-extract >/dev/null # dest folder "squashfs-root"

  # Move into AppDir
  mv squashfs-root ${appdir}/appimageupdatetool.AppDir

  # Create alias in `${appdir}/bin`
  # Use script instead of symlink, because appimageupdatetool.AppDir/AppRun fails
  # when run from a symlink.
  cat >"${appdir}/bin/appimageupdatetool" <<EOF
#!/bin/sh
unset APPDIR APPIMAGE # clear outer values before running inner AppImage
HERE="\$(dirname "\$(readlink -f "\$0")")"
exec "\${HERE}/../appimageupdatetool.AppDir/AppRun" "\$@"
EOF
  chmod +x "${appdir}/bin/appimageupdatetool"
fi

# METHOD OF LAST RESORT
# Special treatment for some dependencies when all other methods fail

# Bundle libnss3 and friends as fallback libraries. Needed on Chromebook.
# See discussion at https://github.com/probonopd/linuxdeployqt/issues/35
libnss3_system_path="$(dirname "$(find_library libnss3.so)")"
libnss3_appdir_path="${appdir}/fallback/libnss3.so" # directory named like library

mkdir -p "${libnss3_appdir_path}"

if [ -d "${libnss3_system_path}/nss" ]; then
  mkdir -p "${libnss3_appdir_path}/nss"

  libnss3_files=(
    # https://packages.ubuntu.com/jammy/amd64/libnss3/filelist
    libnss3.so
    libnssutil3.so
    libsmime3.so
    libssl3.so
    nss/libfreebl3.chk
    nss/libfreebl3.so
    nss/libfreeblpriv3.chk
    nss/libfreeblpriv3.so
    nss/libnssckbi.so
    nss/libnssdbm3.chk
    nss/libnssdbm3.so
    nss/libsoftokn3.chk
    nss/libsoftokn3.so
  )
else
  libnss3_files=(
    # https://packages.ubuntu.com/noble/amd64/libnss3/filelist
    libfreebl3.chk
    libfreebl3.so
    libfreeblpriv3.chk
    libfreeblpriv3.so
    libnss3.so
    libnssckbi.so
    libnssdbm3.chk
    libnssdbm3.so
    libnssutil3.so
    libsmime3.so
    libsoftokn3.chk
    libsoftokn3.so
    libssl3.so
  )
fi

for file in "${libnss3_files[@]}"; do
  cp -L "${libnss3_system_path}/${file}" "${libnss3_appdir_path}/${file}"
  rm -f "${appdir}/lib/$(basename "${file}")" # in case it was already packaged by linuxdeploy
done

##########################################################################
# TURN APPDIR INTO AN APPIMAGE
##########################################################################

appimage="${APPIMAGE_NAME}" # name to use for AppImage file

appimagetool_args=( # array
  --no-appstream # do not check upstream metadata
  )

created_files=(
  "${appimage}"
  )

if [[ "${UPDATE_INFORMATION}" ]]; then
  appimagetool_args+=( # append to array
    --updateinformation "${UPDATE_INFORMATION}"
    )
  created_files+=(
    "${appimage}.zsync" # this file will contain delta update data
    )
else
  cat >&2 <<EOF
$0: Automatic updates disabled.
To enable automatic updates, please set the env. variable UPDATE_INFORMATION
according to <https://github.com/AppImage/AppImageSpec/blob/master/draft.md>.
EOF
fi

# create AppImage
appimagetool "${appimagetool_args[@]}" "${appdir}" "${appimage}"

echo "Making AppImage finished"
