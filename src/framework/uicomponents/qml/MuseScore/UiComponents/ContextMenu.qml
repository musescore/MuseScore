/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.9
import QtQuick.Controls 2.15

Menu {
    id: root

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnReleaseOutsideParent | Popup.CloseOnPressOutside

    implicitWidth: 220

    bottomPadding: 4
    topPadding: bottomPadding

    function clear() {
        for (var i = root.contentData.length - 1; i >= 0; --i) {
            root.removeItem(i)
        }
    }

    function addMenuItem(itemAction) {
        var obj = menuItem.createObject(root.parent, { action: itemAction })
        addItem(obj)
    }

    background: Rectangle {
        radius: 3

        border.width: 1
        border.color: ui.theme.strokeColor

        color: ui.theme.popupBackgroundColor
    }

    Component {
        id: menuItem

        StyledContextMenuItem {
            hintIcon: Boolean(action) ? action.icon.name : ""
        }
    }
}
