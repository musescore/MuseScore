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

import QtQuick 2.8
import QtQml.Models 2.2

QtObject {
    property DelegateModel delegateModel: null

    property var placeholder: null

    property bool active: placeholder !== null
    property int index: placeholder ? placeholder.index : -1

    function removePlaceholder() {
        if (placeholder) {
            const idx = placeholder.index;
            placeholder.group.remove(placeholder.index, 1);
            placeholder = null;
        }
    }

    function makePlaceholder(idx, data) {
        // ensure that no placeholder is present currently
        removePlaceholder();
        delegateModel.items.insert(idx, data);
        placeholder = {
            group: delegateModel.items,
            index: idx
        };
    }
}
