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
import QtQuick.Window 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

StyledPopupView {
    id: root

    property alias model: view.model

    property int preferredAlign: Qt.AlignRight // Left, HCenter, Right

    signal handleMenuItem(string itemId)

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

    contentWidth: menuMetrics.itemWidth

    padding: 8
    margins: 0
    showArrow: false

    animationEnabled: false //! NOTE disabled - because trouble with simultaneous opening of submenu

    isCloseByEscape: false

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "StyledMenu"
        section: root.navigationSection
        direction: NavigationPanel.Vertical
        order: 1
    }

    navigationSection.onNavigationEvent: function(event) {
        if (event.type === NavigationEvent.Escape) {
            if (prv.showedSubMenu) {
                prv.showedSubMenu.close()
            } else {
                root.close()
            }
        }
    }

    signal loaded()

    function requestFocus() {
        var focused = prv.focusOnSelected()
        if (!focused) {
            focused = prv.focusOnFirstEnabled()
        }

        return focused
    }

    onModelChanged: {
        menuMetrics.calculate(model)

        //! NOTE: Due to the fact that the view has a dynamic delegate,
        //  the height calculation occurs with an error
        //  (by default, the delegate height is taken as the menu item height).
        //  Let's manually adjust the height of the content
        var sepCount = 0
        for (let i = 0; i < model.length; i++) {
            let item = Boolean(model.get) ? model.get(i).itemRole : model[i]
            if (!Boolean(item.title)) {
                sepCount++
            }
        }

        var itemHeight = 0
        for(var child in view.contentItem.children) {
            itemHeight = Math.max(itemHeight, view.contentItem.children[child].height)
        }

        var itemsCount = model.length - sepCount

        var anchorItemHeight = Boolean(root.anchorItem) ? root.anchorItem.height : (Screen.desktopAvailableHeight - padding * 2)

        root.contentHeight = Math.min(itemHeight * itemsCount + sepCount * prv.separatorHeight +
                                      prv.viewVerticalMargin * 2, anchorItemHeight)

        root.loaded()
    }

    MenuMetrics {
        id: menuMetrics
    }

    QtObject {
        id: prv

        property var showedSubMenu: null

        readonly property int separatorHeight: 1
        readonly property int viewVerticalMargin: 4

        function focusOnFirstEnabled() {
            for (var i = 0; i < view.count; ++i) {
                var loader = view.itemAtIndex(i)
                if (loader && !loader.isSeparator && loader.item && loader.item.enabled) {
                    loader.item.navigation.requestActive()
                    return true
                }
            }

            return false
        }

        function focusOnSelected() {
            for (var i = 0; i < view.count; ++i) {
                var loader = view.itemAtIndex(i)
                if (loader && !loader.isSeparator && loader.item && loader.item.isSelected) {
                    loader.item.navigation.requestActive()
                    return true
                }
            }
            return false
        }
    }

    StyledListView {
        id: view

        anchors.fill: parent
        anchors.topMargin: prv.viewVerticalMargin
        anchors.bottomMargin: prv.viewVerticalMargin

        spacing: 0
        interactive: contentHeight > root.height
        arrowControlsAvailable: true

        function itemByKey(key) {
            for (var i = 0; i < view.count; ++i) {
                var loader = view.itemAtIndex(i)

                if (!Boolean(loader) || loader.isSeparator) {
                    continue
                }

                var title = loader.item.title
                if (Boolean(title)) {
                    title = title.toLowerCase()
                    var index = title.indexOf('&')
                    if (index === -1) {
                        continue
                    }

                    var activateKey = title[index + 1]
                    if (activateKey === key) {
                        return loader.item
                    }
                }
            }

            return null
        }

        delegate: Loader {
            id: loader

            property var itemData: Boolean(root.model.get) ? model.itemRole : modelData
            property bool isSeparator: !Boolean(itemData) || !Boolean(itemData.title) || itemData.title === ""

            sourceComponent: isSeparator ? separatorComp : menuItemComp

            onLoaded: {
                loader.item.modelData = Qt.binding(() => (itemData))
                loader.item.width = Qt.binding(() => (menuMetrics.itemWidth))
            }

            Component {
                id: menuItemComp

                StyledMenuItem {
                    id: item

                    property string title: Boolean (loader.itemData) ? loader.itemData.title : ""

                    menuAnchorItem: root.anchorItem
                    parentWindow: root.window

                    navigation.panel: root.navigationPanel
                    navigation.row: model.index

                    iconAndCheckMarkMode: menuMetrics.iconAndCheckMarkMode

                    reserveSpaceForShortcutsOrSubmenuIndicator:
                        menuMetrics.hasItemsWithShortcut || menuMetrics.hasItemsWithSubmenu

                    padding: root.padding

                    Keys.onShortcutOverride: function(event) {
                        var activatedItem = view.itemByKey(event.text)
                        event.accepted = Boolean(activatedItem)
                    }

                    Keys.onPressed: function(event) {
                        var activatedItem = view.itemByKey(event.text)
                        if (Boolean(activatedItem)) {
                            activatedItem.navigation.requestActive()
                            activatedItem.navigation.triggered()
                        }
                    }

                    onOpenSubMenuRequested: function(menu) {
                        if (prv.showedSubMenu){
                            if (prv.showedSubMenu === menu) {
                                return
                            } else {
                                prv.showedSubMenu.close()
                            }
                        }

                        menu.toggleOpened()
                    }

                    onSubMenuShowed: function(menu) {
                        root.closePolicy = PopupView.NoAutoClose
                        prv.showedSubMenu = menu
                    }

                    onSubMenuClosed: {
                        root.closePolicy = PopupView.CloseOnPressOutsideParent
                        prv.showedSubMenu = null

                        if (!root.activeFocus) {
                            root.forceActiveFocus()
                        }

                        if (!item.activeFocus) {
                            item.forceActiveFocus()
                        }
                    }

                    onHandleMenuItem: function(itemId) {
                        // NOTE: reset view state
                        view.update()

                        root.handleMenuItem(itemId)
                    }

                    onRequestParentItemActive: {
                        if (root.navigationParentControl) {
                            root.navigationParentControl.requestActive()
                        }

                        root.close()
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
