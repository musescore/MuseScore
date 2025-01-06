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
import QtQuick.Layouts 1.12

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Item {
    id: root

    required property QtObject titleBarCpp

    property Component titleBarItem: null
    property var contextMenuModel: null

    property NavigationPanel navigationPanel: null
    property int navigationOrder: 0

    signal handleContextMenuItemRequested(string itemId)

    width: parent.width
    height: visible ? heightWhenVisible : 0

    visible: Boolean(titleBarCpp)

    Loader {
        id: titleBarLoader
        anchors.fill: parent

        property var titleBarCpp: root.titleBarCpp

        sourceComponent: root.titleBarItem ?? defaultTitleBarComponent

        onLoaded: {
            if (item) {
                item.navigationPanel = Qt.binding(function() { return root.navigationPanel})
                item.navigationOrder = Qt.binding(function() { return root.navigationOrder})
                item.contextMenuModel = Qt.binding(function() { return root.contextMenuModel})
            }
        }
    }

    Component {
        id: defaultTitleBarComponent

        Item {
            id: titleBar

            anchors.fill: parent
            implicitWidth: rowLayout.implicitWidth
            implicitHeight: rowLayout.implicitHeight

            property NavigationPanel navigationPanel
            property int navigationOrder
            property var contextMenuModel

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.NoButton
                cursorShape: Qt.SizeAllCursor
            }

            RowLayout {
                id: rowLayout
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12

                spacing: 4

                StyledTextLabel {
                    id: titleLabel
                    Layout.fillWidth: true

                    text: root.titleBarCpp?.title ?? ""
                    font: ui.theme.bodyBoldFont
                    horizontalAlignment: Qt.AlignLeft
                }

                MenuButton {
                    id: contextMenuButton

                    width: 20
                    height: width

                    navigation.panel: root.navigationPanel
                    navigation.order: root.navigationOrder
                    menuModel: root.contextMenuModel

                    onHandleMenuItem: function(itemId) {
                        root.handleContextMenuItemRequested(itemId)
                    }
                }
            }
        }
    }
}
