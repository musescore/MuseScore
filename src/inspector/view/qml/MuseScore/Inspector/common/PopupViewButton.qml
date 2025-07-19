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

        placementPolicies: PopupView.PreferBelow | PopupView.IgnoreFit

        closePolicies: PopupView.NoAutoClose

        openPolicies: PopupView.Default | PopupView.OpenOnContentReady
        isContentReady: false

        contentData: Loader {
            id: contentLoader

            active: false

            width: root.popupAvailableWidth - 2 * popup.margins
            height: implicitHeight

            sourceComponent: root.popupContent

            onStatusChanged: {
                if (contentLoader.status == Loader.Ready) {
                    Qt.callLater(popup.markContentIsReady)
                }
            }
        }

        onContentHeightChanged: {
            if (contentHeight > 0) {
                Qt.callLater(checkForInsufficientSpace)
            }
        }

        onOpened: {
            if (contentHeight > 0) {
                Qt.callLater(checkForInsufficientSpace)
            }

            if (prv.needActiveFirstItem) {
                forceFocusIn()
            }

            root.popupOpened(popup, root)
        }

        onClosed: {
            contentLoader.active = false
            popup.isContentReady = false
        }

        function forceFocusIn() {
            if (Boolean(contentLoader.item) && Boolean(contentLoader.item.forceFocusIn)) {
                contentLoader.item.forceFocusIn()
            }
        }

        function checkForInsufficientSpace() {
            var buttonGlobalPos = root.mapToItem(root.anchorItem, Qt.point(0, 0))
            var popupHeight = contentHeight + padding*2 + margins*2

            var buttonBottom = buttonGlobalPos.y + root.height
            var spaceBelow = root.anchorItem.height - buttonBottom
            if (spaceBelow > popupHeight) {
                return
            }

            var invisibleContentHeight = spaceBelow - popupHeight

            root.ensureContentVisibleRequested(invisibleContentHeight)
        }

        function markContentIsReady() {
            popup.isContentReady = true
        }

        property NavigationPanel navigationPanel: NavigationPanel {
            name: root.navigation.name + " Popup"
            section: popup.navigationSection
            order: 1
            direction: NavigationPanel.Vertical
        }
    }
}
