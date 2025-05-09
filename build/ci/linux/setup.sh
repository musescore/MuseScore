#!/usr/bin/env bash
echo "############################## Setup Linux build environment (setup.sh) ##############################"
trap 'echo setup.sh failed; exit 1' ERR

df -h .

# PARAM 

#   optional
PACKARCH="x86_64" # mtest, vtest
CFLAGS="-Wno-psabi"
CXXFLAGS="-Wno-psabi"
ADDSUDO=''
VTESTPREFIX=''
#   consume
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --arch) PACKARCH="$2"; shift ;;
        --cflags) CFLAGS="$2"; shift ;;
        --cxxflags) CXXFLAGS="$2"; shift ;;
        --is_calling_without_sudo) ADDSUDO="$2"; shift ;; #apt-get x86_64, mtest, vtest
        -p) VTESTPREFIX="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done
#   required
#     none

# INIT and MAIN
if [[ -n "$VTESTPREFIX" ]]; then
  BUILD_TOOLS=$HOME/$VTESTPREFIX/build_tools
else
  BUILD_TOOLS=$HOME/build_tools
fi
mkdir -p $BUILD_TOOLS
ENV_FILE=$BUILD_TOOLS/environment.sh
rm -f $ENV_FILE
echo "New $ENV_FILE is created overwriting existing."
echo 'echo "Setup MuseScore build environment"' >> $ENV_FILE

if [[ "$PACKARCH" == "armv7l" ]] || [[ "$PACKARCH" == "aarch64" ]]; then
  export DEBIAN_FRONTEND="noninteractive" TZ="Europe/London"
fi
if [[ -n "$ADDSUDO" ]]; then SUDO="sudo"; else SUDO=""; fi

echo "############################## GET DEPENDENCIES ##############################"

apt_packages=( # musescore 3
  # apt_packages_basic=(
  file
  git
  # pkg-config # https://github.com/musescore/MuseScore/pull/25609
  software-properties-common # installs `add-apt-repository`
  unzip
  p7zip-full
  # apt_packages_standard=(
  curl
  libasound2-dev 
  libfontconfig1-dev
  libfreetype6-dev
  libfreetype6
  libgl1-mesa-dev
  libjack-dev
  libmp3lame-dev # musescore 3 needs, but removed in musescore 4
  libnss3-dev
  libportmidi-dev
  libpulse-dev
  libsndfile1-dev
  make
  portaudio19-dev # musescore 3 needs, but removed in musescore 4
  wget
  )
apt_packagesARM=(
# adapt musescore 4. Apr20,2025 https://github.com/musescore/MuseScore/blob/b02a3fc49e37ae5d7a41892add56d36d3ee689d9/buildscripts/ci/linux/setup.sh
# comment out to leave as future backport ref, mark 3 = duplicate musescore 3
  coreutils
  # 3 curl
  desktop-file-utils # installs `desktop-file-validate` for appimagetool
  gawk
  # 3 file
  # 3 git
  libboost-dev
  libboost-filesystem-dev
  libboost-regex-dev
  libcairo2-dev
  libfuse-dev
  libtool
  libssl-dev
  # 3 pkg-config
  xxd
  # 3 p7zip-full
  # 3 libasound2-dev 
  # 3 libfontconfig1-dev
  # 3 libfreetype6
  # 3 libfreetype6-dev
  libgcrypt20-dev
  # 3 libgl1-mesa-dev
  libglib2.0-dev
  libgpgme-dev # install for appimagetool
  # 3 libjack-dev
  # 3 libnss3-dev
  # 3 libportmidi-dev
  # 3 libpulse-dev
  librsvg2-dev
  # 3 libsndfile1-dev
  # 3 libssl-dev
  # 3 libtool
  # 3 make
  # 3 p7zip-full
  sed
  # 3 software-properties-common
  # 3 unzip
  # 3 wget
  zsync # installs `zsyncmake` for appimagetool
  )

# when slimming down, note that compile success != run without bug
apt_packages_runtime=(
  libcups2
  libdbus-1-3
  libegl1-mesa-dev
  libodbc1
  libpq-dev
  libxcomposite-dev
  libxcursor-dev
  libxi-dev
  libxkbcommon-x11-0
  libxrandr2
  libxtst-dev
  libdrm-dev
  )
