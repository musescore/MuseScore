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

    property var toolBarLeftDropDestination: { "dock": toolBarLeftDockingHolder, "dropLocation": DropLocation.Top }
    property var toolBarRightDropDestination: { "dock": toolBarRightDockingHolder, "dropLocation": DropLocation.Top }
    property var toolBarTopDropDestination: { "dock": toolBarTopDockingHolder, "dropLocation": DropLocation.Left }
    property var toolBarBottomDropDestination: { "dock": toolBarBottomDockingHolder, "dropLocation": DropLocation.Left }

    property var panelLeftDropDestination: { "dock": panelLeftDockingHolder, "dropLocation": DropLocation.Top }
    property var panelRightDropDestination: { "dock": panelRightDockingHolder, "dropLocation": DropLocation.Top }
    property var panelTopDropDestination: { "dock": panelTopDockingHolder, "dropLocation": DropLocation.Left }
    property var panelBottomDropDestination: { "dock": panelBottomDockingHolder, "dropLocation": DropLocation.Left }

    centralDock: DockCentralView {
        objectName: root.objectName + "_central"

        Loader {
            id: central

            anchors.fill: parent
        }
    }

    toolBarsDockingHolders: [
        DockingHolder {
            id: toolBarLeftDockingHolder

            objectName: root.objectName + "_toolBarsDockingHolderLeft"
            location: DockBase.Left
        },
        DockingHolder {
            id: toolBarRightDockingHolder

            objectName: root.objectName + "_toolBarsDockingHolderRight"
            location: DockBase.Right
        },
        DockingHolder {
            id: toolBarTopDockingHolder

            objectName: root.objectName + "_toolBarsDockingHolderTop"
            location: DockBase.Top
        },
        DockingHolder {
            id: toolBarBottomDockingHolder

            objectName: root.objectName + "_toolBarsDockingHolderBottom"
            location: DockBase.Bottom
        }
    ]

    panelsDockingHolders: [
        DockingHolder {
            id: panelLeftDockingHolder

            objectName: root.objectName + "_panelsDockingHolderLeft"
            location: DockBase.Left
        },
        DockingHolder {
            id: panelRightDockingHolder

            objectName: root.objectName + "_panelsDockingHolderRight"
            location: DockBase.Right
        },
        DockingHolder {
            id: panelTopDockingHolder

            objectName: root.objectName + "_panelsDockingHolderTop"
            location: DockBase.Top
        },
        DockingHolder {
            id: panelBottomDockingHolder

            objectName: root.objectName + "_panelsDockingHolderBottom"
            location: DockBase.Bottom
        }
    ]
}
