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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

DropdownView {
    id: root

    property var model: null
    required property int visibleItemsCount

    default property alias contentData: content.contentData

    property alias background: content.background

    property alias width: content.width
    property alias height: content.height

    property alias margins: content.margins

    property alias animationEnabled: content.animationEnabled

    property alias closeOnEscape: content.closeOnEscape
    property alias navigationSection: content.navigationSection

    required property int currentIndex
    required property string textRole
    required property string valueRole

    required property int itemWidth
    required property int itemHeight

    property int contentWidth: root.itemWidth
    property int contentHeight: content.contentBodyHeight

    property string typeAheadStr: ""
    property string lastTypedChar: ""
    property bool typeAheadSameChar: true
    property int currentNavIndex: root.currentIndex

    //! NOTE: Due to the fact that the dropdown window opens without activating focus,
    //!       for all items in the dropdown, the accessible window must be the window
    //!       of the element from which the dropdown was opened
    required property var accessibleWindow

    x: 0
    y: 0

    padding: 8
    margins: 0

    showArrow: false

    property Timer typeAheadTimer: Timer {
        interval: 1000
        repeat: false
        onTriggered: {
            typeAheadStr = ""
            lastTypedChar = ""
            typeAheadSameChar = true
        }
    }

    signal handleItem(int index, var value)

    onOpened: {
        content.navigationSection.requestActive()

        prv.positionViewAtIndex(root.currentIndex)
    }

    onClosed: {
        content.focus = false
        root.requestHighlight(false)
    }

    onModelChanged: {
        root.contentHeight = root.itemHeight * Math.min(root.model.length, root.visibleItemsCount)

        var viewHeight = root.itemHeight * Math.min(model.length, root.visibleItemsCount)

        var viewCenterY = viewHeight / 2
        if (root.currentIndex === -1) {
            root.focusItemY = viewCenterY
            return
        }

        var itemY = root.currentIndex * root.itemHeight

        if (itemY < viewCenterY) {
            root.focusItemY = itemY
            return
        }

        var newViewHeight = (model.length - Math.min(model.length, root.visibleItemsCount)) * root.itemHeight + viewHeight
        var newViewCenterY = newViewHeight - viewHeight / 2

        if (itemY < newViewCenterY) {
            root.focusItemY = viewCenterY
            return
        }

        if (itemY > newViewCenterY) {
            root.focusItemY = viewHeight - (model.length - root.currentIndex) * root.itemHeight
            return
        }

        root.focusItemY = viewCenterY
    }

    contentItem: PopupContent {
        id: content

        objectName: "Dropdown"

        contentWidth: root.contentWidth
        contentHeight: root.contentHeight

        padding: root.padding

        showArrow: root.showArrow
        arrowX: root.arrowX
        opensUpward: root.opensUpward
        isOpened: root.isOpened

        onCloseRequested: {
            root.close()
        }

        StyledListView {
            id: view

            anchors.fill: parent

            model: root.model

            property NavigationPanel navigationPanel: NavigationPanel {
                name: "Dropdown"
                section: root.navigationSection
                direction: NavigationPanel.Vertical
                order: 1
                accessible.window: root.accessibleWindow

                onNavigationEvent: function(event) {
                    if (event.type === NavigationEvent.AboutActive) {
                        var item = view.itemAtIndex(root.currentIndex)
                        var controlName = item.navigation.name
                        if (controlName !== "") {
                            event.setData("controlName", controlName)
                        }
                    }
                }
            }

            QtObject {
                id: prv

                function typeAheadFind(typedChar) {
                    if (typeAheadSameChar && typeAheadStr.length > 0 && typedChar !== lastTypedChar) {
                        typeAheadSameChar = false
                    }

                    typeAheadStr += typedChar
                    lastTypedChar = typedChar

                    focusNextMatchingItem(
                        typeAheadSameChar ? typedChar : typeAheadStr,
                        currentNavIndex + (typeAheadSameChar ? 1 : 0)
                    )
                }

                function focusNextMatchingItem(str, startIndex) {
                    root.typeAheadTimer.restart()
                    let nextMatchIndex = findNextMatchingIndex(str, startIndex)

                    if (nextMatchIndex !== -1) {
                        highlightItem(nextMatchIndex)
                        currentNavIndex = nextMatchIndex
                        return true
                    }
                    return false
                }

                function findNextMatchingIndex(str, startIndex) {
                    let modelLength = root.model.length;
                    if (startIndex < 0 || startIndex >= modelLength) {
                        startIndex = 0
                    }

                    str = normalizeForSearch(str);
                    for (let i = startIndex; i < modelLength; i++) {
                        if (matchItemText(i, str)) {
                            return i
                        }
                    }
                    for (let j = 0; j < startIndex; j++) {
                        if (matchItemText(j, str)) {
                            return j
                        }
                    }

                    return -1
                }

                function matchItemText(i, str) {
                    let itemText = normalizeForSearch(Utils.getItemValue(root.model, i, root.textRole).replace(/^\s+/, ""))
                    return itemText.startsWith(str)
                }

                function highlightItem(index) {
                    scrollToItem(index)
                    setItemFocus(index)
                    root.requestHighlight(true)
                }

                function scrollToItem(index) {
                    let itemHeight = root.itemHeight
                    let totalItems = root.model.length
                    let itemTop = index * itemHeight

                    let middleOffset = (view.height / 2);
                    let scrollPosition = itemTop - middleOffset;

                    let maxContentHeight = totalItems * itemHeight
                    view.contentY = Math.max(0, Math.min(scrollPosition, maxContentHeight - view.height))
                }

                function positionViewAtIndex(itemIndex) {
                    view.positionViewAtIndex(itemIndex, ListView.Contain)

                    correctPosition(itemIndex)
                }

                function correctPosition(itemIndex) {
                    let item = view.itemAtIndex(itemIndex)
                    if (Boolean(item)) {
                        let diff = item.mapToGlobal(0, 0).y - root.parent.mapToGlobal(0, 0).y

                        if (view.contentY + diff + view.height > view.contentHeight) {
                            view.positionViewAtEnd()
                        } else if (view.contentY + diff < 0) {
                            view.positionViewAtBeginning()
                        } else {
                            view.contentY += diff
                        }
                    }
                    setItemFocus(itemIndex)
                }

                function setItemFocus(index) {
                    let item = view.itemAtIndex(index)
                    if (Boolean(item)) {
                        item.navigation.requestActive()
                    }
                }

                function normalizeForSearch(str) {
                    return str.toLowerCase().normalize("NFKD")
                }
            }

            delegate: ListItemBlank {
                id: item

                objectName: "dropitem"

                height: root.itemHeight
                width: root.contentWidth

                normalColor: ui.theme.buttonColor
                radius: 0

                isSelected: model.index === root.currentIndex

                navigation.name: label.text
                navigation.panel: view.navigationPanel
                navigation.row: model.index
                navigation.accessible.name: label.text
                navigation.accessible.window: root.accessibleWindow
                navigation.onActiveChanged: {
                    if (navigation.highlight) {
                        view.positionViewAtIndex(model.index, ListView.Contain)
                    }
                }

                Keys.onShortcutOverride: function(event) {

                    let typedChar = event.text
                    event.accepted = false

                    if (typedChar === "" || typedChar.match(/[\x00-\x1F\x7F]/)) {
                        return // ignore shortcuts and text containing ASCII control characters
                    }

                    if (typedChar === " " && typeAheadStr.length === 0) {
                        return // ignore space if it's the first character typed
                    }

                    // User typed a printable character.
                    prv.typeAheadFind(typedChar)
                    event.accepted = true
                }

                StyledTextLabel {
                    id: label
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    horizontalAlignment: Text.AlignLeft

                    text: Utils.getItemValue(root.model, model.index, root.textRole, "")
                }

                onClicked: {
                    var value = Utils.getItemValue(root.model, model.index, root.valueRole, undefined)
                    root.handleItem(model.index, value)
                }

                mouseArea.onContainsMouseChanged: {
                    if (!label.truncated) {
                        return
                    }

                    if (mouseArea.containsMouse) {
                        ui.tooltip.show(item, label.text)
                    } else {
                        ui.tooltip.hide(item)
                    }
                }
            }
        }
    }
}
