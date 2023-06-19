#!/usr/bin/env bash

INSTALL_DIR="$1" # MuseScore was installed here
APPIMAGE_NAME="$2" # name for AppImage file (created outside $INSTALL_DIR)

if [ -z "$INSTALL_DIR" ]; then echo "error: not set INSTALL_DIR"; exit 1; fi
if [ -z "$APPIMAGE_NAME" ]; then echo "error: not set APPIMAGE_NAME"; exit 1; fi

HERE="$(cd "$(dirname "$0")" && pwd)"
ORIGIN_DIR=${PWD}
BUILD_TOOLS=$HOME/build_tools

mkdir -p $BUILD_TOOLS

##########################################################################
# INSTALL APPIMAGETOOL AND LINUXDEPLOY
##########################################################################

function download_github_release()
{
  local -r repo_slug="$1" release_tag="$2" file="$3"
  if [[ "${release_tag}" == "latest" ]]; then
    local -r url="https://github.com/${repo_slug}/releases/latest/download/${file}"
  else
    local -r url="https://github.com/${repo_slug}/releases/download/${release_tag}/${file}"
  fi
  wget -q --show-progress "${url}"
  chmod +x "${file}"
}

function extract_appimage()
{
  # Extract AppImage so we can run it without having to install FUSE
  local -r appimage="$1" binary_name="$2"
  local -r appdir="${appimage%.AppImage}.AppDir"
  "./${appimage}" --appimage-extract >/dev/null # dest folder "squashfs-root"
  mv squashfs-root "${appdir}" # rename folder to avoid collisions
  ln -s "${appdir}/AppRun" "${binary_name}" # symlink for convenience
  rm -f "${appimage}"
}

function download_appimage_release()
{
  local -r github_repo_slug="$1" binary_name="$2" tag="$3"
  local -r appimage="${binary_name}-x86_64.AppImage"
  download_github_release "${github_repo_slug}" "${tag}" "${appimage}"
  extract_appimage "${appimage}" "${binary_name}"
}

if [[ ! -d $BUILD_TOOLS/appimagetool ]]; then
  mkdir $BUILD_TOOLS/appimagetool
  cd $BUILD_TOOLS/appimagetool
  download_appimage_release AppImage/AppImageKit appimagetool continuous
  cd $ORIGIN_DIR
fi
export PATH="$BUILD_TOOLS/appimagetool:$PATH"
appimagetool --version

if [[ ! -d $BUILD_TOOLS/appimageupdatetool ]]; then
  mkdir $BUILD_TOOLS/appimageupdatetool
  cd $BUILD_TOOLS/appimageupdatetool
  download_appimage_release AppImage/AppImageUpdate appimageupdatetool continuous
  cd $ORIGIN_DIR
fi
if [[ "${UPDATE_INFORMATION}" ]]; then
  export PATH="$BUILD_TOOLS/appimageupdatetool:$PATH"
  appimageupdatetool --version
fi

function download_linuxdeploy_component()
{
  download_appimage_release "linuxdeploy/$1" "$1" continuous
}

if [[ ! -d $BUILD_TOOLS/linuxdeploy ]]; then
  mkdir $BUILD_TOOLS/linuxdeploy
  cd $BUILD_TOOLS/linuxdeploy
  download_linuxdeploy_component linuxdeploy
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
# rm -f ${QT_PATH}/plugins/sqldrivers/libqsql{mysql,psql}.so
qt_sql_drivers_path="${QT_PATH}/plugins/sqldrivers"
qt_sql_drivers_tmp="/tmp/qtsqldrivers"
mkdir -p "$qt_sql_drivers_tmp"
mv "${qt_sql_drivers_path}/libqsqlmysql.so" "${qt_sql_drivers_tmp}/libqsqlmysql.so"
mv "${qt_sql_drivers_path}/libqsqlpsql.so" "${qt_sql_drivers_tmp}/libqsqlpsql.so"

# Semicolon-separated list of platforms to deploy in addition to `libqxcb.so`.
# Used by linuxdeploy-plugin-qt.
export EXTRA_PLATFORM_PLUGINS="libqoffscreen.so;libqwayland-egl.so;libqwayland-generic.so"

# Colon-separated list of root directories containing QML files.
# Needed for linuxdeploy-plugin-qt to scan for QML imports.
# Qml files can be in different directories, the qmlimportscanner will go through everything recursively.
export QML_SOURCES_PATHS=./

linuxdeploy --appdir "${appdir}" # adds all shared library dependencies
linuxdeploy-plugin-qt --appdir "${appdir}" # adds all Qt dependencies

# Approximately on June 1, the QtQuick/Controls.2 stopped being deploying 
# (at that time the linux deploy was updated). 
# This is a hack, for the deployment of QtQuick/Controls.2 
if [ ! -f ${appdir}/usr/lib/libQt5QuickControls2.so.5 ]; then
    cp -r ${QT_PATH}/qml/QtQuick/Controls.2 ${appdir}/usr/qml/QtQuick/Controls.2
    cp -r ${QT_PATH}/qml/QtQuick/Templates.2 ${appdir}/usr/qml/QtQuick/Templates.2
    cp ${QT_PATH}/lib/libQt5QuickControls2.so.5 ${appdir}/usr/lib/libQt5QuickControls2.so.5 
    cp ${QT_PATH}/lib/libQt5QuickTemplates2.so.5 ${appdir}/usr/lib/libQt5QuickTemplates2.so.5 
fi

