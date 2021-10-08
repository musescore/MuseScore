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

import MuseScore.Dock 1.0

DockPageView {
    id: root

    property alias central: central.sourceComponent

    centralDock: DockCentralView {
        objectName: root.objectName + "_central"

        Loader {
            id: central

            anchors.fill: parent
        }
    }

    toolBarsDockingHolders: [
        DockToolBarHolder {
            objectName: root.objectName + "_toolBarsDockingHolderLeft"
            location: DockBase.Left

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        },
        DockToolBarHolder {
            objectName: root.objectName + "_toolBarsDockingHolderRight"
            location: DockBase.Right

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        },
        DockToolBarHolder {
            objectName: root.objectName + "_toolBarsDockingHolderTop"
            location: DockBase.Top

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        },
        DockToolBarHolder {
            objectName: root.objectName + "_toolBarsDockingHolderBottom"
            location: DockBase.Bottom

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        }
    ]

    panelsDockingHolders: [
        DockPanelHolder {
            objectName: root.objectName + "_panelsDockingHolderLeft"
            location: DockBase.Left

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        },
        DockPanelHolder {
            objectName: root.objectName + "_panelsDockingHolderRight"
            location: DockBase.Right

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        },
        DockPanelHolder {
            objectName: root.objectName + "_panelsDockingHolderTop"
            location: DockBase.Top

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        },
        DockPanelHolder {
            objectName: root.objectName + "_panelsDockingHolderBottom"
            location: DockBase.Bottom

            Rectangle { color: ui.theme.backgroundPrimaryColor }
        }
    ]
}
