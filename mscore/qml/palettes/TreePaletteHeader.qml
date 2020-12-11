//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

import QtQuick 2.8
import QtQuick.Controls 2.1

import MuseScore.Palette 3.3
import MuseScore.Utils 3.3

Item {
    id: paletteHeader

    property bool expanded: false
    property bool hovered: false
    property string text: ""
    property bool hidePaletteElementVisible
    property bool editingEnabled: true
    property bool custom: false
    property bool unresolved: false

    property PaletteWorkspace paletteWorkspace
    property var modelIndex: null

    signal toggleExpandRequested()
    signal enableEditingToggled(bool val)
    signal hideSelectedElementsRequested()
    signal insertNewPaletteRequested()
    signal hidePaletteRequested()
    signal editPalettePropertiesRequested()

    implicitHeight: paletteExpandArrow.height
    implicitWidth: paletteExpandArrow.implicitWidth + textItem.implicitWidth + paletteHeaderMenuButton.implicitWidth + 8 // 8 for margins

    function showPaletteMenu() {
        paletteHeaderMenu.x = paletteHeaderMenuButton.x + paletteHeaderMenuButton.width - paletteHeaderMenu.width;
        paletteHeaderMenu.y = paletteHeaderMenuButton.y;
        paletteHeaderMenu.open();
    }

    StyledToolButton {
        id: paletteExpandArrow
        z: 1000
        width: height
        visible: !paletteHeader.unresolved // TODO: make a separate palette placeholder component
        activeFocusOnTab: false // same focus object as parent palette
        text: paletteHeader.expanded ? qsTr("Collapse") : qsTr("Expand")

        padding: 4

        contentItem: StyledIcon {
            source: paletteHeader.expanded ? "icons/ArrowDown.svg" : "icons/ArrowRight.svg"
        }

        onClicked: paletteHeader.toggleExpandRequested()
    }
    Text {
        id: textItem
        height: parent.height
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHLeft
        anchors {
            left: paletteExpandArrow.right; leftMargin: 4;
            right: deleteButton.visible ? deleteButton.left : (paletteHeaderMenuButton.visible ? paletteHeaderMenuButton.left : parent.right)
        }
        text: paletteHeader.text
        font: globalStyle.font
        color: globalStyle.text
        elide: Text.ElideRight
    }
//     StyledToolButton {
//         z: 1000
//         height: parent.height
//         anchors { left: paletteExpandArrow.right }
//         text: paletteHeader.text
//     }

    StyledToolButton {
        id: deleteButton
        z: 1000
        height: parent.height
        width: height
        anchors.right: paletteHeaderMenuButton.left
//         icon.name: "delete" // can't use icon until Qt 5.10... https://doc.qt.io/qt-5/qtquickcontrols2-icons.html
//         icon.source: "icons/delete.png"
        text: qsTr("Remove element")
        visible: paletteHeader.hidePaletteElementVisible && paletteHeader.editingEnabled
        activeFocusOnTab: mainPalette.currentItem === paletteTree.currentTreeItem

        KeyNavigation.backtab: mainPalette.currentItem
        KeyNavigation.tab: focusBreaker

        onHoveredChanged: {
            if (hovered) {
                mscore.tooltip.item = deleteButton;
                mscore.tooltip.text = deleteButton.text;
            } else if (mscore.tooltip.item == deleteButton)
                mscore.tooltip.item = null;
        }

        padding: 4

        contentItem: StyledIcon {
            source: "icons/TrashCan.svg"
        }

        onClicked: hideSelectedElementsRequested()
    }

    StyledToolButton {
        id: paletteHeaderMenuButton
        z: 1000
        height: parent.height
        anchors.right: parent.right

        visible: paletteHeader.expanded || paletteHeader.hovered || paletteHeaderMenu.visible

        activeFocusOnTab: parent.parent.parent === paletteTree.currentTreeItem

        padding: 4

        contentItem: StyledIcon {
            source: "icons/ThreeDotMenu.svg"
        }

        text: qsTr("Palette menu") // used by screen readers (they ignore Accessible.name for buttons)

        onClicked: showPaletteMenu()
    }

    MouseArea {
        id: rightClickArea
        anchors.fill: parent
        acceptedButtons: Qt.RightButton

        onClicked: {
            if (paletteHeaderMenu.popup) // Menu.popup() is available since Qt 5.10 only
                paletteHeaderMenu.popup();
            else {
                paletteHeaderMenu.x = mouseX;
                paletteHeaderMenu.y = mouseY;
                paletteHeaderMenu.open();
            }
        }
    }

    Menu {
        id: paletteHeaderMenu
        MenuItem {
            text: custom ? qsTr("Hide/Delete Palette") : qsTr("Hide Palette")
            onTriggered: paletteHeader.hidePaletteRequested()
        }
        MenuItem {
            text: qsTr("Insert New Palette")
            onTriggered: paletteHeader.insertNewPaletteRequested()
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Enable Editing")
            checkable: true
            checked: paletteHeader.editingEnabled
            onTriggered: paletteHeader.enableEditingToggled(checked)
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Reset Palette")
            onTriggered: paletteHeader.paletteWorkspace.resetPalette(paletteHeader.modelIndex)
        }
        MenuItem {
            text: qsTr("Save Palette…")
            onTriggered: paletteHeader.paletteWorkspace.savePalette(paletteHeader.modelIndex)
        }
        MenuItem {
            text: qsTr("Load Palette…")
            onTriggered: paletteHeader.paletteWorkspace.loadPalette(paletteHeader.modelIndex)
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Palette Properties…")
            enabled: paletteHeader.editingEnabled
            onTriggered: paletteHeader.editPalettePropertiesRequested()
        }
    }
}
