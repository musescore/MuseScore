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
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

FlatButton {
    id: root

    property alias popup: popup
    property alias popupContent: popup.contentData

    property int popupAvailableWidth: 0
    property var anchorItem: null

    Layout.fillWidth: true
    Layout.minimumWidth: (popupAvailableWidth - 4) / 2

    signal ensureContentVisibleRequested(int invisibleContentHeight)
    signal popupOpened()

    onVisibleChanged: {
        if (!visible) {
            popup.close()
        }
    }

    onClicked: {
        if (!popup.isOpened) {
            popup.open()
        } else {
            popup.close()
        }
    }

    StyledPopupView {
        id: popup

        anchorItem: root.anchorItem

        navigationParentControl: root.navigation
        navigation.name: root.navigation.name + "Popup"
        navigation.direction: NavigationPanel.Both

        onContentHeightChanged: {
            calculateContentVisible()
        }

        onOpened: {
            calculateContentVisible()
            root.popupOpened()
        }

        onClosed: {
            root.ensureContentVisibleRequested(root.anchorItem.height) // reset contentY
        }

        function calculateContentVisible() {
            if (!isOpened) {
                return
            }

            var buttonGlobalPos = root.mapToItem(root.anchorItem, Qt.point(root.x, root.y))
            var popupHeight = contentHeight + padding*2 + margins*2
            var invisibleContentHeight = root.anchorItem.height - (buttonGlobalPos.y + root.height + popupHeight)

            root.ensureContentVisibleRequested(invisibleContentHeight)
        }
    }
}
