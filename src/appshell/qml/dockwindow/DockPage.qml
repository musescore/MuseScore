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

    toolBarsDockingHolders: [
        Dock.DockToolBarHolder {
            objectName: root.objectName + "_toolBarsDockingHolderLeft"
            location: Dock.DockBase.Left

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        },
        Dock.DockToolBarHolder {
            objectName: root.objectName + "_toolBarsDockingHolderRight"
            location: Dock.DockBase.Right

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        },
        Dock.DockToolBarHolder {
            objectName: root.objectName + "_toolBarsDockingHolderTop"
            location: Dock.DockBase.Top

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        },
        Dock.DockToolBarHolder {
            objectName: root.objectName + "_toolBarsDockingHolderBottom"
            location: Dock.DockBase.Bottom

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        }
    ]
}
