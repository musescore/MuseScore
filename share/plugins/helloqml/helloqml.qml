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
import QtQuick 2.0
import MuseScore 3.0


MuseScore {
    menuPath:    "Plugins.helloQml"
    version:     "3.0"
    description: qsTr("This demo plugin shows some basic tasks.")
    pluginType: "dialog"

    width:  150
    height: 75
    onRun: {
        console.log(qsTr("hello world"));
        var score = curScore
        console.log(curScore)
        console.log(score.name)
        var m
        m = score.firstMeasure
        while (m) {
            console.log(qsTr("measure"))
            var segment = m.firstSegment
            while (segment) {
                var element
                element = segment.elementAt(0)
                if (element && element.type == Element.CHORD) {
                    console.log(qsTr("    element"))
                    console.log(element.beamMode)
                    if (element.beamMode == Beam.NONE)
                        console.log("  beam no")
                    }
                segment = segment.next
                }
            m = m.nextMeasure
            }
        }

    Rectangle {
        color: "grey"
        anchors.fill: parent

        Text {
            anchors.centerIn: parent
            text: qsTr("Hello Qml")
            }

        MouseArea {
            anchors.fill: parent
            onClicked: Qt.quit()
            }
        }
    }