apt_packages_runtimeARM=( # adapt musescore 4. Apr20,2025 https://github.com/musescore/MuseScore/blob/b02a3fc49e37ae5d7a41892add56d36d3ee689d9/buildscripts/ci/linux/setup.sh
# comment out to leave as future backport ref, mark 3 = duplicate musescore 3
  # 3 libcups2
  # 3 libdbus-1-3
  # 3 libegl1-mesa-dev
  libgles2-mesa-dev # arm64 https://github.com/musescore/MuseScore/commit/2c38219cdb956003d7d5b1872447d8eee1f5205c
  # 3 libodbc1
  # 3 libpq-dev
  # 3 libssl-dev
  # 3 libxcomposite-dev
  # 3 libxcursor-dev
  # 3 libxi-dev
  # 3 libxkbcommon-x11-0
  # 3 libxrandr2
  # 3 libxtst-dev
  # 3 libdrm-dev
  libxcb-icccm4
  libxcb-image0
  libxcb-keysyms1
  libxcb-randr0
  libxcb-render-util0
  libxcb-xinerama0
  # libxcb-xkb-dev # Added support Qt6.2 to Lin CI #21544 
  # libxkbcommon-dev # Added support Qt6.2 to Lin CI #21544 
  libopengl-dev # fix ARM startup bug   https://github.com/musescore/MuseScore/issues/24228 , qt5 and qt6 https://bugreports.qt.io/browse/QTBUG-89754 , libOpenGL.so.0 in make_appimage.sh 
  # libvulkan-dev # Added support Qt6.2 to Lin CI #21544 
  )

# ARM xkb missing package compile error
apt_packages_ffmpeg=(
  ffmpeg
  libavcodec-dev 
  libavformat-dev 
  libswscale-dev
  )

$SUDO apt-get update # no package lists in Docker image
case "$PACKARCH" in
  x86_64)
    $SUDO apt-get install -y --no-install-recommends \
      "${apt_packages[@]}" \
      "${apt_packages_runtime[@]}" 
    ;;
  armv7l | aarch64)
    $SUDO apt-get install -y --no-install-recommends \
      "${apt_packages[@]}" \
      "${apt_packagesARM[@]}" \
      "${apt_packages_runtime[@]}" \
      "${apt_packages_runtimeARM[@]}" \
      "${apt_packages_ffmpeg[@]}"
    ;;
esac

echo "############################## GET QT ##############################"

case "$PACKARCH" in
  x86_64)
    # https://github.com/Jojo-Schmitz/MuseScore/commit/1e58c6a9f6d3971801d530098f813475a1b104fc
    #   5.9 to 5.15
    #   Mainly to fix https://musescore.org/en/node/317323 for macOS. but
    #   updating the builds for Linux and Windows too.
    # https://github.com/Jojo-Schmitz/MuseScore/commit/3a8e9017f13ad456ca2fa36f71d90bca53482b38
    #   5.15 revert back to 5.9
    #   Fix vtests build using a similar method as the mtest.
    #   Doesn't work, so back to 5.9.8, for Linux, for now...

    # qtselect="qt5"
    # qt_version="5152"
    # qt_dir="$BUILD_TOOLS/Qt/${qt_version}"
    # if [[ ! -d "${qt_dir}" ]]; then
    #   mkdir -p "${qt_dir}"
    #   qt_url="https://s3.amazonaws.com/utils.musescore.org/Qt${qt_version}_gcc64.7z"
    #   wget -q --show-progress -O qt5.7z "${qt_url}"
    #   7z x -y qt5.7z -o"${qt_dir}"
    # fi

    qtselect="qt5"
    qt_version="598"
    qt_dir="$BUILD_TOOLS/qt${qt_version}"
    if [[ ! -d "${qt_dir}" ]]; then
      mkdir -p "${qt_dir}"
      qt_url="https://s3.amazonaws.com/utils.musescore.org/qt${qt_version}.zip"
      wget -q --show-progress -O qt5.zip "${qt_url}"
      7z x -y qt5.zip -o"${qt_dir}"
    fi
    # todo : utils.musescore.org does not contain wayland, edit also make_appimage.sh

    ;;
  armv7l | aarch64)
    # kitware is the cmake company
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ bionic main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null
    # ? needed by ppa qt ? bionic because ubuntu 22 ( see apt.kitware.com) ?

    add-apt-repository --yes ppa:theofficialgman/opt-qt-5.15.2-focal-arm
    echo "using ppa:theofficialgman/opt-qt-5.15.2-focal-arm" # one package containing focal jammy and bionic
    qtselect="qt5"
    qt_dir="/opt/qt515" # qt_version="5152"
    $SUDO apt-get update

    apt_packages_qt=(
      qt515base
      qt515declarative
      qt515quickcontrols
      qt515quickcontrols2
      qt515graphicaleffects
      qt515imageformats
      qt515networkauth-no-lgpl
      qt515remoteobjects
      qt515svg
      qt515tools
      qt515translations
      qt515wayland
      qt515x11extras
      qt515xmlpatterns
      qt515webengine # main/cmakelists.txt L232 QtWebEngineProcess  ? QtWebEngine cause bug ? QtWebEngine != Process ? Let's keep unedited main/cmakelists.txt til musescore.com cloud save login page bug (error 498) server fix. 
      )
    $SUDO apt-get install -y \
      "${apt_packages_qt[@]}"
    ;;
