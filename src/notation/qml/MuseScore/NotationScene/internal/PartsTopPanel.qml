/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

Item {
    id: root

    property int sideMargin: 0

    signal createNewPartRequested()

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "PartsControlPanel"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Horizontal

        //: Accessibility description of the button group at the top of the "Parts" dialog
        accessible.name: qsTrc("notation", "Parts actions")

        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }
    }

    StyledTextLabel {
        anchors.left: parent.left
        anchors.leftMargin: root.sideMargin

        text: qsTrc("notation", "Parts")
        font: ui.theme.headerBoldFont
    }

    FlatButton {
        text: qsTrc("notation", "Create new part")

        anchors.right: parent.right
        anchors.rightMargin: root.sideMargin

        navigation.name: "CreateNewPartButton"
        navigation.panel: root.navigationPanel
        navigation.column: 0

        onClicked: {
            root.createNewPartRequested()
        }
    }
}
