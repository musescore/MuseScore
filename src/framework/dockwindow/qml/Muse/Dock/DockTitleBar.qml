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

import "qrc:/kddockwidgets/private/quick/qml/" as KDDW

Item {
    id: root

    required property QtObject titleBarCpp

    property Component titleBarItem: null
    property var contextMenuModel: null
    property real heightWhenVisible: titleBarLoader.item?.heightWhenVisible ?? 0
    property NavigationPanel navigationPanel: null
    property int navigationOrder: 0
    property bool isHorizontalPanel: false

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

        KDDW.TitleBarBase {
            id: titleBar

            anchors.fill: parent

            implicitHeight: titleBarContent.implicitHeight
            heightWhenVisible: titleBarContent.implicitHeight
            color: ui.theme.backgroundPrimaryColor

            property NavigationPanel navigationPanel
            property int navigationOrder
            property var contextMenuModel

            visible: parent.visible

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                acceptedButtons: Qt.NoButton
                cursorShape: Qt.SizeAllCursor
            }

            Column {
                id: titleBarContent

                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12

                spacing: 0

                RowLayout {
                    width: parent.width
                    height: 34

                    StyledTextLabel {
                        id: titleLabel
                        Layout.fillWidth: true

                        text: titleBar.title
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

                SeparatorLine {
                    id: bottomSeparator
                    orientation: Qt.Horizontal
                    anchors.margins: -12
                    visible: root.isHorizontalPanel
                }
            }
        }
    }
}
