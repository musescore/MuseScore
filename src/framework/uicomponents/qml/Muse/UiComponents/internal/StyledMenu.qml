/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
import QtQuick.Controls
import QtQuick.Window

import Muse.Ui
import Muse.UiComponents

MenuView {
    id: root

    property alias model: filteredModel.rawModel

    property int preferredAlign: Qt.AlignRight // Left, HCenter, Right
    required property bool hasSiblingMenus

    signal handleMenuItem(string itemId)
    signal openPrevMenu()
    signal openNextMenu()

    property alias width: content.width
    property alias height: content.height

    property string accessibleName: ""

    signal loaded()

    function requestFocus() {
        if (root.isSearchable) {
            searchLoader.requestFocus()
            return
        }
        var focused = prv.focusOnSelected()
        if (!focused) {
            prv.focusOnFirstEnabled()
        }
    }

    property Component menuMetricsComponent: Component {
        MenuMetrics{}
    }

    function navigateWithSymbolRequested(symbol, requestingMenuModel) {
        // If this menu does not have the focus, pass on to the next open menu.
        if (!content.navigationSection.active) {
            if (root.isSubMenuOpen) {
                root.subMenuLoader.menu.navigateWithSymbolRequested(symbol, requestingMenuModel)
            }
            return
        }

        // Find the currently active item (if any). The search will start from the item
        // following it and will wrap from the beginning once the last item is reached.
        let startingIndex = 0
        for (let i = 0; i < listView.count; ++i) {
            let loader = listView.itemAtIndex(i)
            if (loader && !loader.isSeparator && loader.item && loader.item.navigation.active) {
                startingIndex = i + 1
                break
            }
        }

        // Find the first menu item that matches the given underlined symbol (letter).
        let firstMatchingIndex = -1
        let isSingleMatch = true
        for (let j = 0; j < listView.count; ++j) {
            let index = startingIndex + j
            if (index >= listView.count) {
                index -= listView.count
            }

            let item = Boolean(model.get) ? model.get(index).item : model[index]
            if (item && item.enabled && requestingMenuModel.menuItemMatchesSymbol(item, symbol)) {
                if (firstMatchingIndex === -1) {
                    firstMatchingIndex = index
                } else {
                    isSingleMatch = false
                    break
                }
            }
        }

        // Highlight the first matching menu item. If it is the only match, click it.
        // Otheriwise do nothing and give the user a chance to navigate to the other matches.
        if (firstMatchingIndex !== -1) {
            let loader = listView.itemAtIndex(firstMatchingIndex)
            if (loader) {
                if (root.isSubMenuOpen && root.subMenuLoader.parent !== loader.item) {
                    root.subMenuLoader.close()
                }

                loader.item.navigation.requestActive()

                if (isSingleMatch) {
                    Qt.callLater(loader.item.clicked, null)
                }
            }
        }
    }

    desiredHeight: {
        const anchorItemHeight = Boolean(root.anchorItem) ? root.anchorItem.height - padding * 2 : Screen.height
        const searchHeight = root.isSearchable ? searchLoader.height : 0
        return Math.min(listView.actualHeight + searchHeight, anchorItemHeight)
    }

    desiredWidth: root.menuMetrics?.itemWidth ?? 0

    onAboutToClose: function(closeEvent) {
        closeSubMenu()
    }

    function closeSubMenu() {
        if (root.isSubMenuOpen) {
            root.subMenuLoader.close()
        }
    }

    property var subMenuLoader: null
    readonly property bool isSubMenuOpen: Boolean(root.subMenuLoader) && root.subMenuLoader.isMenuOpened
    property MenuMetrics menuMetrics: null

    readonly property bool isPlacedAbove: root.popupPosition === PopupPosition.Top

    contentItem: PopupContent {
        id: content

        property alias cascadeAlign: root.cascadeAlign

        objectName: "Menu"

        contentWidth: root.contentWidth
        contentHeight: root.contentHeight

        padding: root.padding
        margins: 0

        showArrow: root.showArrow
        popupPosition: root.popupPosition
        isOpened: root.isOpened

        animationEnabled: false //! NOTE disabled - because trouble with simultaneous opening of submenu
        closeOnEscape: false

        navigationSection.onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.Escape) {
                if (root.isSubMenuOpen) {
                    root.subMenuLoader.close()
                } else {
                    root.close()
                }
            }
        }

        property NavigationPanel navigationPanel: NavigationPanel {
            name: "StyledMenu"
            section: content.navigationSection
            direction: NavigationPanel.Vertical
            order: 1

            accessible.name: root.accessibleName

            onNavigationEvent: function(event) {
                switch (event.type) {
                case NavigationEvent.Right:
                    var selectedItem = prv.selectedItem()
                    if (!Boolean(selectedItem) || !selectedItem.hasSubMenu) {
                       if (root.hasSiblingMenus)  {
                            root.close(true)
                            root.openNextMenu()
                       }
                        return
                    }

                    //! NOTE Go to submenu if shown
                    selectedItem.openSubMenuRequested(false)

                    event.accepted = true

                    break
                case NavigationEvent.Left:
                    if (root.isSubMenuOpen) {
                        root.subMenuLoader.close()
                        event.accepted = true
                        return
                    }

                    //! NOTE Go to parent item
                    if (root.navigationParentControl) {
                        root.navigationParentControl.requestActive()
                    }

                    root.close()

                    if(root.hasSiblingMenus) {
                        root.openPrevMenu()
                    }
                    break
                case NavigationEvent.Up:
                case NavigationEvent.Down:
                    if (root.isSubMenuOpen) {
                        root.subMenuLoader.close()
                    }

                    break
                }
            }
        }

        onCloseRequested: {
            root.close()
        }

        Component.onCompleted: {
            var menuLoaderComponent = Qt.createComponent("../StyledMenuLoader.qml");
            root.subMenuLoader = menuLoaderComponent.createObject(root.window)
            root.subMenuLoader.menuAnchorItem = root.anchorItem
            root.subMenuLoader.hasSiblingMenus = root.hasSiblingMenus

            root.subMenuLoader.handleMenuItem.connect(function(itemId) {
                root.handleMenuItem(itemId)
            })

            root.subMenuLoader.opened.connect(function(itemId) {
                root.closePolicies = PopupView.NoAutoClose
            })

            root.subMenuLoader.closed.connect(function(force) {
                root.closePolicies = PopupView.CloseOnPressOutsideParent

                if (force) {
                    root.close(true)
                    root.openNextMenu()
                }
            })
        }

        Loader {
            id: searchLoader

            active: root.isSearchable

            width: parent.width
            height: item ? item.implicitHeight : 0

            // If the window is placed above then the search goes underneath the list
            y: root.isPlacedAbove ? parent.height - searchLoader.height : 0

            signal requestFocus()

            sourceComponent: Column {
                width: parent.width

                bottomPadding: root.isPlacedAbove ? root.viewMargins : 0
                topPadding: root.isPlacedAbove ? 0 : root.viewMargins
                spacing: root.viewMargins

                SeparatorLine {
                    visible: root.isPlacedAbove
                    width: parent.width
                }

                SearchField {
                    id: searchField

                    navigation.panel: content.navigationPanel
                    navigation.row: 0

                    inputField.activeFocusOnPress: true

                    anchors {
                        left: parent.left
                        right: parent.right

                        leftMargin: root.viewMargins
                        rightMargin: root.viewMargins
                    }

                    onSearchTextChanged: {
                        if (root.isSubMenuOpen) {
                            // This is a failsafe - see onIsSubmenuOpenChanged...
                            return
                        }
                        root.isSearching = searchField.searchText !== ""
                        filteredModel.setFilterText(searchField.searchText)
                    }

                    property bool searchNeedsRefocus: false

                    Connections {
                        target: root

                        function onIsSubMenuOpenChanged() {
                            // The following prevents a crash when typing while a submenu is open
                            if (root.isSubMenuOpen) {
                                if (searchField.inputField.focus) {
                                    searchField.inputField.focus = false
                                    searchField.searchNeedsRefocus = true
                                }
                                return
                            }
                            if (searchField.searchNeedsRefocus) {
                                searchField.inputField.focus = true
                                searchField.searchNeedsRefocus = false
                            }
                        }
                    }

                    Connections {
                        target: searchLoader
                        function onRequestFocus() { searchField.navigation.requestActive() }
                    }
                }

                SeparatorLine {
                    visible: !root.isPlacedAbove
                    width: parent.width
                }
            }
        }

        StyledListView {
            id: listView

            // Slight hack: Due to the fact that this has a dynamic delegate, the height
            // calculation occurs with an error (by default, the delegate height is taken
            // as the item height). Let's manually calculate the height...
            readonly property int actualHeight: {
                const model = listView.model
                if (!Boolean(model)) {
                    return 0
                }

                var separatorCount = 0
                for (let i = 0; i < model.length; i++) {
                    let item = Boolean(model.get) ? model.get(i).item : model[i]
                    if (!Boolean(item.title)) {
                        separatorCount++
                    }
                }

                var itemHeight = 0
                for(var child in listView.contentItem.children) {
                    itemHeight = Math.max(itemHeight, listView.contentItem.children[child].height)
                }

                const totalItemsHeight = itemHeight * (model.length - separatorCount)
                const totalSeparatorsHeight = separatorCount * prv.separatorHeight
                const totalMarginsHeight = listView.anchors.topMargin + listView.anchors.bottomMargin

                return totalItemsHeight + totalSeparatorsHeight + totalMarginsHeight
            }

            FilteredFlyoutModel {
                id: filteredModel
                rawModel: root.model
            }

            width: parent.width
            model: filteredModel.filteredModel

            onModelChanged: {
                root.menuMetrics = root.menuMetricsComponent.createObject(root)
                root.menuMetrics.calculate(listView.model)
            }

            // It would be preferable to use a Column for this but, due to the height problems outlined
            // above, the automatic height calculations do not work as expected...
            anchors {
                top: {
                    if (!root.isSearchable) {
                        return parent.top
                    }
                    return root.isPlacedAbove ? parent.top : searchLoader.bottom
                }

                bottom: {
                    if (!root.isSearchable) {
                        return parent.bottom
                    }
                    return root.isPlacedAbove ? searchLoader.top : parent.bottom
                }

                topMargin: root.viewMargins
                bottomMargin: root.viewMargins
            }

            spacing: 0

            // TODO: If it's true that the dynamic delegate causes problems with height calculations, then
            // the following logic is also likely to be unreliable [C.M]
            interactive: contentHeight > root.height

            arrowControlsAvailable: !root.isSearching
            scrollBarPolicy: root.isSearching ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

            QtObject {
                id: prv

                readonly property int separatorHeight: 1

                function focusOnFirstEnabled() {
                    for (var i = 0; i < listView.count; ++i) {
                        var loader = listView.itemAtIndex(i)
                        if (loader && !loader.isSeparator && loader.item && loader.item.enabled) {
                            loader.item.navigation.requestActive()
                            return true
                        }
                    }

                    return false
                }

                function focusOnSelected() {
                    var item = selectedItem()
                    if (Boolean(item)) {
                        item.navigation.requestActive()
                        return true
                    }

                    return false
                }

                function selectedItem() {
                    for (var i = 0; i < listView.count; ++i) {
                        var loader = listView.itemAtIndex(i)
                        if (loader && !loader.isSeparator && loader.item && loader.item.isSelected) {
                            return loader.item
                        }
                    }

                    return null
                }
            }

            delegate: Loader {
                id: loader

                required property var model
                required property int index

                readonly property var modelData: Boolean(root.model.get) ? model.item : model.modelData
                readonly property bool isSeparator: !(modelData?.title)

                sourceComponent: isSeparator ? separatorComp : menuItemComp

                Component {
                    id: menuItemComp

                    StyledMenuItem {
                        id: item
                        width: root.menuMetrics?.itemWidth ?? 0

                        modelData: loader.modelData

                        menuAnchorItem: root.anchorItem
                        parentWindow: root.window

                        navigation.panel: content.navigationPanel
                        navigation.row: root.isSearchable ? loader.index + 1 : loader.index

                        navigation.onActiveChanged: {
                            if (item.navigation.active) {
                                listView.positionViewAtIndex(loader.index, ListView.Contain)
                            }
                        }

                        iconAndCheckMarkMode: root.menuMetrics?.iconAndCheckMarkMode || StyledMenuItem.None
                        wideIcon: root.menuMetrics?.hasItemsWithWideIcon || false

                        reserveSpaceForShortcutsOrSubmenuIndicator: Boolean(root.menuMetrics) ?
                                                                        (root.menuMetrics.hasItemsWithShortcut || root.menuMetrics.hasItemsWithSubmenu) : false

                        padding: root.padding

                        subMenuShowed: root.isSubMenuOpen && root.subMenuLoader.parent === item

                        onOpenSubMenuRequested: function(byHover) {
                            if (!hasSubMenu) {
                                if (byHover) {
                                    root.subMenuLoader.close()
                                }

                                return
                            }

                            if (!byHover) {
                                if (subMenuShowed) {
                                    root.subMenuLoader.close()
                                    return
                                }
                            }

                            root.subMenuLoader.parent = item
                            root.subMenuLoader.open(subMenuItems)
                        }

                        onCloseSubMenuRequested: {
                            root.subMenuLoader.close()
                        }

                        onHandleMenuItem: function(itemId) {
                            // NOTE: reset view state
                            listView.update()

                            root.handleMenuItem(itemId)
                        }
                    }
                }

                Component {
                    id: separatorComp

                    Rectangle {
                        width: root.menuMetrics?.itemWidth ?? 0
                        height: prv.separatorHeight
                        color: ui.theme.strokeColor
                    }
                }
            }
        }
    }
}
