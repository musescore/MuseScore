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
import QtQuick 2.8
import QtQuick.Controls 2.1

import MuseScore.Palette 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Item {
    id: root

    property bool expanded: false
    property bool hovered: false
    property string text: ""
    property bool hidePaletteElementVisible
    property bool editingEnabled: true
    property bool custom: false
    property bool unresolved: false

    property PaletteWorkspace paletteWorkspace
    property var modelIndex: null

    property NavigationPanel navigationPanel: null
    property int navigationRow: 0

    signal toggleExpandRequested()
    signal enableEditingToggled(bool val)
    signal hideSelectedElementsRequested()
    signal insertNewPaletteRequested()
    signal hidePaletteRequested()
    signal editPalettePropertiesRequested()

    implicitHeight: paletteExpandArrow.height
    implicitWidth: paletteExpandArrow.implicitWidth + textItem.implicitWidth + rootMenuButton.implicitWidth + 8 // 8 for margins

    FlatButton {
        id: paletteExpandArrow
        z: 1000
        width: height
        visible: !root.unresolved // TODO: make a separate palette placeholder component
        activeFocusOnTab: false // same focus object as parent palette
        icon: root.expanded ? IconCode.SMALL_ARROW_DOWN : IconCode.SMALL_ARROW_RIGHT
        normalStateColor: "transparent"

        enabled: paletteExpandArrow.visible
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow
        navigation.column: 1

        onClicked: root.toggleExpandRequested()
    }

    StyledTextLabel {
        id: textItem
        height: parent.height
        horizontalAlignment: Text.AlignHLeft
        anchors {
            left: paletteExpandArrow.right; leftMargin: 4;
            right: deleteButton.visible ? deleteButton.left : (rootMenuButton.visible ? rootMenuButton.left : parent.right)
        }

        text: root.text
        font: ui.theme.bodyBoldFont
    }

    FlatButton {
        id: deleteButton

        anchors.right: rootMenuButton.left
        anchors.rightMargin: 6

        height: parent.height
        width: height

        z: 1000

        icon: IconCode.DELETE_TANK
        toolTipTitle: deleteButton.text
        visible: root.hidePaletteElementVisible && root.editingEnabled
        activeFocusOnTab: mainPalette.currentItem === paletteTree.currentTreeItem
        normalStateColor: "transparent"

        enabled: deleteButton.visible
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow
        navigation.column: 2

        onClicked: {
            hideSelectedElementsRequested()
        }
    }

    FlatButton {
        id: rootMenuButton
        z: 1000
        height: parent.height
        anchors.right: parent.right

        visible: root.expanded || root.hovered || menuLoader.isMenuOpened || rootMenuButton.navigation.active

        enabled: rootMenuButton.visible
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow
        navigation.column: 3

        icon: IconCode.MENU_THREE_DOTS
        normalStateColor: "transparent"

        onClicked: {
            menuLoader.toggleContextMenu(rootMenuButton.navigation)
        }
    }

    MouseArea {
        id: rightClickArea
        anchors.fill: parent
        acceptedButtons: Qt.RightButton

        onClicked: {
            menuLoader.toggleContextMenu(null)
        }
    }

    StyledMenuLoader {
        id: menuLoader

        function toggleContextMenu(navigationParentControl) {
            var items = [
                        {code: "hide", title: root.custom ? qsTrc("palette", "Hide/Delete Palette") : qsTrc("palette", "Hide Palette") },
                        {code: "new", title: qsTrc("palette", "Insert New Palette") },
                        {code: "", title: "" }, // separator
                        {code: "edit", title: qsTrc("palette", "Enable Editing"), checkable: true, checked: root.editingEnabled },
                        {code: "", title: "" }, // separator
                        {code: "reset", title: qsTrc("palette", "Reset Palette") },
                        {code: "save", title: qsTrc("palette", "Save Palette…") },
                        {code: "load", title: qsTrc("palette", "Load Palette…") },
                        {code: "", title: "" }, // separator
                        {code: "properties", title: qsTrc("palette", "Palette Properties…") },
                    ]

            toggleOpened(items, navigationParentControl)
        }

        onHandleAction: {
            switch(actionCode) {
            case "hide": root.hidePaletteRequested(); break
            case "new": root.insertNewPaletteRequested(); break
            case "edit": root.enableEditingToggled(!root.editingEnabled); break
            case "reset": root.paletteWorkspace.resetPalette(root.modelIndex); break
            case "save": root.paletteWorkspace.savePalette(root.modelIndex); break
            case "load": root.paletteWorkspace.loadPalette(root.modelIndex); break
            case "properties": Qt.callLater(root.editPalettePropertiesRequested); break
            }
        }
    }
}