# At an unknown point in time, the libqgtk3 plugin stopped being deployed
if [ ! -f ${appdir}/plugins/platformthemes/libqgtk3.so ]; then
  cp ${QT_PATH}/plugins/platformthemes/libqgtk3.so ${appdir}/plugins/platformthemes/libqgtk3.so 
fi

# The system must be used
if [ -f ${appdir}/lib/libglib-2.0.so.0 ]; then
  rm -f ${appdir}/lib/libglib-2.0.so.0 
fi

unset QML_SOURCES_PATHS EXTRA_PLATFORM_PLUGINS

# In case this container is reused multiple times, return the moved libraries back
mv "${qt_sql_drivers_tmp}/libqsqlmysql.so" "${qt_sql_drivers_path}/libqsqlmysql.so"
mv "${qt_sql_drivers_tmp}/libqsqlpsql.so" "${qt_sql_drivers_path}/libqsqlpsql.so"

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

function fallback_library()
{
  # Copy a library into a special fallback directory inside the AppDir.
  # Fallback libraries are not loaded at runtime by default, but they can
  # be loaded if it is found that the application would crash otherwise.
  local library="$1"
  local full_path="$(find_library "$1")"
  local new_path="${appdir}/fallback/${library}"
  mkdir -p "${new_path}" # directory has the same name as the library
  cp -L "${full_path}" "${new_path}/${library}"
  # Use the AppRun script to check at runtime whether the user has a copy of
  # this library. If not then add our copy's directory to $LD_LIBRARY_PATH.
}

# UNWANTED FILES
# linuxdeploy or linuxdeploy-plugin-qt may have added some files or folders
# that we don't want. List them here using paths relative to AppDir root.
# Report new additions at https://github.com/linuxdeploy/linuxdeploy/issues
# or https://github.com/linuxdeploy/linuxdeploy-plugin-qt/issues for Qt libs.
unwanted_files=(
  # none
)

# ADDITIONAL QT COMPONENTS
# linuxdeploy-plugin-qt may have missed some Qt files or folders that we need.
# List them here using paths relative to the Qt root directory. Report new
# additions at https://github.com/linuxdeploy/linuxdeploy-plugin-qt/issues
additional_qt_components=(
  plugins/printsupport/libcupsprintersupport.so

  # Wayland support (run with QT_QPA_PLATFORM=wayland to use)
  plugins/wayland-decoration-client
  plugins/wayland-graphics-integration-client
  plugins/wayland-shell-integration
)

# ADDITIONAL LIBRARIES
# linuxdeploy may have missed some libraries that we need
# Report new additions at https://github.com/linuxdeploy/linuxdeploy/issues
additional_libraries=(
  # none
)

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
)

# PREVIOUSLY EXTRACTED APPIMAGES
# These include their own dependencies. We bundle them uncompressed to avoid
# creating a double layer of compression (AppImage inside AppImage).
if [[ "${UPDATE_INFORMATION}" ]]; then
extracted_appimages=(
  appimageupdatetool
)
else
extracted_appimages=(
  # none
)
fi

for file in "${unwanted_files[@]}"; do
  rm -rf "${appdir}/${file}"
done

for file in "${additional_qt_components[@]}"; do
  mkdir -p "${appdir}/$(dirname "${file}")"
  cp -Lr "${QT_PATH}/${file}" "${appdir}/${file}"
done

for lib in "${additional_libraries[@]}"; do
  full_path="$(find_library "${lib}")"
  cp -L "${full_path}" "${appdir}/lib/${lib}"
done

for fb_lib in "${fallback_libraries[@]}"; do
  fallback_library "${fb_lib}"
done

for name in "${extracted_appimages[@]}"; do
  symlink="$(which "${name}")"
  apprun="$(dirname "${symlink}")/$(readlink "${symlink}")"
  if [[ ! -L "${symlink}" || ! -f "${apprun}" ]]; then
    echo "$0: Warning: Unable to find AppImage for '${name}'. Will not bundle." >&2
    continue
  fi
  extracted_appdir_path="$(dirname "${apprun}")"
  extracted_appdir_name="$(basename "${extracted_appdir_path}")"
  cp -r "${extracted_appdir_path}" "${appdir}/"
  cat >"${appdir}/bin/${name}" <<EOF
#!/bin/sh
unset APPDIR APPIMAGE # clear outer values before running inner AppImage
HERE="\$(dirname "\$(readlink -f "\$0")")"
exec "\${HERE}/../${extracted_appdir_name}/AppRun" "\$@"
EOF
  chmod +x "${appdir}/bin/${name}"
done

# METHOD OF LAST RESORT
# Special treatment for some dependencies when all other methods fail

# Bundle libnss3 and friends as fallback libraries. Needed on Chromebook.
# See discussion at https://github.com/probonopd/linuxdeployqt/issues/35
libnss3_files=(
  # https://packages.ubuntu.com/xenial/amd64/libnss3/filelist
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

libnss3_system_path="$(dirname "$(find_library libnss3.so)")"
libnss3_appdir_path="${appdir}/fallback/libnss3.so" # directory named like library

mkdir -p "${libnss3_appdir_path}/nss"

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

# We are running as root in the Docker image so all created files belong to
# root. Allow non-root users outside the Docker image to access these files.
chmod a+rwx "${created_files[@]}"
parent_dir="${PWD}"
while [[ "$(dirname "${parent_dir}")" != "${parent_dir}" ]]; do
  [[ "$parent_dir" == "/" ]] && break
  chmod a+rwx "$parent_dir"
  parent_dir="$(dirname "$parent_dir")"
done

echo "Making AppImage finished"
