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
import QtQuick 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

FlatButton {
    id: root

    property bool isOpened: popup.isOpened

    property alias popupNavigationPanel: popup.navigationPanel

    property Component popupContent

    property int popupAvailableWidth: parent ? parent.width : 0
    property var anchorItem: null

    signal ensureContentVisibleRequested(int invisibleContentHeight)
    signal popupOpened(var popup, var control)

    Layout.fillWidth: true
    Layout.minimumWidth: (popupAvailableWidth - 4) / 2

    function closePopup() {
        popup.close()
    }

    QtObject {
        id: prv

        property bool needActiveFirstItem: false
    }

    Component.onCompleted: {
        popupController.load()
    }

    onClicked: {
        prv.needActiveFirstItem = root.navigation.highlight
        contentLoader.active = !contentLoader.active
        popup.toggleOpened()
    }

    StyledPopupView {
        id: popup

        anchorItem: root.anchorItem

        contentWidth: contentLoader.width
        contentHeight: contentLoader.height

        closePolicies: PopupView.NoAutoClose

        contentData: Loader {
            id: contentLoader

            active: false

            width: root.popupAvailableWidth - 2 * popup.margins
            height: implicitHeight

            sourceComponent: root.popupContent
        }

        onContentHeightChanged: {
            checkForInsufficientSpace()
        }

        onOpened: {
            Qt.callLater(checkForInsufficientSpace)

            if (prv.needActiveFirstItem) {
                forceFocusIn()
            }

            root.popupOpened(popup, root)
        }

        onClosed: {
            contentLoader.active = false

            root.ensureContentVisibleRequested(root.anchorItem.height) // reset contentY
        }

        function forceFocusIn() {
            if (Boolean(contentLoader.item) && Boolean(contentLoader.item.forceFocusIn)) {
                contentLoader.item.forceFocusIn()
            }
        }

        function checkForInsufficientSpace() {
            if (!isOpened) {
                return
            }

            var buttonGlobalPos = root.mapToItem(root.anchorItem, Qt.point(0, 0))
            var popupHeight = contentHeight + padding*2 + margins*2
            var invisibleContentHeight = root.anchorItem.height - (buttonGlobalPos.y + root.height + popupHeight)

            root.ensureContentVisibleRequested(invisibleContentHeight)
        }

        property NavigationPanel navigationPanel: NavigationPanel {
            name: root.navigation.name + " Popup"
            section: popup.navigationSection
            order: 1
            direction: NavigationPanel.Vertical
        }
    }
}
