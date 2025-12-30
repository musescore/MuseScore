/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

Item {
    id: root

    property alias sectionDelegate: repeater.delegate
    required property var itemModel
    required property int index

    implicitWidth: repeater.itemAt(0)?.implicitWidth ?? 0
    implicitHeight: repeater.itemAt(0)?.implicitHeight ?? 0

    Repeater {
        id: repeater

        model: [
            { itemModel: root.itemModel, itemIndex: root.index }
        ]
    }
}
