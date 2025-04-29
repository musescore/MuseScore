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

    property int leftPadding: 0
    property int rightPadding: 0
    property int topPadding: 0
    property int bottomPadding: 0

    property int rowHeight: 32
    property int separatorHeight: rowHeight

    property int maximumWidth: -1

    property NavigationPanel navigationPanel: NavigationPanel {
        name: root.objectName !== "" ? root.objectName : "ToolBarView"
        enabled: root.enabled && root.visible

        accessible.name: "ToolBar"
        accessible.visualItem: root
    }

    property var sourceComponentCallback

    width: content.width + leftPadding + rightPadding
    height: content.height + topPadding + bottomPadding

    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        if (root.model) {
            root.model.load()
        }
    }

    Flow {
        id: content

        anchors.left: root.left
        anchors.leftMargin: root.leftPadding
        anchors.top: root.top
        anchors.topMargin: root.topPadding

        width: {
            var result = 0
            var children = content.children

            for (var i = 0; i < children.length; ++i) {
                result += children[i].width + spacing

            }

            if (result > 0) {
                result -= spacing
            }

            return root.maximumWidth !== -1 ? Math.min(result, root.maximumWidth) : result
        }
        height: childrenRect.height

        clip: true
        spacing: 4

        Repeater {
            id: repeater

            Item {
                width: loader.width
                height: root.rowHeight

                Loader {
                    id: loader

                    anchors.verticalCenter: parent.verticalCenter

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

                        if (Boolean(loader.item.navigation)) {
                            loader.item.navigation.panel = root.navigationPanel
                            loader.item.navigation.order = model.index
                        }
                    }

                    Component {
                        id: separatorComp

                        SeparatorLine {
                            property var itemData: loader.itemData

                            width: 1
                            height: root.separatorHeight

                            orientation: Qt.Vertical
                        }
                    }

                    Component {
                        id: actionComp

                        StyledToolBarItem {
                            itemData: loader.itemData
                        }
                    }
                }
            }
        }
    }
}
