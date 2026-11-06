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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

import MuseScore.InstrumentsScene

RowLayout {
    id: root

    required property AbstractLayoutPanelTreeItem item
    required property int depth
    required property bool isExpanded
    required property bool isEnabled
    required property bool isStaveSharingEnabled
    required property bool isHovered
    required property bool isHighlighted

    required property NavigationPanel navigationPanel
    required property int navigationRow

    readonly property int type: root.item ? root.item.type : LayoutPanelItemType.UNDEFINED
    readonly property string title: root.item ? root.item.title : ""
    readonly property bool isItemVisible: Boolean(root.item) && root.item.isVisible
    readonly property alias isPopupOpened: settingsPopup.isOpened

    signal visibilityToggled(bool visible)
    signal expandToggled(bool expand)
    signal enabledToggled(bool enabled)

    spacing: 2

    Loader {
        Layout.fillWidth: true
        Layout.fillHeight: true

        sourceComponent: root.type === LayoutPanelItemType.SYSTEM_OBJECTS_LAYER ? systemObjectsLayerTitleComp
                                                                                : visibilityControlsComp
    }

    FlatButton {
        id: settingsButton

        visible: Boolean(root.item) && root.item.settingsAvailable
        enabled: root.visible && Boolean(root.item) && root.item.settingsEnabled

        // Transparent rather than hidden, to stay keyboard-accessible
        opacity: root.isHovered || root.isHighlighted || settingsPopup.isOpened || settingsButton.mouseArea.containsMouse
                 || settingsButton.navigation.highlight || sharedPartToggle.navigation.highlight ? 1 : 0

        transparent: !settingsPopup.isOpened
        accentButton: settingsPopup.isOpened

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

    Component {
        id: visibilityControlsComp

        VisibilityControls {
            navigationPanel: root.navigationPanel
            navigationRow: root.navigationRow

            title: root.title
            isRootControl: root.type === LayoutPanelItemType.PART || root.type === LayoutPanelItemType.SHARED_PART

            isVisible: root.isItemVisible
            onVisibilityButtonClicked: function(isVisible) {
                root.visibilityToggled(!isVisible)
            }

            isEnabled: root.isEnabled

            isExpandable: Boolean(root.item) && root.item.isExpandable
            isExpanded: root.isExpanded
            expandableDepth: root.depth
            onExpandButtonClicked: function(expand) {
                root.expandToggled(expand)
            }
        }
    }

    Component {
        id: systemObjectsLayerTitleComp

        RowLayout {
            spacing: 2

            StyledIconLabel {
                Layout.preferredWidth: ui.theme.defaultButtonSize // same as the eye, so the titles line up

                iconCode: IconCode.MINUS
                opacity: root.isItemVisible ? 1 : 0.75
            }

            StyledTextLabel {
                Layout.fillWidth: true
                Layout.leftMargin: 4
                Layout.rightMargin: 8

                text: root.title
                horizontalAlignment: Text.AlignLeft
                opacity: root.isItemVisible ? 1 : 0.75
            }
        }
    }
}
