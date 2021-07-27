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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

ListItemBlank {
    id: root

    property string title: ""
    property int currentPartIndex: -1

    property int sideMargin: 0

    signal copyPartRequested()
    signal removePartRequested()
    signal partClicked()
    signal titleEdited()
    signal titleEditingFinished()

    function startEditTitle() {
        if (titleLoader.sourceComponent !== editPartTitleField) {
            titleLoader.sourceComponent = editPartTitleField
        }
    }

    function endEditTitle() {
        if (titleLoader.sourceComponent !== partTitle) {
            titleLoader.sourceComponent = partTitle
            titleEditingFinished()
        }
    }

    height: 42

    onClicked: {
        endEditTitle()
        root.partClicked()
    }

    onDoubleClicked: {
        root.startEditTitle()
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

            sourceComponent: partTitle

            Connections {
                target: root

                function onCurrentPartIndexChanged(currentPartIndex) {
                    root.endEditTitle()
                }
            }

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

                TextInputField {
                    Component.onCompleted: {
                        forceActiveFocus()
                    }

                    currentText: root.title

                    onCurrentTextEdited: {
                        root.titleEdited(newTextValue)
                    }

                    onTextEditingFinished: {
                        Qt.callLater(root.endEditTitle)
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        FlatButton {
            normalStateColor: "transparent"
            icon: IconCode.MENU_THREE_DOTS

            onClicked: {
                contextMenu.popup()
            }
        }
    }

    ContextMenu {
        id: contextMenu

        StyledContextMenuItem {
            id: duplicateItem

            text: qsTrc("notation", "Duplicate")

            onTriggered: {
                root.copyPartRequested()
            }
        }

        StyledContextMenuItem {
            id: deleteItem

            text: qsTrc("notation", "Delete")

            onTriggered: {
                root.removePartRequested()
            }
        }

        StyledContextMenuItem {
            id: renameItem

            text: qsTrc("notation", "Rename")

            onTriggered: {
                root.startEditTitle()
            }
        }
    }

    SeparatorLine {
        anchors.leftMargin: -root.anchors.leftMargin
        anchors.rightMargin: -root.anchors.rightMargin
        anchors.bottom: parent.bottom
    }
}
