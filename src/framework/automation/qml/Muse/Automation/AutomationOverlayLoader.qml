/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

pragma ComponentBehavior: Bound

import QtQuick 2.15

import Muse.Automation

Loader {
    id: root

    property var viewMatrix: null
    property var linesData: null

    signal pointChangeRequested(var lineIdx, var pointIdx, var x, var y)

    sourceComponent: AutomationOverlay {
        id: automationOverlay

        viewMatrix: root.viewMatrix

        onPointChangeRequested: function(lineIdx, pointIdx, x, y) {
            root.pointChangeRequested(lineIdx, pointIdx, x, y)
        }

        Connections {
            target: root
            function onLoaded() {
                automationOverlay.initAutomationLinesData(root.linesData)
            }
        }

        Connections {
            target: root
            function onLinesDataChanged() {
                automationOverlay.initAutomationLinesData(root.linesData)
            }
        }
    }
}
