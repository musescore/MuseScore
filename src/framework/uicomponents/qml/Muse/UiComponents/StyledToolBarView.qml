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

    property NavigationPanel navigationPanel: NavigationPanel {
        name: root.objectName !== "" ? root.objectName : "ToolBarView"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.List
        accessible.name: "ToolBar"
        accessible.visualItem: root
    }

    signal sourceComponentRequested(var type)

    width: content.width + prv.padding * 2
    height: content.height + prv.padding * 2

    color: ui.theme.backgroundPrimaryColor

    QtObject {
        id: prv

        property int padding: 4
    }

    Flow {
        id: content

        anchors.verticalCenter: parent.verticalCenter

        clip: true
        spacing: 4

        Component.onCompleted: {
            model.load()
        }

        Repeater {
            id: repeater

            Loader {
                id: loader

                property var itemData: Boolean(model) ? model.itemRole : null

                width: Boolean(item) ? item.implicitWidth : 0
                height: Boolean(item) ? item.implicitHeight : 0

                sourceComponent: {
                    if (!Boolean(loader.itemData)) {
                        return null
                    }

                    var type = loader.itemData.type
                    var comp = root.sourceComponentRequested(type)
                    if (comp) {
                        return comp
                    }

                    switch(type) {
                    case ToolBarItemType.ACTION: return actionComp
                    case ToolBarItemType.SEPARATOR: return separatorComp
                    }

                    return null
                }

                Component {
                    id: separatorComp

                    SeparatorLine {
                        orientation: Qt.Vertical
                    }
                }

                Component {
                    id: actionComp

                    StyledToolBarItem {
                        id: btn

                        item: loader.itemData

                        navigation.panel: root.navigationPanel
                        navigation.order: model.index
                    }
                }
            }
        }
    }
}
