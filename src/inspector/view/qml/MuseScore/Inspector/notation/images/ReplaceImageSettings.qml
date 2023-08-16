/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0

Column {
    id: root
    
    property QtObject model: null

    property NavigationPanel navigationPanel: null

    property bool keepSize: true

    width: parent.width
    height: implicitHeight

    spacing: 12

    function forceFocusIn() {
        keepSizeRadioButton.requestActive()
    }

    RoundedRadioButton {
        id: keepSizeRadioButton

        navigation.panel: root.navigationPanel
        navigation.name: "KeepSizeRadioButton"
        navigation.row: 1

        text: qsTrc("inspector", "Keep current image size")
        checked: root.keepSize
        width: parent.width

        onToggled: root.keepSize = !root.keepSize
    }

    RoundedRadioButton {
        id: newSizeRadioButton

        navigation.panel: root.navigationPanel
        navigation.name: "NewSizeRadioButton"
        navigation.row: keepSizeRadioButton.navigation.row + 1

        text: qsTrc("inspector", "Use new image size")
        checked: !root.keepSize
        width: parent.width

        onToggled: root.keepSize = !root.keepSize
    }

    FlatButton {
        id: replaceImageButton

        navigation.panel: root.navigationPanel
        navigation.name: "ReplaceImageButton"
        navigation.row: newSizeRadioButton.navigation.row + 1

        text: qsTrc("inspector", "Replace image")
        width: parent.width

        onClicked: {
            if (root.model) {
                root.model.replaceImage(root.keepSize)
            }
        }
    }
}
