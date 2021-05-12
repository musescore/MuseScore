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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

ListItemBlank {

    id: root

    property var modelData

    property int iconAndCheckMarkMode: StyledMenuItem.ShowOne
    property bool reserveSpaceForShortcutOrSubmenuIndicator: prv.hasShortcut || prv.hasSubMenu

    signal handleAction(string actionCode, int actionIndex)

    signal subMenuShowed()
    signal subMenuClosed()

    signal requestParentItemActive()

    hoveredStateColor: ui.theme.accentColor
    pressedStateColor: ui.theme.accentColor
    enabled: (Boolean(modelData) && modelData.enabled !== undefined ) ? Boolean(modelData.enabled) : true // default true

    isSelected: Boolean(prv.showedSubMenu) || (prv.hasIcon && prv.isSelectable && prv.isSelected)


    enum IconAndCheckMarkMode {
        None,
        ShowOne,
        ShowBoth
    }

    navigation.onActiveChanged: {
        if (prv.hasSubMenu) {
            if (navigation.active) {
                prv.showSubMenu()
            } else {
                Qt.callLater(function() {
                    if (prv.showedSubMenu && !prv.showedSubMenu.navigation.active) {
                        prv.closeSubMenu()
                    }
                })
            }
        }
    }

    navigation.onNavigationEvent: {
        switch (event.type) {
        case NavigationEvent.Right:
            //! NOTE Go to submenu if shown
            if (prv.showedSubMenu) {
                event.accepted = true
                prv.showedSubMenu.focusOnFirstItem()
            }
            break;
        case NavigationEvent.Left:
            //! NOTE Go to parent item
            root.requestParentItemActive()
        }
    }

    navigation.onTriggered: root.clicked()

    QtObject {
        id: prv

        property bool hasShortcut: Boolean(modelData) && Boolean(modelData.shortcut)

        property bool hasSubMenu: Boolean(modelData) && Boolean(modelData.subitems) && modelData.subitems.length > 0
        property var showedSubMenu: null

        property bool isCheckable: Boolean(modelData) && Boolean(modelData.checkable)
        property bool isChecked: isCheckable && Boolean(modelData.checked)

        property bool isSelectable: Boolean(modelData) && Boolean(modelData.selectable)
        property bool isSelected: isSelectable && Boolean(modelData.selected)

        property bool hasIcon: Boolean(modelData) && Boolean(modelData.icon)

        function showSubMenu() {
            if (prv.showedSubMenu) {
                return
            }

            var menuComponent = Qt.createComponent("StyledMenu.qml");
            var menu = menuComponent.createObject(root)
            menu.x = root.width
            menu.y = 0

            menu.navigationParentControl = root.navigation
            menu.navigation.name = root.navigation.name+"SubMenu"

            menu.model = modelData.subitems

            menu.handleAction.connect(function(actionCode, actionIndex) {
                Qt.callLater(root.handleAction, actionCode, actionIndex)
                menu.close()
            })

            menu.closed.connect(function() {
                prv.showedSubMenu = null
                menu.destroy()
                subMenuClosed()
            })

            subMenuShowed()

            prv.showedSubMenu = menu
            menu.toggleOpened()
        }

        function closeSubMenu() {
            if (prv.showedSubMenu) {
                prv.showedSubMenu.isDoActiveParentOnClose = false
                prv.showedSubMenu.close()
            }
        }
    }

    RowLayout {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: root.iconAndCheckMarkMode === StyledMenuItem.None ? 24 : 12
        anchors.right: parent.right
        anchors.rightMargin: 18

        spacing: 12

        StyledIconLabel {
            Layout.alignment: Qt.AlignLeft
            width: 16
            iconCode: {
                if (root.iconAndCheckMarkMode !== StyledMenuItem.ShowBoth && prv.hasIcon) {
                    return prv.hasIcon ? modelData.icon : IconCode.NONE
                } else if (prv.isCheckable) {
                    return prv.isChecked ? IconCode.TICK_RIGHT_ANGLE : IconCode.NONE
                } else  if (prv.isSelectable) {
                    return prv.isSelected ? IconCode.TICK_RIGHT_ANGLE : IconCode.NONE
                }

                return IconCode.NONE
            }
            visible: !isEmpty || root.iconAndCheckMarkMode !== StyledMenuItem.None
        }

        StyledIconLabel {
            Layout.alignment: Qt.AlignLeft
            width: 16
            iconCode: prv.hasIcon ? modelData.icon : IconCode.NONE
            visible: root.iconAndCheckMarkMode === StyledMenuItem.ShowBoth
        }

        StyledTextLabel {
            Layout.fillWidth: true
            text: Boolean(modelData) && Boolean(modelData.title) ? modelData.title : ""
            horizontalAlignment: Text.AlignLeft
        }

        StyledTextLabel {
            id: shortcutLabel
            Layout.alignment: Qt.AlignRight
            text: prv.hasShortcut ? modelData.shortcut : ""
            horizontalAlignment: Text.AlignRight
            visible: !isEmpty || (root.reserveSpaceForShortcutOrSubmenuIndicator)
        }

        StyledIconLabel {
            id: submenuIndicator
            Layout.alignment: Qt.AlignRight
            width: 16
            iconCode: prv.hasSubMenu ? IconCode.SMALL_ARROW_RIGHT : IconCode.NONE
            visible: !isEmpty || (root.reserveSpaceForShortcutOrSubmenuIndicator && !shortcutLabel.visible)
        }
    }

    onHovered: {
        if (isHovered) {
            root.navigation.ensureActive()
        }

        if (!prv.hasSubMenu) {
            return
        }

        if (isHovered) {
            prv.showSubMenu()
        } else {
            var mouseGlogalPos = mapToGlobal(Qt.point(mouseX, mouseY))
            var showedSubMenuGlobalPos = prv.showedSubMenu.contentItem.mapToGlobal(0, 0)

            var eps = 8
            var subMenuWidth = prv.showedSubMenu.width
            var subMenuHeight = prv.showedSubMenu.height

            var isHoveredOnShowedSubMenu = (showedSubMenuGlobalPos.x < mouseGlogalPos.x + eps && mouseGlogalPos.x - eps < showedSubMenuGlobalPos.x + subMenuWidth)
                    && (showedSubMenuGlobalPos.y < mouseGlogalPos.y + eps && mouseGlogalPos.y - eps < showedSubMenuGlobalPos.y + subMenuHeight)

            if (isHoveredOnShowedSubMenu) {
                return
            }

            prv.closeSubMenu()
        }
    }

    onClicked: {
        if (prv.hasSubMenu) {
            prv.showSubMenu()
            return
        }

        root.handleAction(modelData.code, prv.isSelectable ? index : -1)
    }
}
