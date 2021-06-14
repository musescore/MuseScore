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
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../../common"

FocusableItem {

    property alias chordStylesModel: grid.model
    height: 2*grid.cellHeight

        Component {
            id: styleDelegate
            Column{
                FlatButton {
                    id: button
                    anchors.margins: 5
                    width: grid.cellWidth
                    height: grid.cellHeight
                    Text {
                        text: styleName
                        horizontalAlignment: button.horizontalCenter
                        verticalAlignment: button.verticalCenter
                        wrapMode: Text.Wrap
                    }
                }
                Text {
                    id: label
                    text: qsTr(styleName)
                }
            }

        }
        GridView {
            id: grid
            anchors.fill: parent
            cellWidth: grid.width/2;
            cellHeight: 80
            delegate: styleDelegate
        }
}
