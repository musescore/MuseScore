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

file(GLOB TS_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/*.ts)

if (MUE_RUN_LRELEASE)
    qt5_add_translation(QM_FILES ${TS_FILES})

    add_custom_target(lrelease ALL
        DEPENDS ${QM_FILES}
        COMMENT "Generating .qm files" VERBATIM
    )
endif()

install(FILES
    ${CMAKE_CURRENT_LIST_DIR}/languages.json
    ${QM_FILES}
    DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}locale
)

install(DIRECTORY ${QT_INSTALL_PREFIX}/translations/
    DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}locale
    FILES_MATCHING
    REGEX "qt_.*\\.qm"
    REGEX "qt_help_.*\\.qm" EXCLUDE
    REGEX "qtbase_.*\\.qm"
    REGEX "qt*_en\\.qm" EXCLUDE)
