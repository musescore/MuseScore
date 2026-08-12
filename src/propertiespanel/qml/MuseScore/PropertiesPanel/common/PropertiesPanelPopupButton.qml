/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.PropertiesPanel

FlatButton {
    id: root

    readonly property bool isOpened: popupLoader.isOpened
    readonly property var popupNavigationPanel: popupLoader.popup ? popupLoader.popup.navigationPanel : null

    property Component popupContent
    property int popupAvailableWidth: parent ? parent.width : 0
    property var anchorItem: null

    signal ensureContentVisibleRequested(int invisibleContentHeight)

    Layout.fillWidth: true
    Layout.minimumWidth: (popupAvailableWidth - 4) / 2

    accentButton: root.isOpened

    function close() {
        popupLoader.close()
    }

    QtObject {
        id: prv
        property bool needActiveFirstItem: false
    }

    PropertiesPanelPopupControllerModel {
        id: popupController
    }

    onClicked: {
        prv.needActiveFirstItem = root.navigation.highlight
        popupLoader.toggleOpened()
    }

    StyledPopupLoader {
        id: popupLoader

        popupAnchorItem: root.anchorItem

        sourceComponent: StyledPopupView {
            id: popup

            contentWidth: contentLoader.width
            contentHeight: contentLoader.height

            placementPolicies: PopupView.PreferBelow | PopupView.IgnoreFit
            closePolicies: PopupView.NoAutoClose
            openPolicies: PopupView.Default | PopupView.OpenOnContentReady
            isContentReady: false

            contentData: Loader {
                id: contentLoader

                width: root.popupAvailableWidth - 2 * popup.margins
                height: implicitHeight

                sourceComponent: root.popupContent

                onStatusChanged: {
                    if (status == Loader.Ready) {
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

                popupController.setPopup(popup, root)
            }

            function forceFocusIn() {
                contentLoader.item?.forceFocusIn?.()
            }

            function checkForInsufficientSpace() {
                if (!root.anchorItem) {
                    return
                }

                var buttonGlobalPos = root.mapToItem(root.anchorItem, Qt.point(0, 0))
                var popupHeight = contentHeight + padding * 2 + margins * 2
                var buttonBottom = buttonGlobalPos.y + root.height
                var spaceBelow = root.anchorItem.height - buttonBottom
                if (spaceBelow > popupHeight) {
                    return
                }
                root.ensureContentVisibleRequested(spaceBelow - popupHeight)
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
}
