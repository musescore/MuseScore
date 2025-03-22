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
import QtQuick.Layouts 1.15
import Muse.Ui 1.0
import Muse.UiComponents 1.0

ListView {
    id: root

    property bool showArrangement: true
    property bool showPitch: true
    property bool showExpression: true

    orientation: Qt.Horizontal

    delegate: Item {
        id: delegateItem

        property bool isSelected: root.model.currentPatternSegment === patternSegmentItem

        height: childrenRect.height
        width: childrenRect.width

        ArticulationPatternPlot {
            id: thumbnailPlot

            height: 64
            width: 64
            patternModel: patternSegmentItem
            thumbnailModeOn: true

            showArrangement: root.showArrangement
            showPitch: root.showPitch
            showExpression: root.showExpression

            MouseArea {
                id: thumbnailMouseArea

                anchors.fill: parent
                hoverEnabled: true

                onClicked: {
                    root.model.currentPatternSegment = patternSegmentItem
                }
            }
        }

        FlatButton {
            id: deleteButton

            anchors.horizontalCenter: thumbnailPlot.horizontalCenter
            anchors.bottom: thumbnailPlot.top
            anchors.bottomMargin: 4

            mouseArea.anchors.margins: -4
            enabled: root.model.isAbleToRemoveCurrentSegment()
            visible: (thumbnailMouseArea.containsMouse && thumbnailPlot.enabled) || (deleteButton.mouseArea.containsMouse && deleteButton.enabled)

            transparent: true

            icon: IconCode.CLOSE_X_ROUNDED

            onClicked: {
                if (root.model.isAbleToRemoveCurrentSegment) {
                    root.model.removeCurrentSegment()
                }
            }
        }

        FlatButton {
            id: createButton

            anchors.verticalCenter: thumbnailPlot.verticalCenter
            anchors.left: thumbnailPlot.right
            anchors.leftMargin: 4

            mouseArea.anchors.margins: -4
            visible: (thumbnailMouseArea.containsMouse && thumbnailPlot.enabled) || (createButton.mouseArea.containsMouse && createButton.enabled)

            transparent: true

            icon: IconCode.PLUS

            onClicked: {
                root.model.appendNewSegment()
            }
        }

        states: [
            State {
                name: "HOVERED"
                when: ((thumbnailMouseArea.containsMouse && thumbnailPlot.enabled)
                            || (createButton.mouseArea.containsMouse && createButton.enabled)
                            || (deleteButton.mouseArea.containsMouse && deleteButton.enabled)
                            || delegateItem.isSelected)
                      && !thumbnailMouseArea.containsPress

                PropertyChanges {
                    target: thumbnailPlot
                    scale: 1.10
                }
            },

            State {
                name: "SELECTED"
                when: thumbnailMouseArea.containsPress && thumbnailPlot.enabled

                PropertyChanges {
                    target: thumbnailPlot
                    scale: 0.9
                }
            }
        ]

        transitions: Transition {
            NumberAnimation {
                target: thumbnailPlot
                properties: "scale"; easing.type: Easing.InOutQuad
                duration: 100
            }
        }
    }
}
