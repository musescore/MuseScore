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

import MuseScore.Dock 1.0 as Dock

Dock.DockPage {
    id: root

    property alias central: central.sourceComponent

    centralDock: Dock.DockCentral {
        objectName: root.objectName + "_central"

        Loader {
            id: central

            anchors.fill: parent
        }
    }

    toolBarsDockingHelpers: [
        Dock.DockToolBar {
            objectName: root.objectName + "_toolBarsDockingHelperLeft"
            visible: false

            width: 48
            height: root.height

            minimumWidth: 48
            minimumHeight: 200

            maximumWidth: 96
            maximumHeight: 100500

            location: Dock.DockBase.Left

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        },
        Dock.DockToolBar {
            objectName: root.objectName + "_toolBarsDockingHelperRight"
            visible: false

            width: 48
            height: root.height

            minimumWidth: 48
            minimumHeight: 200

            maximumWidth: 96
            maximumHeight: 100500

            location: Dock.DockBase.Right

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        },
        Dock.DockToolBar {
            objectName: root.objectName + "_toolBarsDockingHelperTop"
            visible: false

            width: root.width
            height: 48

            minimumWidth: 100
            minimumHeight: 48

            maximumWidth: 100500
            maximumHeight: 48

            location: Dock.DockBase.Top

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        },
        Dock.DockToolBar {
            objectName: root.objectName + "_toolBarsDockingHelperBottom"
            visible: false

            width: root.width
            height: 48

            minimumWidth: 100
            minimumHeight: 48

            maximumWidth: 100500
            maximumHeight: 48

            location: Dock.DockBase.Bottom

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        }
    ]
}
