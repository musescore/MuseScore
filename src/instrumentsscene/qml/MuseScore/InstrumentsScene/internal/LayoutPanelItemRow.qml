/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents

import MuseScore.InstrumentsScene

VisibilityControls {
    id: root

    required property AbstractLayoutPanelTreeItem item
    required property bool isStaveSharingEnabled

    readonly property int type: root.item ? root.item.type : LayoutPanelItemType.UNDEFINED
    readonly property alias isPopupOpened: settingsPopup.isOpened

    signal enabledToggled(bool enabled)

    title: root.item ? root.item.title : ""
    isRootControl: Boolean(root.item) && (root.type === LayoutPanelItemType.PART || root.type === LayoutPanelItemType.SHARED_PART)

    useVisibilityButton: root.type !== LayoutPanelItemType.SYSTEM_OBJECTS_LAYER
    isVisible: Boolean(root.item) && root.item.isVisible

    showDashIcon: root.type === LayoutPanelItemType.SYSTEM_OBJECTS_LAYER

    isExpandable: Boolean(root.item) && root.item.isExpandable

    FlatButton {
        id: settingsButton

        visible: Boolean(root.item) && root.item.settingsAvailable
        enabled: root.visible && Boolean(root.item) && root.item.settingsEnabled

        objectName: "SettingsBtn"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow
        navigation.column: 3
        navigation.accessible.name: qsTrc("layoutpanel", "Settings")

        icon: IconCode.SETTINGS_COG

        onClicked: {
            settingsPopup.toggle(settingsButton)
        }

        Behavior on opacity {
            NumberAnimation { duration: 150 }
        }

        LayoutPanelItemSettingsPopup {
            id: settingsPopup

            item: root.item
        }
    }

    ToggleButton {
        id: sharedPartToggle
        visible: root.type === LayoutPanelItemType.SHARED_PART
        enabled: root.isStaveSharingEnabled
        checked: Boolean(root.item) && root.item.isEnabled
        onToggled: function() {
            root.enabledToggled(!sharedPartToggle.checked)
        }
    }
}
