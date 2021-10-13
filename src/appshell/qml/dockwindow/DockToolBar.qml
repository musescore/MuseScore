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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Dock 1.0

DockToolBarView {
    id: root

    property Component contentComponent

    Rectangle {
        anchors.fill: parent
        color: ui.theme.backgroundPrimaryColor

        Loader {
            id: loader
            anchors.fill: parent

            sourceComponent: root.orientation === Qt.Horizontal ? horizontalView : verticalView
            onLoaded: {
                root.setDraggableMouseArea(loader.item.gripMouseArea)
            }
        }
    }

    Component {
        id: horizontalView

        RowLayout {
            spacing: 0

            property alias gripMouseArea: gripButton.mouseArea

            FlatButton {
                id: gripButton

                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.margins: 2

                mouseArea.objectName: root.objectName + "_toolBarMouseAreaHorizontal"

                transparent: true
                icon: IconCode.TOOLBAR_GRIP

                visible: root.movable
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                sourceComponent: root.contentComponent
            }
        }
    }

    Component {
        id: verticalView

        ColumnLayout {
            spacing: 0

            property alias gripMouseArea: gripButton.mouseArea

            FlatButton {
                id: gripButton

                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                Layout.margins: 2

                mouseArea.objectName: root.objectName + "_toolBarMouseAreaVertical"

                transparent: true
                icon: IconCode.TOOLBAR_GRIP
                rotation: 90

                visible: root.movable
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                sourceComponent: root.contentComponent
            }
        }
    }
}
