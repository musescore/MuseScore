/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

Rectangle {
    id: root

    property string hintText: ""
    property string hintPlainText: ""
    property int maxLinkLength: 0

    property NavigationPanel navigationPanel: null
    property int firstNavigationOrder: 0

    signal linkSubmitted(string link)

    readonly property int contentPadding: 24

    visible: Boolean(root.hintText)

    implicitHeight: linkPasteColumn.implicitHeight + 2 * root.contentPadding

    color: ui.theme.backgroundSecondaryColor
    border.width: 1
    border.color: ui.theme.strokeColor
    radius: 3

    Column {
        id: linkPasteColumn

        anchors.fill: parent
        anchors.margins: root.contentPadding

        spacing: 8

        StyledTextLabel {
            width: parent.width

            text: root.hintText
            font: ui.theme.bodyFont
            horizontalAlignment: Text.AlignLeft
        }

        RowLayout {
            width: parent.width
            spacing: 8

            TextInputField {
                id: linkInputField

                Layout.fillWidth: true
                implicitWidth: 0

                hint: "https://"
                maximumLength: root.maxLinkLength > 0 ? root.maxLinkLength : 32767

                navigation.panel: root.navigationPanel
                navigation.order: root.firstNavigationOrder
                navigation.accessible.name: qsTrc("project/convert", "Link") + " " + linkInputField.currentText
                navigation.accessible.description: root.hintPlainText

                onTextChanged: function(newTextValue) {
                    linkInputField.currentText = newTextValue
                }
            }

            FlatButton {
                Layout.preferredWidth: 90
                Layout.preferredHeight: 30

                text: qsTrc("global", "Next")

                accentButton: true
                enabled: Boolean(linkInputField.currentText.trim())

                navigation.panel: root.navigationPanel
                navigation.order: root.firstNavigationOrder + 1

                onClicked: {
                    root.linkSubmitted(linkInputField.currentText.trim())
                }
            }
        }
    }
}
