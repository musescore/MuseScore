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
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

import com.kdab.dockwidgets 1.0 as KDDW

import "../HomePage"

Page {
    id: root

    property string uri: ""
    property alias uniqueName: layout.uniqueName

    property list<DockPanel> panels

    property alias statusBar: root.footer
    property alias central: central.sourceComponent

    padding: 0

    background: Rectangle {
        color: ui.theme.backgroundPrimaryColor
    }

    contentItem: KDDW.MainWindowLayout {
        id: layout

        anchors.fill: parent

        Repeater {
            model: root.panels

            delegate: KDDW.DockWidget {
                id: panel

                uniqueName: modelData.uniqueName

                Loader {
                    sourceComponent: modelData.content
                }

                Component.onCompleted: {
                    Qt.callLater(init)
                }

                function init() {
                    layout.addDockWidget(panel, KDDW.KDDockWidgets.Location_OnLeft)
                }
            }
        }

        KDDW.DockWidget {
            id: centralDock

            uniqueName: root.uniqueName + "_central"

            Loader {
                id: central

                anchors.fill: parent
            }
        }

        Component.onCompleted: {
            addDockWidget(centralDock, KDDW.KDDockWidgets.Location_OnRight)
        }
    }
}
