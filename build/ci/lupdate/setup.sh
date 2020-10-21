#!/bin/bash

echo "Setup Linux environment for run lupdate"
trap 'echo Setup failed; exit 1' ERR

#  Go one-up from MuseScore root dir
cd ..

# Let's remove the file with environment variables to recreate it
ENV_FILE=./musescore_lupdate_environment.sh
rm -f ${ENV_FILE}

echo "echo 'Setup Linux environment for run lupdate'" >> ${ENV_FILE}

# Get newer Qt (only used cached version if it is the same)
qt_version="598"
qt_dir="Qt/${qt_version}"
if [[ ! -d "${qt_dir}" ]]; then
  mkdir -p "${qt_dir}"
  qt_url="https://s3.amazonaws.com/utils.musescore.org/qt${qt_version}.zip"
  wget -q --show-progress -O qt5.zip "${qt_url}"
  7z x -y qt5.zip -o"${qt_dir}"
  rm -f qt5.zip
fi
qt_path="${PWD%/}/${qt_dir}"

echo export PATH="${qt_path}/bin:\${PATH}" >> ${ENV_FILE}
echo export LD_LIBRARY_PATH="${qt_path}/lib:\${LD_LIBRARY_PATH}" >> ${ENV_FILE}
echo export QT_PATH="${qt_path}" >> ${ENV_FILE}
echo export QT_PLUGIN_PATH="${qt_path}/plugins" >> ${ENV_FILE}
echo export QML2_IMPORT_PATH="${qt_path}/qml" >> ${ENV_FILE}


chmod +x "${ENV_FILE}"

echo "Setup script done"