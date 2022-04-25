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

Loader {
    id: loader

    required property string textRole
    required property string valueRole

    required property int currentIndex
    required property int visibleItemsCount

    required property int itemWidth
    required property int itemHeight

    property alias dropdown: loader.item
    property alias isOpened: loader.active

    signal handleItem(int index, var value)
    signal opened()
    signal closed()

    QtObject {
        id: prv

        function load() {
            loader.active = true
        }

        function unload() {
            loader.active = false
            Qt.callLater(loader.closed)
        }
    }

    active: false

    sourceComponent: StyledDropdownView {
        id: item

        visibleItemsCount: loader.visibleItemsCount
        currentIndex: loader.currentIndex
        itemWidth: loader.itemWidth
        itemHeight: loader.itemHeight

        textRole: loader.textRole
        valueRole: loader.valueRole

        onHandleItem: function(index, value) {
            item.close()
            Qt.callLater(loader.handleItem, index, value)
        }

        onClosed: {
            Qt.callLater(prv.unload)
        }
    }

    function open(model) {
        prv.load()

        var dropdown = loader.dropdown
        dropdown.parent = loader.parent

        dropdown.model = model

        dropdown.open()

        Qt.callLater(loader.opened)
    }

    function toggleOpened(model) {
        prv.load()

        var dropdown = loader.dropdown
        if (dropdown.isOpened) {
            dropdown.close()
            return
        }

        open(model)
    }

    function close() {
        if (loader.isOpened) {
            loader.dropdown.close()
        }
    }
}
