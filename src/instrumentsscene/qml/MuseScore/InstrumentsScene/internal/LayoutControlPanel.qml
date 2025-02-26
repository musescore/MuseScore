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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import MuseScore.InstrumentsScene 1.0

RowLayout {
    id: root

    property bool isMovingUpAvailable: false
    property bool isMovingDownAvailable: false
    property bool isRemovingAvailable: false
    property bool isAddingAvailable: value
    property bool isAddingSystemMarkingsAvailable: value

    property int selectedItemsType: LayoutPanelItemType.UNDEFINED

    property alias navigation: keynavSub

    signal addInstrumentRequested()
    signal addSystemMarkingsRequested()
    signal moveUpRequested()
    signal moveDownRequested()
    signal removingRequested()

    spacing: 6

    focus: true

    Keys.onShortcutOverride: function(event) {
        if (event.key === Qt.Key_Delete) {
            root.removingRequested()
        }
    }

    NavigationPanel {
        id: keynavSub
        name: "InstrumentsHeader"
        enabled: root.enabled && root.visible
    }

    LayoutPanelAddButton {
        Layout.fillWidth: true

        navigation.panel: keynavSub
        navigation.order: 1

        enabled: root.isAddingAvailable
        addSystemMarkingsAvailable: root.isAddingSystemMarkingsAvailable

        onAddInstrumentRequested: {
            root.addInstrumentRequested()
        }

        onAddSystemMarkingsRequested: {
            root.addSystemMarkingsRequested()
        }
    }

    FlatButton {
        Layout.preferredWidth: width

        navigation.name: "Up"
        navigation.panel: keynavSub
        navigation.order: 2

        toolTipTitle: {
            switch(root.selectedItemsType) {
            case LayoutPanelItemType.PART: return qsTrc("layoutpanel", "Move selected instruments up")
            case LayoutPanelItemType.STAFF: return qsTrc("layoutpanel", "Move selected staves up")
            case LayoutPanelItemType.SYSTEM_OBJECTS_LAYER: return qsTrc("layoutpanel", "Move selected system markings up")
            default: return ""
            }
        }

        enabled: root.isMovingUpAvailable

        icon: IconCode.ARROW_UP

        onClicked: {
            root.moveUpRequested()
        }
    }

    FlatButton {
        Layout.preferredWidth: width

        navigation.name: "Down"
        navigation.panel: keynavSub
        navigation.order: 3

        toolTipTitle: {
            switch(root.selectedItemsType) {
            case LayoutPanelItemType.PART: return qsTrc("layoutpanel", "Move selected instruments down")
            case LayoutPanelItemType.STAFF: return qsTrc("layoutpanel", "Move selected staves down")
            case LayoutPanelItemType.SYSTEM_OBJECTS_LAYER: return qsTrc("layoutpanel", "Move selected system markings down")
            default: return ""
            }
        }

        enabled: root.isMovingDownAvailable

        icon: IconCode.ARROW_DOWN

        onClicked: {
            root.moveDownRequested()
        }
    }

    FlatButton {
        Layout.preferredWidth: width

        navigation.name: "Remove"
        navigation.panel: keynavSub
        navigation.order: 4

        toolTipTitle: {
            switch(root.selectedItemsType) {
            case LayoutPanelItemType.PART: return qsTrc("layoutpanel", "Remove selected instruments")
            case LayoutPanelItemType.STAFF: return qsTrc("layoutpanel", "Remove selected staves")
            case LayoutPanelItemType.SYSTEM_OBJECTS_LAYER: return qsTrc("layoutpanel", "Remove selected system markings")
            default: return ""
            }
        }

        enabled: root.isRemovingAvailable

        icon: IconCode.DELETE_TANK

        onClicked: {
            root.removingRequested()
        }
    }
}
