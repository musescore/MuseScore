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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Rectangle {
    id: root

    property var editorModel: null
    property var listCellHeight: 60
    property var listCellWidth: 60

    Flickable {
        id: flickableContainer

        height: root.height
        width: root.width

        boundsBehavior: Flickable.StopAtBounds
        interactive: true

        ScrollBar.vertical: StyledScrollBar {}

        GridView {
            id: chordSpellingGridView

            height: listCellHeight
            width: root.width

            anchors.left: flickableContainer.left

            cellHeight: listCellHeight
            cellWidth: listCellWidth

            model: editorModel.chordSpellingList

            delegate: FlatButton {
                height: listCellHeight
                width: listCellWidth
                text: modelData
                onClicked: {
                    editorModel.setChordSpelling(modelData);
                    chordSpellingGridView.currentIndex = index
                }
            }

            boundsBehavior: Flickable.StopAtBounds

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
            }
        }

        GridView {
            id: majorSeventhGridView

            anchors.top: chordSpellingGridView.bottom

            height: listCellHeight
            width: root.width

            anchors.left: flickableContainer.left

            cellHeight: listCellHeight
            cellWidth: listCellWidth

            model: editorModel.majorSeventhList

            delegate: FlatButton {
                height: listCellHeight
                width: listCellWidth
                text: modelData
            }

            boundsBehavior: Flickable.StopAtBounds

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
            }
        }

        GridView {
            id: halfDiminishedGridView

            height: listCellHeight
            width: root.width

            anchors.left: flickableContainer.left

            anchors.top: majorSeventhGridView.bottom

            cellHeight: listCellHeight
            cellWidth: listCellWidth

            model: editorModel.halfDiminishedList

            delegate: FlatButton {
                height: listCellHeight
                width: listCellWidth
                text: modelData
            }

            boundsBehavior: Flickable.StopAtBounds

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
            }
        }

        GridView {
            id: minorGridView

            height: listCellHeight
            width: root.width

            anchors.left: flickableContainer.left

            cellHeight: listCellHeight
            cellWidth: listCellWidth

            anchors.top: halfDiminishedGridView.bottom

            model: editorModel.minorList

            delegate: FlatButton {
                height: listCellHeight
                width: listCellWidth
                text: modelData
            }

            boundsBehavior: Flickable.StopAtBounds

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
            }
        }

        GridView {
            id: augmentedGridView

            height: listCellHeight
            width: root.width

            anchors.left: flickableContainer.left

            anchors.top: minorGridView.bottom

            cellHeight: listCellHeight
            cellWidth: listCellWidth

            model: editorModel.augmentedList

            delegate: FlatButton {
                height: listCellHeight
                width: listCellWidth
                text: modelData
            }

            boundsBehavior: Flickable.StopAtBounds

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
            }
        }

        GridView {
            id: diminishedGridView

            height: listCellHeight
            width: root.width

            anchors.left: flickableContainer.left

            anchors.top: augmentedGridView.bottom

            cellHeight: listCellHeight
            cellWidth: listCellWidth

            model: editorModel.diminishedList

            delegate: FlatButton {
                height: listCellHeight
                width: listCellWidth
                text: modelData
            }

            boundsBehavior: Flickable.StopAtBounds

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
            }
        }
    }
}
