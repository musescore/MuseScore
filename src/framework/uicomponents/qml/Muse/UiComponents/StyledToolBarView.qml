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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Rectangle {
    id: root

    property alias model: repeater.model

    property alias spacing: content.spacing
    property int rowHeight: 32

    property NavigationPanel navigationPanel: NavigationPanel {
        name: root.objectName !== "" ? root.objectName : "ToolBarView"
        enabled: root.enabled && root.visible

        accessible.name: "ToolBar"
        accessible.visualItem: root
    }

    property var sourceComponentCallback

    width: content.width + prv.padding * 2
    height: content.height + prv.padding * 2

    color: ui.theme.backgroundPrimaryColor

    QtObject {
        id: prv

        property int padding: 4
    }

    Component.onCompleted: {
        root.model.load()
    }

    Flow {
        id: content

        anchors.verticalCenter: parent.verticalCenter

        clip: true
        spacing: 4

        Repeater {
            id: repeater

            Loader {
                id: loader

                property var itemData: Boolean(model) ? model.itemRole : null

                sourceComponent: {
                    if (!Boolean(loader.itemData)) {
                        return null
                    }

                    var type = loader.itemData.type

                    var comp = Boolean(root.sourceComponentCallback) ? root.sourceComponentCallback(type) : null
                    if (Boolean(comp)) {
                        return comp
                    }

                    switch(type) {
                    case ToolBarItemType.ACTION: return actionComp
                    case ToolBarItemType.SEPARATOR: return separatorComp
                    }

                    return null
                }

                onLoaded: {
                    loader.item.itemData = loader.itemData

                    loader.item.y = (root.rowHeight - loader.item.height) / 2 // equal to anchors.verticalCenter = parent.verticalCenter
                }

                Component {
                    id: separatorComp

                    SeparatorLine {
                        property var itemData: loader.itemData

                        width: 1
                        height: root.rowHeight

                        orientation: Qt.Vertical
                    }
                }

                Component {
                    id: actionComp

                    StyledToolBarItem {
                        id: btn

                        itemData: loader.itemData

                        navigation.panel: root.navigationPanel
                        navigation.order: model.index
                    }
                }
            }
        }
    }
}
