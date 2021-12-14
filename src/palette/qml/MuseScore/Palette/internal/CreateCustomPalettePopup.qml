/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

StyledPopup {
    id: root

    signal addCustomPaletteRequested(var paletteName)

    width: parent.width
    height: contentColumn.implicitHeight + topPadding + bottomPadding

    navigation.direction: NavigationPanel.Both
    navigation.name: "CreateCustomPalettePopup"

    onOpened: {
        paletteNameField.forceActiveFocus()
        paletteNameField.navigation.requestActive()
    }

    Column {
        id: contentColumn
        width: parent.width
        spacing: 12

        StyledTextLabel {
            width: parent.width
            text: qsTrc("palette", "Name your custom palette")
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignLeft
        }

        TextInputField {
            id: paletteNameField
            width: parent.width

            navigation.panel: root.navigation
            navigation.row: 1
            navigation.column: 1

            property string name: ""

            onCurrentTextEdited: function(newTextValue) {
                name = newTextValue
            }
        }

        Row {
            width: parent.width
            spacing: 12

            function close() {
                paletteNameField.clear()
                root.close()
            }

            FlatButton {
                text: qsTrc("global", "Cancel")
                width: (parent.width - parent.spacing) / 2
                accentButton: !createButton.enabled

                navigation.panel: root.navigation
                navigation.row: 2
                navigation.column: 1

                onClicked: {
                    parent.close()
                }
            }

            FlatButton {
                id: createButton
                text: qsTrc("palette", "Create")
                width: (parent.width - parent.spacing) / 2
                enabled: Boolean(paletteNameField.name)
                accentButton: enabled

                navigation.panel: root.navigation
                navigation.row: 2
                navigation.column: 2

                onClicked: {
                    root.addCustomPaletteRequested(paletteNameField.name)
                    parent.close()
                }
            }
        }
    }
}
