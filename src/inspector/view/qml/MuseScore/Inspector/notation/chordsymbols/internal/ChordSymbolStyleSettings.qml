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

    property alias chordStylesModel: chordSymbolStylesGrid.model

    height: grid.height

        Component {
            id: styleDelegate

            FlatButton {
                id: button

                width: chordSymbolStylesGrid.cellWidth-5
                height: chordSymbolStylesGrid.cellHeight-5

                text: styleName

                onClicked: {
                    chordStylesModel.setChordStyle(index)
                }
            }
        }

        GridView {
            id: chordSymbolStylesGrid

            anchors.left: root.left
            anchors.right: root.right

            height: cellHeight
            width: cellWidth

            cellWidth: 120
            cellHeight: 60

            currentIndex: chordStylesModel.currentStyleIndex

            delegate: styleDelegate
            clip: true

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
            }
        }
}
