#!/usr/bin/env bash

# For maximum AppImage compatibility, build on the oldest Linux distribution
# that still receives security updates from its manufacturer.

echo "Setup Linux build environment"
trap 'echo Setup failed; exit 1' ERR

df -h .

# Go one-up from MuseScore root dir regardless of where script was run from:
cd "$(dirname "$(readlink -f "${0}")")/../../../.."

# Let's remove the file with environment variables to recreate it
ENV_FILE=./musescore_environment.sh
rm -f ${ENV_FILE}

echo "echo 'Setup MuseScore build environment'" >> ${ENV_FILE}

##########################################################################
# GET DEPENDENCIES
##########################################################################

# DISTRIBUTION PACKAGES

# These are installed by default on Travis CI, but not on Docker
apt_packages_basic=(
  # Alphabetical order please!
  file
  git
  pkg-config
  software-properties-common # installs `add-apt-repository`
  unzip
  p7zip-full
  )

# These are the same as on Travis CI
apt_packages_standard=(
  # Alphabetical order please!
  curl
  libasound2-dev 
  libfontconfig1-dev
  libfreetype6-dev
  libgl1-mesa-dev
  libjack-dev
  libmp3lame-dev
  libnss3-dev
  libportmidi-dev
  libpulse-dev
  libsndfile1-dev
  make
  portaudio19-dev
  wget
  )

# MuseScore compiles without these but won't run without them
apt_packages_runtime=(
  # Alphabetical order please!
  libcups2
  libdbus-1-3
  libegl1-mesa-dev
  libodbc1
  libpq-dev
  libssl1.0.0
  libxcomposite-dev
  libxcursor-dev
  libxi-dev
  libxkbcommon-x11-0
  libxrandr2
  libxtst-dev
  libxcb-icccm4
  libxcb-image0
  libxcb-keysyms1
  libxcb-render-util0
  libxcb-xinerama0
  )

apt-get update # no package lists in Docker image
apt-get install -y --no-install-recommends \
  "${apt_packages_basic[@]}" \
  "${apt_packages_standard[@]}" \
  "${apt_packages_runtime[@]}"

##########################################################################
# GET QT
##########################################################################

# Get newer Qt (only used cached version if it is the same)
qt_version="5151"
qt_dir="Qt/${qt_version}"
if [[ ! -d "${qt_dir}" ]]; then
  mkdir -p "${qt_dir}"
  qt_url="https://s3.amazonaws.com/utils.musescore.org/Qt${qt_version}_gcc64.7z"
  wget -q --show-progress -O qt5.7z "${qt_url}"
  7z x -y qt5.7z -o"${qt_dir}"
fi
qt_path="${PWD%/}/${qt_dir}"

echo export PATH="${qt_path}/bin:\${PATH}" >> ${ENV_FILE}
echo export LD_LIBRARY_PATH="${qt_path}/lib:\${LD_LIBRARY_PATH}" >> ${ENV_FILE}
echo export QT_PATH="${qt_path}" >> ${ENV_FILE}
echo export QT_PLUGIN_PATH="${qt_path}/plugins" >> ${ENV_FILE}
echo export QML2_IMPORT_PATH="${qt_path}/qml" >> ${ENV_FILE}


##########################################################################
# GET TOOLS
##########################################################################

# COMPILER

gcc_version="7"
apt-get install -y --no-install-recommends "g++-${gcc_version}"
update-alternatives \
  --install /usr/bin/gcc gcc "/usr/bin/gcc-${gcc_version}" 40 \
  --slave /usr/bin/g++ g++ "/usr/bin/g++-${gcc_version}"

echo export CC="/usr/bin/gcc-${gcc_version}" >> ${ENV_FILE}
echo export CXX="/usr/bin/g++-${gcc_version}" >> ${ENV_FILE}

gcc-${gcc_version} --version
g++-${gcc_version} --version 

# CMAKE
# Get newer CMake (only used cached version if it is the same)
cmake_version="3.16.0"
cmake_dir="cmake/${cmake_version}"
if [[ ! -d "${cmake_dir}" ]]; then
  mkdir -p "${cmake_dir}"
  cmake_url="https://cmake.org/files/v${cmake_version%.*}/cmake-${cmake_version}-Linux-x86_64.tar.gz"
  wget -q --show-progress --no-check-certificate -O - "${cmake_url}" | tar --strip-components=1 -xz -C "${cmake_dir}"
fi
echo export PATH="${PWD%/}/${cmake_dir}/bin:\${PATH}" >> ${ENV_FILE}
export PATH="${PWD%/}/${cmake_dir}/bin:${PATH}"
cmake --version

##########################################################################
# OTHER
##########################################################################
wget -q --show-progress -O vst_sdk.7z "https://s3.amazonaws.com/utils.musescore.org/VST3_SDK_37.7z"
7z x -y vst_sdk.7z -o"$HOME/vst"
echo export VST3_SDK_PATH="$HOME/vst/VST3_SDK" >> $ENV_FILE

##########################################################################
# POST INSTALL
##########################################################################

chmod +x "${ENV_FILE}"

# # tidy up (reduce size of Docker image)
# apt-get clean autoclean
# apt-get autoremove --purge -y
# rm -rf /tmp/* /var/{cache,log,backups}/* /var/lib/apt/*

df -h .
echo "Setup script done"

