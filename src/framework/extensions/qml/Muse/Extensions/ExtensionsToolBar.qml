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

Item {

    id: root

    property alias isEmpty: extModel.isEmpty

    width: Math.max(32, row.childrenRect.width)
    height: 30

    Component.onCompleted: {
        extModel.init()
    }

    ExtensionsToolBarModel {
        id: extModel
    }

    Row {
        id: row
        anchors.fill: parent

        Repeater {
            model: extModel
            delegate: FlatButton {
                width: 30
                height: width
                anchors.verticalCenter: parent.verticalCenter

                enabled: enabledRole

                icon: iconRole
                iconFont: ui.theme.toolbarIconsFont
                transparent: true

                toolTipTitle: toolTipTitleRole

                onClicked: extModel.onClicked(model.index)
            }
        }
    }
}
