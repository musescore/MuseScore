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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

ListItemBlank {
    id: root

    property string title: ""
    property string incorrectTitleWarning: ""

    property int currentPartIndex: -1

    property bool canReset: false
    property bool canDelete: false

    property int sideMargin: 0

    navigation.column: 0
    navigation.accessible.name: title

    signal copyPartRequested()
    signal resetPartRequested()
    signal removePartRequested()
    signal partClicked()

    signal titleEdited(string newTitle)
    signal titleEditingFinished(string newTitle)

    function startEditTitle() {
        if (titleLoader.sourceComponent !== editPartTitleField) {
            titleLoader.sourceComponent = editPartTitleField
        }
    }

    function endEditTitle(newTitle) {
        if (titleLoader.sourceComponent !== partTitle) {
            titleLoader.sourceComponent = partTitle
            root.incorrectTitleWarning = ""
            root.titleEditingFinished(newTitle)
            root.navigation.requestActive()
        }
    }

    onCurrentPartIndexChanged: {
        root.endEditTitle()
    }

    height: 42

    onClicked: {
        endEditTitle()
        root.partClicked()
    }

    onDoubleClicked: {
        root.startEditTitle()
    }

    onIsSelectedChanged: {
        if (isSelected && !navigation.active) {
            navigation.requestActive()
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: root.sideMargin - partIcon.width/3
        anchors.rightMargin: root.sideMargin

        spacing: 4

        StyledIconLabel {
            id: partIcon

            iconCode: IconCode.NEW_FILE
        }

        Loader {
            id: titleLoader

            Layout.fillWidth: true

            sourceComponent: partTitle

            Component {
                id: partTitle

                StyledTextLabel {
                    text: root.title

                    horizontalAlignment: Qt.AlignLeft
                    font: ui.theme.bodyBoldFont
                }
            }

            Component {
                id: editPartTitleField


                RowLayout {
                    spacing: 38

                    TextInputField {
                        Layout.fillWidth: true

                        navigation.panel: root.navigation.panel
                        navigation.row: root.navigation.row
                        navigation.column: 1

                        maximumLength: 40

                        Component.onCompleted: {
                            forceActiveFocus()
                            navigation.requestActive()
                        }

                        currentText: root.title

                        onTextChanged: function(newTextValue) {
                            root.titleEdited(newTextValue)
                        }

                        onTextEditingFinished: function(newTextValue) {
                            Qt.callLater(root.endEditTitle, newTextValue)
                        }
                    }

                    StyledTextLabel {
                        Layout.preferredWidth: parent.width / 3

                        text: root.incorrectTitleWarning

                        horizontalAlignment: Text.AlignLeft
                    }
                }
            }
        }

        MenuButton {
            Component.onCompleted: {
                var operations = [
                            { "id": "duplicate", "title": qsTrc("notation", "Duplicate") },
                            { "id": "rename", "title": qsTrc("notation", "Rename") },
                            { "id": "reset", "title": qsTrc("notation", "Reset"), "enabled": root.canReset }
                        ]

                if (root.canDelete) {
                    operations.push({ "id": "delete", "title": qsTrc("notation", "Delete") })
                }

                menuModel = operations
            }

            navigation.name: title
            navigation.panel: root.navigation.panel
            navigation.row: root.navigation.row
            navigation.column: 2

            onHandleMenuItem: function(itemId) {
                switch(itemId) {
                case "duplicate":
                    root.copyPartRequested()
                    break
                case "rename":
                    root.startEditTitle()
                    break
                case "reset":
                    root.resetPartRequested()
                    break
                case "delete":
                    root.removePartRequested()
                    break
                }
            }
        }
    }

    SeparatorLine {
        anchors.bottom: parent.bottom
    }
}
