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
    property int itemWidth: 300

    signal handleAction(string actionCode, int actionIndex)

    contentWidth: root.itemWidth
    contentHeight: view.childrenRect.height
    padding: 0
    margins: 0
    x: 0
    y: opensUpward ? -root.height : parent.height
    showArrow: false

    animationEnabled: false //! NOTE disabled - because trouble with simultaneous opening of submenu

    navigation.name: "StyledMenu"
    navigation.direction: NavigationPanel.Vertical

    function focusOnFirstItem() {
        var loader = view.itemAtIndex(0)
        if (loader && loader.item) {
            loader.item.navigation.ensureActive()
        }
    }

    function focusOnSelected() {
        for (var i = 0; i < view.count; ++i) {
            var loader = view.itemAtIndex(i)
            if (loader && loader.item && loader.item.isSelected) {
                loader.item.navigation.ensureActive()
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

        for (let i = 0; i < model.length; i++) {
            let modelData = model[i]
            let hasIcon = (Boolean(modelData.icon) && modelData.icon !== IconCode.NONE)

            if (modelData.checkable && hasIcon) {
                prv.hasItemsWithIconAndCheckable = true
                prv.hasItemsWithIconOrCheckable = true
            } else if (modelData.checkable || hasIcon) {
                prv.hasItemsWithIconOrCheckable = true
            }

            if (Boolean(modelData.subitems) && modelData.subitems.length > 0) {
                prv.hasItemsWithSubmenu = true
            }

            if (Boolean(modelData.shortcut)) {
                prv.hasItemsWithShortcut = true
            }
        }
    }

    QtObject {
        id: prv
        property bool hasItemsWithIconAndCheckable: false
        property bool hasItemsWithIconOrCheckable: false
        property bool hasItemsWithSubmenu: false
        property bool hasItemsWithShortcut: false
    }

    ListView {
        id: view
        anchors.fill: parent
        spacing: 2
        interactive: false

        delegate: Loader {
            id: loader

            sourceComponent: Boolean(modelData.title) ? menuItemComp : separatorComp

            onLoaded: {
                loader.item.modelData = modelData
                loader.item.width = root.itemWidth
            }

            Component {
                id: menuItemComp

                StyledMenuItem {
                    id: item

                    navigation.panel: root.navigation
                    navigation.column: 0
                    navigation.row: model.index

                    iconAndCheckMarkMode: {
                        if (prv.hasItemsWithIconAndCheckable) {
                            return StyledMenuItem.ShowBoth
                        } else if (prv.hasItemsWithIconOrCheckable) {
                            return StyledMenuItem.ShowOne
                        }
                        return StyledMenuItem.None
                    }

                    reserveSpaceForShortcutOrSubmenuIndicator:
                        prv.hasItemsWithSubmenu || prv.hasItemsWithShortcut

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
                        root.navigationParentControl.ensureActive()
                    }
                }
            }

            Component {
                id: separatorComp

                Rectangle {
                    height: 1
                    color: ui.theme.strokeColor

                    property var modelData
                }
            }
        }
    }
}
