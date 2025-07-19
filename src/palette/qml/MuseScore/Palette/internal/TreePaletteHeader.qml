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
import QtQuick 2.8
import QtQuick.Controls 2.1

import MuseScore.Palette 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

Item {
    id: root

    property bool expanded: false
    property bool hovered: false
    property string text: ""
    property bool hidePaletteElementVisible
    property bool editingEnabled: true
    property bool custom: false
    property bool unresolved: false

    property bool isInVisibleArea: true

    property PaletteProvider paletteProvider
    property var modelIndex: null

    property NavigationPanel navigationPanel: null
    property int navigationRow: 0

    function closeContextMenu() {
        if (!menuButton.isMenuOpened) {
            return
        }

        menuButton.toggleMenu(null)
    }

    signal toggleExpandRequested()
    signal enableEditingToggled(bool val)
    signal hideSelectedElementsRequested()
    signal insertNewPaletteRequested()
    signal hidePaletteRequested()
    signal editPalettePropertiesRequested()

    implicitHeight: paletteExpandArrow.height
    implicitWidth: paletteExpandArrow.implicitWidth + textItem.implicitWidth + menuButton.implicitWidth + 8 // 8 for margins

    FlatButton {
        id: paletteExpandArrow
        z: 1000
        width: height
        visible: !root.unresolved // TODO: make a separate palette placeholder component
        activeFocusOnTab: false // same focus object as parent palette
        icon: root.expanded ? IconCode.SMALL_ARROW_DOWN : IconCode.SMALL_ARROW_RIGHT
        transparent: true

        enabled: paletteExpandArrow.visible
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow
        navigation.column: 1
        navigation.accessible.name: root.expanded
                                    //: Collapse a tree item
                                    ? qsTrc("global", "Collapse")
                                    //: Expand a tree item
                                    : qsTrc("global", "Expand")

        onClicked: root.toggleExpandRequested()
    }

    StyledTextLabel {
        id: textItem
        height: parent.height
        horizontalAlignment: Text.AlignHLeft
        anchors {
            left: paletteExpandArrow.right; leftMargin: 4;
            right: deleteButton.visible ? deleteButton.left : (menuButton.visible ? menuButton.left : parent.right)
        }

        text: root.text
        font: ui.theme.bodyBoldFont
    }

    FlatButton {
        id: deleteButton

        anchors.right: menuButton.left
        anchors.rightMargin: 6

        height: parent.height
        width: height

        z: 1000

        icon: IconCode.DELETE_TANK
        toolTipTitle: deleteButton.text
        visible: root.hidePaletteElementVisible && root.editingEnabled
        activeFocusOnTab: mainPalette.currentItem === paletteTree.currentTreeItem
        transparent: true

        enabled: deleteButton.visible
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow
        navigation.column: 2

        onClicked: {
            hideSelectedElementsRequested()
        }
    }

    MouseArea {
        id: rightClickArea

        anchors.fill: parent
        acceptedButtons: Qt.RightButton

        onClicked: {
            contextMenuLoader.show(Qt.point(mouseX, mouseY))
        }

        ContextMenuLoader {
            id: contextMenuLoader

            items: menuButton.menuModel

            onHandleMenuItem: function(itemId) {
                menuButton.menuItemClicked(itemId)
            }
        }
    }

    MenuButton {
        id: menuButton

        anchors.right: parent.right

        z: 1000

        height: 20
        width: height

        anchors.verticalCenter: parent.verticalCenter

        visible: (root.expanded || root.hovered || isMenuOpened || navigation.active) && root.isInVisibleArea

        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow
        navigation.column: 3

        menuModel: [
            {id: "hide", title: root.custom ? qsTrc("palette", "Hide/Delete palette") : qsTrc("palette", "Hide palette") },
            {id: "new", title: qsTrc("palette", "Insert new palette") },
            {id: "", title: "" }, // separator
            {id: "edit", title: qsTrc("palette", "Enable editing"), checkable: true, checked: root.editingEnabled },
            {id: "", title: "" }, // separator
            {id: "reset", title: qsTrc("palette", "Reset palette") },
            {id: "save", title: qsTrc("palette", "Save palette…") },
            {id: "load", title: qsTrc("palette", "Load palette…") },
            {id: "", title: "" }, // separator
            {id: "properties", title: qsTrc("palette", "Palette properties…") },
        ]

        onHandleMenuItem: function(itemId) {
            menuItemClicked(itemId)
        }

        function menuItemClicked(itemId) {
            switch(itemId) {
            case "hide": root.hidePaletteRequested(); break
            case "new": root.insertNewPaletteRequested(); break
            case "edit": root.enableEditingToggled(!root.editingEnabled); break
            case "reset": root.paletteProvider.resetPalette(root.modelIndex); break
            case "save": root.paletteProvider.savePalette(root.modelIndex); break
            case "load": root.paletteProvider.loadPalette(root.modelIndex); break
            case "properties": Qt.callLater(root.editPalettePropertiesRequested); break
            }
        }
    }
}
