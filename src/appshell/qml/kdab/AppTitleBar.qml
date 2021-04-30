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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.AppShell 1.0

Rectangle {
    id: root

    color: ui.theme.backgroundPrimaryColor

    signal showWindowMinimizedRequested()
    signal toggleWindowMaximizedRequested()
    signal closeWindowRequested()
    signal startSystemMoveRequested()

    AppMenuModel {
        id: appMenuModel
    }

    Component.onCompleted: {
        appMenuModel.load()
    }

    TapHandler {
        gesturePolicy: TapHandler.DragThreshold
        onTapped: {
            if (tapCount === 2) {
                toggleWindowMaximizedRequested()
            }
        }
    }

    DragHandler {
        grabPermissions: TapHandler.CanTakeOverFromAnything
        onActiveChanged: {
            if (active) {
                startSystemMoveRequested()
            }
        }
    }

    Item {
        anchors.fill: parent
        anchors.margins: 8

        RadioButtonGroup {
            id: radioButtonList

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            height: 30
            width: contentItem.childrenRect.width

            model: appMenuModel.items

            delegate: FlatButton {
                id: radioButtonDelegate

                width: 60

                ButtonGroup.group: radioButtonList.radioButtonGroup

                normalStateColor: !_menu.isOpened ? "transparent" : ui.theme.accentColor
                text: modelData["title"]

                onClicked: {
                    _menu.toggleOpened()
                }

                StyledMenu {
                    id: _menu

                    model: modelData["subitems"]

                    onHandleAction: {
                        Qt.callLater(appMenuModel.handleAction, actionCode, actionIndex)
                        _menu.close()
                        console.log("selected " + actionCode)
                    }
                }
            }
        }

        StyledTextLabel {
            anchors.centerIn: parent

            horizontalAlignment: Text.AlignLeft
            text: qsTrc("appshell", "MuseScore 4")

            textFormat: Text.RichText
        }

        Row {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            FlatButton {
                icon: IconCode.MINUS
                normalStateColor: "transparent"

                onClicked: {
                    showWindowMinimizedRequested()
                }
            }

            FlatButton {
                icon: IconCode.SPLIT_OUT_ARROWS
                normalStateColor: "transparent"

                onClicked: {
                    toggleWindowMaximizedRequested()
                }
            }

            FlatButton {
                icon: IconCode.CLOSE_X_ROUNDED
                normalStateColor: "transparent"

                onClicked: {
                    closeWindowRequested()
                }
            }
        }
    }
}
