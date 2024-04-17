# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2022 MuseScore Limited
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

include(GetPlatformInfo)

if (OS_IS_MAC)
    install(FILES AppIcon/MS4_AppIcon.icns RENAME AppIcon.icns DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME})

    install(FILES MsczIcon/MS4_MsczIcon.icns RENAME MsczIcon.icns DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME})
    install(FILES MscxIcon/MS4_MscxIcon.icns RENAME MscxIcon.icns DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME})

elseif(OS_IS_WIN)
    # See also windows_icons.rc

    if (WIN_PORTABLE)
        install(FILES AppIcon/MS4_AppIcon.ico RENAME appicon.ico DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo)
        install(FILES AppIcon/MS4_AppIcon_16x16.png RENAME appicon_16.png DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo)
        install(FILES AppIcon/MS4_AppIcon_32x32.png RENAME appicon_32.png DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo)
        install(FILES AppIcon/MS4_AppIcon_128x128.png RENAME appicon_128.png DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo)
        install(FILES MsczIcon/MS4_MsczIcon.ico RENAME mscz.ico DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons)
        install(FILES MsczIcon/MS4_MsczIcon.png RENAME mscz.png DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons)
        install(FILES MscxIcon/MS4_MscxIcon.ico RENAME mscx.ico DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons)
        install(FILES MscxIcon/MS4_MscxIcon.png RENAME mscx.png DESTINATION ${CMAKE_INSTALL_PREFIX}/../AppInfo/FileTypeIcons)
    endif()

elseif(OS_IS_LIN)
    foreach(SIZE 16 24 32 48 64 96 128 512)
        install(FILES AppIcon/MS4_AppIcon_${SIZE}x${SIZE}.png RENAME mscore${MUSE_APP_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/${SIZE}x${SIZE}/apps)
    endforeach()

    install(FILES MsczIcon/MS4_MsczIcon.png RENAME application-x-musescore${MUSE_APP_INSTALL_SUFFIX}.png
            DESTINATION share/icons/hicolor/512x512/mimetypes)
    install(FILES MscxIcon/MS4_MscxIcon.png RENAME application-x-musescore${MUSE_APP_INSTALL_SUFFIX}+xml.png
            DESTINATION share/icons/hicolor/512x512/mimetypes)
    # Note: Must now run "gtk-update-icon-cache" to set the new icons.

endif()
