/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.InstrumentsScene 1.0

MenuButton {
    id: root

    signal addInstrumentRequested()
    signal addSystemMarkingsRequested()

    property bool addSystemMarkingsAvailable: true

    //: Keep in sync with the message that appears if there are no instruments in the score (LayoutPanel.qml)
    text: qsTrc("layoutpanel", "Add")

    navigation.name: "Add"
    accessible.name: qsTrc("layoutpanel", "Add instruments or system markings")

    icon: IconCode.NONE
    transparent: false
    accentButton: false

    menuModel: [
        { id: "ADD_INSTRUMENT", title: qsTrc("layoutpanel", "New instrument")  },
        {},
        { id: "ADD_SYSTEM_MARKINGS", title: qsTrc("layoutpanel", "System markings (tempo, rehearsal marks, etc.)"), enabled: root.addSystemMarkingsAvailable },
    ]

    onHandleMenuItem: function(itemId) {
        switch(itemId) {
        case "ADD_INSTRUMENT":
            root.addInstrumentRequested()
            break;
        case "ADD_SYSTEM_MARKINGS":
            root.addSystemMarkingsRequested()
            break;
        }
    }
}
