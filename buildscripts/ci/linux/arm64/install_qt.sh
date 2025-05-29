#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2025 MuseScore Limited
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

echo "Install Qt"
trap 'echo Qt installation failed; exit 1' ERR

# TODO: Update to Qt 6.9
apt_packages_qt6=(
    libqt6core5compat6-dev
    libqt6networkauth6-dev
    libqt6opengl6-dev
    libqt6printsupport6
    libqt6qml6
    libqt6quick6
    libqt6quickcontrols2-6
    libqt6quicktemplates2-6
    libqt6quickwidgets6
    libqt6svg6-dev
    libqt6websockets6-dev
    libqt6xml6
    qml6-module-* # installs all qml modules
    qt6-base-dev
    qt6-base-private-dev
    qt6-declarative-dev
    qt6-gtk-platformtheme
    qt6-l10n-tools
    qt6-scxml-dev
    qt6-tools-dev
    qt6-tools-dev-tools
    qt6-wayland
)

sudo apt-get install -y --no-install-recommends \
    "${apt_packages_qt6[@]}"

qt_dir="/usr/lib/aarch64-linux-gnu/qt6"

if [[ ! -d "${qt_dir}" ]]; then
    echo "Qt directory not found: ${qt_dir}"
    exit 1
fi

export QT_SELECT=qt6
echo QT_SELECT=qt6 >>${GITHUB_ENV}
echo QT_ROOT_DIR="${qt_dir}" >>${GITHUB_ENV}
echo QT_PLUGIN_PATH="${qt_dir}/plugins" >>${GITHUB_ENV}
echo QML2_IMPORT_PATH="${qt_dir}/qml" >>${GITHUB_ENV}
# explicitly set QMAKE path for linuxdeploy-plugin-qt
echo QMAKE="/usr/bin/qmake6" >>${GITHUB_ENV}

echo "${qt_dir}/bin" >>${GITHUB_PATH}

echo CFLAGS="-Wno-psabi" >>${GITHUB_ENV}
echo CXXFLAGS="-Wno-psabi" >>${GITHUB_ENV}

# https://askubuntu.com/questions/1460242/ubuntu-22-04-with-qt6-qmake-could-not-find-a-qt-installation-of
qtchooser -install qt6 $(which qmake6)
