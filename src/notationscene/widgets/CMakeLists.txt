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

target_sources(notationscene PRIVATE
    alignSelect.cpp
    alignSelect.h
    breaksdialog.cpp
    breaksdialog.h
    colorlabel.cpp
    colorlabel.h
    editpitch.cpp
    editpitch.h
    editstaff.cpp
    editstaff.h
    editstafftype.cpp
    editstafftype.h
    editstringdata.cpp
    editstringdata.h
    editstyle.cpp
    editstyle.h
    editstyleutils.cpp
    editstyleutils.h
    exampleview.cpp
    exampleview.h
    fontStyleSelect.cpp
    fontStyleSelect.h
    measureproperties.cpp
    measureproperties.h
    offsetSelect.cpp
    offsetSelect.h
    pagesettings.cpp
    pagesettings.h
    realizeharmonydialog.cpp
    realizeharmonydialog.h
    selectdialog.cpp
    selectdialog.h
    selectnotedialog.cpp
    selectnotedialog.h
    stafftextpropertiesdialog.cpp
    stafftextpropertiesdialog.h
    timeline.cpp
    timeline.h
    transposedialog.cpp
    transposedialog.h
    tupletdialog.cpp
    tupletdialog.h
    voicingSelect.cpp
    voicingSelect.h
)

qt_add_resources(notationscene notationscene_widgets_resources
    PREFIX
        /
    FILES
        resources/std_sample.mscx
        resources/tab_sample.mscx
)
