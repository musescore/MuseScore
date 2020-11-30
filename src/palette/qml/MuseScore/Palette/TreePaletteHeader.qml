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

import MuseScore.Palette 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

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


    function toggleContextMenu() {
        if (paletteHeaderMenu.opened) {
            paletteHeaderMenu.close()
            return
        }

        paletteHeaderMenu.open()
    }

    FlatButton {
        id: paletteExpandArrow
        z: 1000
        width: height
        visible: !paletteHeader.unresolved // TODO: make a separate palette placeholder component
        activeFocusOnTab: false // same focus object as parent palette
        icon: paletteHeader.expanded ? IconCode.SMALL_ARROW_DOWN : IconCode.SMALL_ARROW_RIGHT
        normalStateColor: "transparent"

        onClicked: paletteHeader.toggleExpandRequested()
    }

    StyledTextLabel {
        id: textItem
        height: parent.height
        horizontalAlignment: Text.AlignHLeft
        anchors {
            left: paletteExpandArrow.right; leftMargin: 4;
            right: deleteButton.visible ? deleteButton.left : (paletteHeaderMenuButton.visible ? paletteHeaderMenuButton.left : parent.right)
        }

        text: paletteHeader.text
        font: ui.theme.bodyBoldFont
    }

    FlatButton {
        id: deleteButton
        z: 1000
        height: parent.height
        width: height
        anchors.right: paletteHeaderMenuButton.left
        anchors.rightMargin: 6
        icon: IconCode.DELETE_TANK
        visible: paletteHeader.hidePaletteElementVisible && paletteHeader.editingEnabled
        activeFocusOnTab: mainPalette.currentItem === paletteTree.currentTreeItem
        normalStateColor: "transparent"

        KeyNavigation.backtab: mainPalette.currentItem
        KeyNavigation.tab: focusBreaker

        onHoveredChanged: {
            if (hovered) {
                ui.tooltip.show(deleteButton, deleteButton.text)
            } else {
                ui.tooltip.hide(deleteButton)
            }
        }

        onClicked: hideSelectedElementsRequested()
    }

    FlatButton {
        id: paletteHeaderMenuButton
        z: 1000
        height: parent.height
        anchors.right: parent.right

        visible: paletteHeader.expanded || paletteHeader.hovered || paletteHeaderMenu.visible

        activeFocusOnTab: parent.parent.parent === paletteTree.currentTreeItem

        icon: IconCode.MENU_THREE_DOTS
        normalStateColor: "transparent"

        onClicked: {
            toggleContextMenu()
        }
    }

    MouseArea {
        id: rightClickArea
        anchors.fill: parent
        acceptedButtons: Qt.RightButton

        onClicked: {
            toggleContextMenu()
        }
    }

    ContextMenu {
        id: paletteHeaderMenu

        x: paletteHeaderMenuButton.x + paletteHeaderMenuButton.width
        y: paletteHeaderMenuButton.y + paletteHeaderMenuButton.height

        StyledMenuItem {
            text: custom ? qsTrc("palette", "Hide/Delete Palette") : qsTrc("palette", "Hide Palette")

            onTriggered: {
                paletteHeader.hidePaletteRequested()
            }
        }

        StyledMenuItem {
            text: qsTrc("palette", "Insert New Palette")

            onTriggered: {
                paletteHeader.insertNewPaletteRequested()
            }
        }

        SeparatorLine {}

        StyledMenuItem {
            text: qsTrc("palette", "Enable Editing")
            checkable: true
            checked: paletteHeader.editingEnabled

            onTriggered: {
                paletteHeader.enableEditingToggled(checked)
            }
        }

        SeparatorLine {}

        StyledMenuItem {
            text: qsTrc("palette", "Reset Palette")

            onTriggered: {
                paletteHeader.paletteWorkspace.resetPalette(paletteHeader.modelIndex)
            }
        }

        StyledMenuItem {
            text: qsTrc("palette", "Save Palette…")

            onTriggered: {
                paletteHeader.paletteWorkspace.savePalette(paletteHeader.modelIndex)
            }
        }

        StyledMenuItem {
            text: qsTrc("palette", "Load Palette…")

            onTriggered: {
                paletteHeader.paletteWorkspace.loadPalette(paletteHeader.modelIndex)
            }
        }

        SeparatorLine {}

        StyledMenuItem {
            text: qsTrc("palette", "Palette Properties…")
            enabled: paletteHeader.editingEnabled

            onTriggered: {
                Qt.callLater(paletteHeader.editPalettePropertiesRequested)
            }
        }
    }
}
