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

import Muse.UiComponents

StyledTabBar {
    id: root
    width: parent.width
    spacing: 12

    background: Item {
        implicitHeight: 28
    }

    // Equally divided share given to overflowing tabs, or Infinity when everything fits:
    readonly property real truncatedItemWidth: {
        let items = contentChildren.filter(i => i.visible)
        if (items.length === 0) {
            return Infinity
        }

        let totalSpacing = (items.length - 1) * root.spacing
        let totalContent = items.reduce((s, i) => s + i.implicitWidth, 0)
        if (totalContent + totalSpacing <= root.width) {
            return Infinity
        }

        let share = (root.width - totalSpacing) / items.length
        let prev = -1
        while (share !== prev) {
            prev = share
            let fittedItems = items.filter(i => i.implicitWidth < share)
            let totalFittedWidth = fittedItems.reduce((s, i) => s + i.implicitWidth, 0)
            share = (root.width - totalFittedWidth - totalSpacing) / (items.length - fittedItems.length)
        }
        return share
    }

}
