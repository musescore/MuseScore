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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Item {

    id: root

    property var provider: ui.tooltip

    anchors.fill: parent

    QtObject {
        id: prv

        property var showedToolTip: null
    }

    Component {
        id: toolTipComp

        StyledToolTip {}
    }

    Connections {
        target: root.provider

        function onShowToolTip(parent, title, description, shortcut) {

            onHideToolTip()

            var toolTip = toolTipComp.createObject(parent)
            toolTip.title = title
            toolTip.description = description
            toolTip.shortcut = shortcut

            if (Boolean(parent.navigation)) {
                toolTip.navigationParentControl = parent.navigation
            }

            toolTip.closed.connect(function() {
                prv.showedToolTip.destroy()
                prv.showedToolTip = null
            })

            prv.showedToolTip = toolTip
            toolTip.toggleOpened()
        }

        function onHideToolTip() {
            if (prv.showedToolTip) {
                prv.showedToolTip.close()
            }
        }
    }
}
