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
import QtQuick.Controls 2.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

FocusableItem {
    id: root

    property alias chordStylesModel: grid.model
    height: 2*grid.cellHeight + 20
    width: 2*grid.cellWidth + 20

        Component {
            id: styleDelegate
            Column{
                FlatButton {
                    id: button
                    width: grid.cellWidth-10
                    height: 55
                    Text {
                        text: styleName
                        horizontalAlignment: button.horizontalCenter
                        verticalAlignment: button.verticalCenter
                        wrapMode: Text.Wrap
                    }
                    onClicked: {
                        chordStylesModel.setChordStyle(styleName)
                    }
                }
                Text {
                    id: label
                    text: qsTr(styleName)
                    width: grid.cellWidth-10
                    height: 15
                    horizontalAlignment: label.horizontalCenter
                }
            }

        }

        GridView {
            id: grid
            anchors.left: root.left
            anchors.right: root.right
            height: 2*cellHeight
            width: 2*cellWidth
            clip: true
            cellWidth: 100
            cellHeight: 80
            delegate: styleDelegate
        }
}
