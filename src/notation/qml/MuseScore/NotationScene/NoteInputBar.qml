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
import QtQuick 2.12
import QtQuick.Controls 2.12

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Rectangle {
    id: root

    property alias orientation: gridView.orientation

    property alias navigation: keynavSub

    color: ui.theme.backgroundPrimaryColor

    QtObject {
        id: privatesProperties

        property bool isHorizontal: orientation === Qt.Horizontal
    }

    NavigationPanel {
        id: keynavSub
        name: "NoteInputBar"
    }

    NoteInputBarModel {
        id: noteInputModel
    }

    Component.onCompleted: {
        noteInputModel.load()
    }

    GridViewSectional {
        id: gridView
        anchors.fill: parent

        sectionRole: "sectionRole"

        rowSpacing: 6
        columnSpacing: 6

        cellWidth: 36
        cellHeight: cellWidth

        clip: true

        model: noteInputModel

        sectionDelegate: SeparatorLine {
            orientation: gridView.isHorizontal ? Qt.Vertical : Qt.Horizontal
            visible: itemIndex !== 0
        }

        itemDelegate: FlatButton {
            id: btn
            property var item: Boolean(itemModel) ? itemModel : null
            property var hasMenu: Boolean(item) && item.subitemsRole.length !== 0

            accentButton: (Boolean(item) && item.checkedRole) || menuLoader.isMenuOpened
            normalStateColor: accentButton ? ui.theme.accentColor : "transparent"

            icon: Boolean(item) ? item.iconRole : IconCode.NONE

            toolTipTitle: Boolean(item) ? item.titleRole : ""
            toolTipDescription: Boolean(item) ? item.descriptionRole : ""
            toolTipShortcut: Boolean(item) ? item.shortcutRole : ""

            iconFont: ui.theme.toolbarIconsFont

            navigation.panel: keynavSub
            navigation.name: toolTipTitle
            navigation.order: Boolean(item) ? item.orderRole : 0
            isClickOnKeyNavTriggered: false
            navigation.onTriggered: {
                if (hasMenu && item.isMenuSecondaryRole) {
                    btn.pressAndHold()
                } else {
                    btn.clicked()
                }
            }

            mouseArea.pressAndHoldInterval: 200

            width: gridView.cellWidth
            height: gridView.cellWidth

            mouseArea.acceptedButtons: hasMenu && item.isMenuSecondaryRole
                                       ? Qt.LeftButton | Qt.RightButton
                                       : Qt.LeftButton

            onClicked: function (mouse) {
                if (menuLoader.isMenuOpened // If already menu open, close it
                        || (hasMenu // Or if can open menu
                            && (!item.isMenuSecondaryRole // And _should_ open menu
                                || mouse.button === Qt.RightButton))) {
                    menuLoader.toggleOpened(item.subitemsRole, btn.navigation)
                    return
                }

                if (mouse.button === Qt.LeftButton) {
                    Qt.callLater(noteInputModel.handleAction, item.codeRole)
                }
            }

            onPressAndHold: {
                if (menuLoader.isMenuOpened || !hasMenu) {
                    return
                }

                menuLoader.toggleOpened(item.subitemsRole, btn.navigation)
            }

            Canvas {
                visible: Boolean(btn.item) && btn.item.isMenuSecondaryRole

                width: 4
                height: 4

                anchors.margins: 2
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                onPaint: {
                    const ctx = getContext("2d");
                    ctx.fillStyle = ui.theme.fontPrimaryColor;
                    ctx.moveTo(width, 0);
                    ctx.lineTo(width, height);
                    ctx.lineTo(0, height);
                    ctx.closePath();
                    ctx.fill();
                }
            }

            StyledMenuLoader {
                id: menuLoader
                onHandleAction: noteInputModel.handleAction(actionCode, actionIndex)
            }
        }
    }

    FlatButton {
        id: customizeButton

        anchors.margins: 8

        width: gridView.cellWidth
        height: gridView.cellHeight

        icon: IconCode.CONFIGURE
        iconFont: ui.theme.toolbarIconsFont
        normalStateColor: "transparent"
        navigation.panel: keynavSub
        navigation.order: 100

        onClicked: {
            api.launcher.open("musescore://notation/noteinputbar/customise")
        }
    }

    states: [
        State {
            when: privatesProperties.isHorizontal
            PropertyChanges {
                target: gridView
                sectionWidth: 1
                sectionHeight: root.height
                rows: 1
                columns: gridView.noLimit
            }

            AnchorChanges {
                target: customizeButton
                anchors.right: root.right
                anchors.verticalCenter: root.verticalCenter
            }
        },
        State {
            when: !privatesProperties.isHorizontal
            PropertyChanges {
                target: gridView
                sectionWidth: root.width
                sectionHeight: 1
                rows: gridView.noLimit
                columns: 2
            }

            AnchorChanges {
                target: customizeButton
                anchors.bottom: root.bottom
                anchors.right: root.right
            }
        }
    ]
}
