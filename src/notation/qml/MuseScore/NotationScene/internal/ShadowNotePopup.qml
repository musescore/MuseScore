/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
import MuseScore.NotationScene 1.0

StyledPopupView {
    id: root

    property NavigationSection notationViewNavigationSection: null
    property int navigationOrderStart: 0
    property int navigationOrderEnd: 0

    contentWidth: contentLoader.width
    contentHeight: contentLoader.height

    showArrow: false
    openPolicies: PopupView.NoActivateFocus
    takeFocusOnClick: false

    padding: 0 // The popup will "steal" mouse events if the padding overlaps with the shadow note area
    margins: 3

    signal elementRectChanged(var elementRect)

    function updatePosition() {
        root.x = root.parent.width * 1.5
        root.y = (root.parent.height / 2) - (root.height / 2)
    }

    Loader {
        id: contentLoader

        function contentByType(type) {
            switch (type) {
            case ShadowNotePopupContent.PERCUSSION_CONTENT: return percussionContent
            }
            return null
        }

        ShadowNotePopupModel {
            id: shadowNotePopupModel

            onItemRectChanged: function(rect) {
                root.elementRectChanged(rect)
            }
        }

        Component.onCompleted: {
            shadowNotePopupModel.init()
            contentLoader.sourceComponent = contentLoader.contentByType(shadowNotePopupModel.currentPopupType)
        }

        Component {
            id: percussionContent
            PercussionNotePopupContent {
            }
        }
    }
}
