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
import QtQuick 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    color: ui.theme.backgroundPrimaryColor

    property alias title: titleLabel.text
    property alias text: textLabel.text
    property alias textFormat: textLabel.textFormat

    property bool withIcon: false
    property alias iconCode: icon.iconCode

    property bool withShowAgain: false

    property var buttons: []
    property int defaultButtonId: 0

    property var contentWidth: Math.max(textContent.contentWidth, buttons.contentWidth)
    property var contentHeight: content.contentHeight

    signal clicked(int buttonId, bool showAgain)

    ColumnLayout {
        id: content

        anchors.fill: parent

        property int contentHeight: childrenRect.height

        spacing: 16

        RowLayout {
            id: textContent

            property int contentWidth: childrenRect.width

            Layout.preferredWidth: childrenRect.width
            Layout.preferredHeight: childrenRect.height

            spacing: 28

            StyledIconLabel {
                id: icon

                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 48
                Layout.preferredHeight: 48

                font.pixelSize: 48

                visible: root.withIcon && !isEmpty
            }

            Column {
                property int _preferredWidth: Math.max(Math.min(titleLabel.implicitWidth, 420),
                                                       Math.min(textLabel.implicitWidth, 420))

                Layout.preferredWidth: _preferredWidth + /*todo*/ 76
                Layout.preferredHeight: childrenRect.height

                spacing: 16

                StyledTextLabel {
                    id: titleLabel

                    width: parent._preferredWidth + /*todo*/ 4

                    font: ui.theme.largeBodyBoldFont
                    horizontalAlignment: Text.AlignLeft
                    maximumLineCount: 2
                    wrapMode: Text.WordWrap
                }

                StyledTextLabel {
                    id: textLabel

                    width: parent._preferredWidth + /*todo*/ 4

                    horizontalAlignment: Text.AlignLeft
                    maximumLineCount: 3
                    wrapMode: Text.WordWrap

                    visible: !isEmpty
                }

                CheckBox {
                    id: withShowAgainCheckBox

                    Layout.fillWidth: true

                    text: qsTrc("ui", "Show this message again")

                    checked: true

                    visible: Boolean(root.withShowAgain)

                    onClicked: {
                        checked = !checked
                    }
                }
            }
        }

        Item {
            Layout.preferredHeight: 30
            Layout.fillWidth: true

            ListView {
                id: buttons
                anchors.right: parent.right
                spacing: 12

                width: contentWidth
                height: contentHeight

                model: root.buttons
                orientation: Qt.Horizontal

                delegate: FlatButton {
                    text: modelData.title
                    accentButton: Boolean(modelData.accent)

                    onClicked: {
                        root.clicked(modelData.buttonId, withShowAgainCheckBox.checked)
                    }
                }
            }
        }
    }
}
