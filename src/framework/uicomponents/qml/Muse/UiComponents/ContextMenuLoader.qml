/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

Item {
    id: container

    // Useful for static context menus
    property var items: []


    property NavigationSection notationViewNavigationSection: null
    property int navigationOrderStart: 0

    property alias closeMenuOnSelection: contextMenuLoader.closeMenuOnSelection
    property alias opensUpward: contextMenuLoader.opensUpward
    property alias focusOnOpened: contextMenuLoader.focusOnOpened
    property alias item: contextMenuLoader.item

    signal handleMenuItem(string itemId)
    signal opened()
    signal closed()

    //! NOTE: Height and width are equal to zero - the menu will appear exactly
    //  next(depending on the limitation) to the pressed position.
    width: 0
    height: 0

    onNotationViewNavigationSectionChanged: function() {
        contextMenuLoader.item.navigationSectionOverride = container.notationViewNavigationSection
    }
    onNavigationOrderStartChanged: function() {
        contextMenuLoader.item.navigationOrderStart = container.navigationOrderStart
    }

    function show(position: point, items) {
        if (!items) {
            items = container.items
        }

        container.x = position.x
        container.y = position.y

        var posOnContainer = parent.mapToItem(container, position)

        if (contextMenuLoader.isMenuOpened) {
            contextMenuLoader.update(items, posOnContainer.x, posOnContainer.y)
        } else {
            contextMenuLoader.open(items, posOnContainer.x, posOnContainer.y)
        }
    }

    function close() {
        contextMenuLoader.close()
    }

    StyledMenuLoader {
        id: contextMenuLoader

        onHandleMenuItem: function(itemId) {
            container.handleMenuItem(itemId)
        }

        onOpened: {
            container.opened()
        }

        onClosed: {
            container.closed()
        }
    }
}
