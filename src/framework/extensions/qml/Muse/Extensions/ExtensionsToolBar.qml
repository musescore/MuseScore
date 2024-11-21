/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

import Muse.UiComponents 1.0
import Muse.Extensions 1.0

StyledToolBarView {

    id: root

    property alias isEmpty: extModel.isEmpty

    navigationPanel.name: "ExtensionsToolBar"
    navigationPanel.accessible.name: qsTrc("notation", "Extensions toolbar")

    spacing: 2

    ExtensionsToolBarModel {
        id: extModel
    }

    model: extModel
}
