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

Rectangle {

    id: root

    property int currentIndex: 0
    property alias model: view.model

    height: 30
    width: 120
    color: "#ff0000"

    StyledTextLabel {
        anchors.fill: parent
        text: root.model[root.currentIndex].text
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            popup.open()
        }
    }

    StyledPopupView {
        id: popup

        contentWidth: root.width
        contentHeight: 300
        padding: 0
        margins: 0
        showArrow: false

        x: 0
        y: 0

        ListView {
            id: view

            anchors.fill: parent
            clip: true

            delegate: ListItemBlank {
                height: root.height
                width: parent.width
                StyledTextLabel {
                    anchors.fill: parent
                    text: modelData.text
                }
                onClicked: {
                    root.currentIndex = model.index
                    popup.close()
                }
            }
        }
    }
}
