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
import QtQuick

import Muse.Ui
import Muse.UiComponents

StyledPopupView {
    id: root

    property int popupAvailableWidth: 0

    contentWidth: root.popupAvailableWidth - 2 * root.margins
    contentHeight: contentColumn.implicitHeight

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "CreateCustomPalettePopup"
        section: root.navigationSection
        order: 1
        direction: NavigationPanel.Vertical
    }

    signal addCustomPaletteRequested(var paletteName)

    onOpened: {
        paletteNameField.forceActiveFocus()
        paletteNameField.navigation.requestActive()
    }

    Column {
        id: contentColumn

        anchors.fill: parent
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

            navigation.panel: root.navigationPanel
            navigation.row: 1

            property string name: ""

            onTextChanged: function(newTextValue) {
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

                navigation.panel: root.navigationPanel
                navigation.row: 2

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

                navigation.panel: root.navigationPanel
                navigation.row: 3

                onClicked: {
                    root.addCustomPaletteRequested(paletteNameField.name)
                    parent.close()
                }
            }
        }
    }
}
