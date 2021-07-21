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

StyledPopupView {
    id: root

    property alias model: view.model
    property int minimumMenuWidth: 178

    property int preferredAlign: Qt.AlignRight // Left, HCenter, Right

    signal handleAction(string actionCode, int actionIndex)

    x: {
        switch(preferredAlign) {
        case Qt.AlignLeft:
            return -contentWidth + padding
        case Qt.AlignHCenter:
            return -contentWidth / 2 + padding
        case Qt.AlignRight:
            return 0
        }

        return 0
    }

    y: parent.height

    contentWidth: prv.itemWidth

    padding: 8
    margins: 0
    showArrow: false

    animationEnabled: false //! NOTE disabled - because trouble with simultaneous opening of submenu

    navigation.name: "StyledMenu"
    navigation.direction: NavigationPanel.Vertical

    function focusOnFirstItem() {
        var loader = view.itemAtIndex(0)
        if (loader && loader.item) {
            loader.item.navigation.requestActive()
        }
    }

    function focusOnSelected() {
        for (var i = 0; i < view.count; ++i) {
            var loader = view.itemAtIndex(i)
            if (loader && loader.item && loader.item.isSelected) {
                loader.item.navigation.requestActive()
                return true
            }
        }
        return false
    }

    onModelChanged: {
        prv.hasItemsWithIconAndCheckable = false
        prv.hasItemsWithIconOrCheckable = false
        prv.hasItemsWithSubmenu = false
        prv.hasItemsWithShortcut = false

        //! NOTE Policy:
        //! - if the menu contains checkable items, space for the checkmarks is reserved
        //! - if the menu contains items with an icon, space for icons is reserved
        //! - selectable items that don't have an icon are treated as checkable
        //! - selectable items that do have an icon are treated as non-checkable
        //! - all selectable items that are selected get an accent color background

        for (let i = 0; i < model.length; i++) {
            let item = model[i]
            let hasIcon = (Boolean(item.icon) && item.icon !== IconCode.NONE)

            if (item.checkable && hasIcon) {
                prv.hasItemsWithIconAndCheckable = true
                prv.hasItemsWithIconOrCheckable = true
            } else if (item.checkable || hasIcon || item.selectable) {
                prv.hasItemsWithIconOrCheckable = true
            }

            if (Boolean(item.subitems) && item.subitems.length > 0) {
                prv.hasItemsWithSubmenu = true
            }

            if (Boolean(item.shortcut)) {
                prv.hasItemsWithShortcut = true
            }
        }

        let leftWidth = 0
        let rightWidth = 0

        for (let j = 0; j < model.length; j++) {
            prv.testItem.modelData = model[j]
            leftWidth = Math.max(leftWidth, prv.testItem.calculatedLeftPartWidth())
            rightWidth = Math.max(rightWidth, prv.testItem.calculatedRightPartWidth())
        }

        prv.itemLeftPartWidth = leftWidth
        prv.itemRightPartWidth = rightWidth

        //! NOTE: Due to the fact that the view has a dynamic delegate,
        //  the height calculation occurs with an error
        //  (by default, the delegate height is taken as the menu item height).
        //  Let's manually adjust the height of the content
        var sepCount = 0
        for (let k = 0; k < model.length; k++) {
            if (!Boolean(model[k].title)) {
                sepCount++
            }
        }

        var itemHeight = 0
        for(var child in view.contentItem.children) {
            itemHeight = Math.max(itemHeight, view.contentItem.children[child].height)
        }

        var itemsCount = model.length - sepCount

        root.contentHeight = itemHeight * itemsCount + sepCount * prv.separatorHeight +
                prv.viewVerticalMargin * 2
    }

    QtObject {
        id: prv

        property bool hasItemsWithIconAndCheckable: false
        property bool hasItemsWithIconOrCheckable: false
        property bool hasItemsWithSubmenu: false
        property bool hasItemsWithShortcut: false

        property int itemLeftPartWidth: 100
        property int itemRightPartWidth: 100
        readonly property int itemWidth:
            Math.max(itemLeftPartWidth + itemRightPartWidth, root.minimumMenuWidth)

        readonly property int separatorHeight: 1
        readonly property int viewVerticalMargin: 4

        readonly property int iconAndCheckMarkMode: {
            if (prv.hasItemsWithIconAndCheckable) {
                return StyledMenuItem.ShowBoth
            } else if (prv.hasItemsWithIconOrCheckable) {
                return StyledMenuItem.ShowOne
            }
            return StyledMenuItem.None
        }

        property StyledMenuItem testItem: StyledMenuItem {
            iconAndCheckMarkMode: prv.iconAndCheckMarkMode

            reserveSpaceForShortcutOrSubmenuIndicator:
                prv.hasItemsWithShortcut || prv.hasItemsWithSubmenu
        }
    }

    ListView {
        id: view

        anchors.fill: parent
        anchors.topMargin: prv.viewVerticalMargin
        anchors.bottomMargin: prv.viewVerticalMargin

        spacing: 0
        interactive: false

        delegate: Loader {
            id: loader

            property bool isSeparator: Boolean(modelData.title)

            sourceComponent: isSeparator ? menuItemComp : separatorComp

            onLoaded: {
                loader.item.modelData = Qt.binding(() => (modelData))
                loader.item.width = Qt.binding(() => (prv.itemWidth))
            }

            Component {
                id: menuItemComp

                StyledMenuItem {
                    id: item

                    navigation.panel: root.navigation
                    navigation.column: 0
                    navigation.row: model.index

                    iconAndCheckMarkMode: prv.iconAndCheckMarkMode

                    reserveSpaceForShortcutOrSubmenuIndicator:
                        prv.hasItemsWithShortcut || prv.hasItemsWithSubmenu

                    padding: root.padding

                    onSubMenuShowed: {
                        root.closePolicy = PopupView.NoAutoClose
                    }

                    onSubMenuClosed: {
                        root.closePolicy = PopupView.CloseOnPressOutsideParent
                    }

                    onHandleAction: {
                        // NOTE: reset view state
                        view.update()

                        root.handleAction(actionCode, actionIndex)
                    }

                    onRequestParentItemActive: {
                        root.navigationParentControl.requestActive()
                    }
                }
            }

            Component {
                id: separatorComp

                Rectangle {
                    height: prv.separatorHeight
                    color: ui.theme.strokeColor

                    property var modelData
                }
            }
        }
    }
}
