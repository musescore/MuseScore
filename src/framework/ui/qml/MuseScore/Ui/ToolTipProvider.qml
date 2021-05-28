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

    Connections {
        target: root.provider

        function onShowToolTip(parent, title, description, shortcut) {
            var toolTipComponentPath = "../UiComponents/StyledToolTip.qml"
            var toolTipComponent = Qt.createComponent(toolTipComponentPath)
            if (toolTipComponent.status !== Component.Ready) {
                console.log("[qml] failed create component: " + toolTipComponentPath + ", err: " + toolTipComponent.errorString())
                return
            }

            var toolTip = toolTipComponent.createObject(parent)
            toolTip.title = title
            toolTip.description = description
            toolTip.shortcut = shortcut

            toolTip.closed.connect(function() {
                prv.showedToolTip = null
                toolTip.destroy()
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