esac

echo export QT_SELECT="$qtselect" >> ${ENV_FILE}
echo export PATH="${qt_dir}/bin:\${PATH}" >> ${ENV_FILE}
echo export LD_LIBRARY_PATH="${qt_dir}/lib:\${LD_LIBRARY_PATH}" >> ${ENV_FILE}
echo export QT_PATH="${qt_dir}" >> ${ENV_FILE}
echo export QT_PLUGIN_PATH="${qt_dir}/plugins" >> ${ENV_FILE}
echo export QML2_IMPORT_PATH="${qt_dir}/qml" >> ${ENV_FILE}

echo "############################## GET COMPILERS ##############################"

$SUDO apt-get install -y --no-install-recommends automake

echo "download gcc g++"
case "$PACKARCH" in
  x86_64)
    gcc_version="9"
    $SUDO apt-get install -y --no-install-recommends "g++-${gcc_version}"
    $SUDO update-alternatives \
      --install /usr/bin/gcc gcc "/usr/bin/gcc-${gcc_version}" 40 \
      --slave /usr/bin/g++ g++ "/usr/bin/g++-${gcc_version}"
    echo export CC="/usr/bin/gcc-${gcc_version}" >> ${ENV_FILE}
    echo export CXX="/usr/bin/g++-${gcc_version}" >> ${ENV_FILE}
    gcc-${gcc_version} --version
    g++-${gcc_version} --version
    ;;
  armv7l | aarch64)
    $SUDO apt-get install -y --no-install-recommends gcc
    $SUDO apt-get install -y --no-install-recommends g++
    $SUDO update-alternatives \
      --install /usr/bin/gcc gcc "$(readlink -f "$(which gcc)")" 40 \
      --slave /usr/bin/g++ g++ "$(readlink -f "$(which g++)")"
    echo export CC="$(readlink -f "$(which gcc)")" >> ${ENV_FILE}
    echo export CXX="$(readlink -f "$(which g++)")" >> ${ENV_FILE}
    ;;
esac

echo "download cmake"
case "$PACKARCH" in
  x86_64)
    cmake_version="3.16.0"
    cmake_dir="$BUILD_TOOLS/cmake/$cmake_version"
    if [[ ! -d "$cmake_dir" ]]; then
      mkdir -p "$cmake_dir"
      cmake_url="https://cmake.org/files/v${cmake_version%.*}/cmake-${cmake_version}-Linux-x86_64.tar.gz" 
      wget -q --show-progress --no-check-certificate -O - "$cmake_url" | tar --strip-components=1 -xz -C "$cmake_dir"
    fi
      echo export PATH="$cmake_dir/bin:\${PATH}" >> ${ENV_FILE}
    ;;
  armv7l | aarch64)
    $SUDO apt-get install -y --no-install-recommends cmake
    ;;
esac
cmake --version

echo export CFLAGS="'$CFLAGS'" >> $ENV_FILE
echo export CXXFLAGS="'$CXXFLAGS'" >> $ENV_FILE

# ms3 does not use ninja (yet)

##########################################################################
# POST INSTALL
##########################################################################

chmod +x "$ENV_FILE"

case "$PACKARCH" in
  armv7l | aarch64)
    # add an exception for the "detected dubious ownership in repository" (only seen inside a Docker image)
    git config --global --add safe.directory /MuseScore
    ;;
esac

df -h .

echo "setup.sh ended"