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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

FlatButton {
    id: root

    property var currentViewMode
    property var availableViewModeList: []

    signal changeCurrentViewModeRequested(var newViewMode)

    transparent: !menu.isMenuOpened
    accentButton: menu.isMenuOpened

    buttonType: FlatButton.Horizontal
    isNarrow: true
    margins: 6

    visible: Boolean(root.currentViewMode)

    text: Boolean(root.currentViewMode) ? root.currentViewMode.title : ""
    icon: Boolean(root.currentViewMode) ? root.currentViewMode.icon : IconCode.NONE

    orientation: Qt.Horizontal

    contentItem: Row {
        spacing: 4

        StyledIconLabel {
            iconCode: root.icon
            font: ui.theme.toolbarIconsFont
        }

        StyledTextLabel {
            anchors.verticalCenter: parent.verticalCenter

            text: root.text
        }

        StyledIconLabel {
            anchors.verticalCenter: parent.verticalCenter

            iconCode: IconCode.SMALL_ARROW_DOWN
        }
    }

    onClicked: {
        menu.toggleOpened(root.availableViewModeList)
    }

    StyledMenuLoader {
        id: menu

        menuAnchorItem: ui.rootItem

        onHandleMenuItem: function(itemId) {
            Qt.callLater(root.changeCurrentViewModeRequested, itemId)
        }
    }
}
