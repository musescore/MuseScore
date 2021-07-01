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
    property var listCellHeight: 50
    property var listCellWidth: 80
    property var listCellMargin: 6

    Flickable {
        id: flickableContainer

        height: 200
        width: root.width

        boundsBehavior: Flickable.StopAtBounds
        interactive: true

        contentHeight: 400

        ScrollBar.vertical: StyledScrollBar {}
        Column{
            GridView {
                id: chordSpellingGridView

                height: listCellHeight + 2*listCellMargin
                width: root.width

                anchors.left: flickableContainer.left

                cellHeight: listCellHeight
                cellWidth: listCellWidth + listCellMargin

                model: editorModel.chordSpellingList
                currentIndex: editorModel.chordSpellingIndex

                delegate: FlatButton {
                    height: listCellHeight
                    width: listCellWidth
                    anchors.rightMargin: listCellMargin
                    text: modelData

                    onClicked: {
                        editorModel.setChordSpelling(modelData);
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

                height: listCellHeight + 2*listCellMargin
                width: root.width

                anchors.left: flickableContainer.left

                cellHeight: listCellHeight
                cellWidth: listCellWidth + listCellMargin

                model: editorModel.majorSeventhList
                currentIndex: editorModel.majorSeventhIndex

                delegate: FlatButton {
                    height: listCellHeight
                    width: listCellWidth
                    anchors.rightMargin: listCellMargin
                    text: modelData

                    onClicked: {
                        editorModel.setQualitySymbol("major7th",modelData);
                    }
                }

                boundsBehavior: Flickable.StopAtBounds

                highlight: Rectangle {
                    color: ui.theme.accentColor
                    radius: 3
                }
            }

            GridView {
                id: halfDiminishedGridView

                height: listCellHeight + 2*listCellMargin
                width: root.width

                anchors.left: flickableContainer.left

                anchors.top: majorSeventhGridView.bottom

                cellHeight: listCellHeight
                cellWidth: listCellWidth + listCellMargin

                model: editorModel.halfDiminishedList
                currentIndex: editorModel.halfDiminishedIndex

                delegate: FlatButton {
                    height: listCellHeight
                    width: listCellWidth
                    anchors.rightMargin: listCellMargin
                    text: modelData

                    onClicked: {
                        editorModel.setQualitySymbol("half-diminished",modelData);
                    }
                }

                boundsBehavior: Flickable.StopAtBounds

                highlight: Rectangle {
                    color: ui.theme.accentColor
                    radius: 3
                }
            }

            GridView {
                id: minorGridView

                height: listCellHeight + 2*listCellMargin
                width: root.width

                anchors.left: flickableContainer.left

                cellHeight: listCellHeight
                cellWidth: listCellWidth + listCellMargin

                anchors.top: halfDiminishedGridView.bottom

                model: editorModel.minorList
                currentIndex: editorModel.minorIndex

                delegate: FlatButton {
                    height: listCellHeight
                    width: listCellWidth
                    anchors.rightMargin: listCellMargin
                    text: modelData

                    onClicked: {
                        editorModel.setQualitySymbol("minor",modelData);
                    }
                }

                boundsBehavior: Flickable.StopAtBounds

                highlight: Rectangle {
                    color: ui.theme.accentColor
                    radius: 3
                }
            }

            GridView {
                id: augmentedGridView

                height: listCellHeight + 2*listCellMargin
                width: root.width

                anchors.left: flickableContainer.left

                anchors.top: minorGridView.bottom

                cellHeight: listCellHeight
                cellWidth: listCellWidth + listCellMargin

                model: editorModel.augmentedList
                currentIndex: editorModel.augmentedIndex

                delegate: FlatButton {
                    height: listCellHeight
                    width: listCellWidth
                    anchors.rightMargin: listCellMargin
                    text: modelData

                    onClicked: {
                        editorModel.setQualitySymbol("augmented",modelData);
                    }
                }

                boundsBehavior: Flickable.StopAtBounds

                highlight: Rectangle {
                    color: ui.theme.accentColor
                    radius: 3
                }
            }

            GridView {
                id: diminishedGridView

                height: listCellHeight + 2*listCellMargin
                width: root.width

                anchors.left: flickableContainer.left

                anchors.top: augmentedGridView.bottom

                cellHeight: listCellHeight
                cellWidth: listCellWidth + listCellMargin

                model: editorModel.diminishedList
                currentIndex: editorModel.diminishedIndex

                delegate: FlatButton {
                    height: listCellHeight
                    width: listCellWidth
                    anchors.rightMargin: listCellMargin
                    text: modelData

                    onClicked: {
                        editorModel.setQualitySymbol("diminished",modelData);
                    }
                }

                boundsBehavior: Flickable.StopAtBounds

                highlight: Rectangle {
                    color: ui.theme.accentColor
                    radius: 3
                }
            }

            GridView {
                id: omitGridView

                height: listCellHeight + 2*listCellMargin
                width: root.width

                anchors.left: flickableContainer.left

                anchors.top: diminishedGridView.bottom

                cellHeight: listCellHeight
                cellWidth: listCellWidth + listCellMargin

                model: editorModel.omitList
                currentIndex: editorModel.omitIndex

                delegate: FlatButton {
                    height: listCellHeight
                    width: listCellWidth
                    anchors.rightMargin: listCellMargin
                    text: modelData

                    onClicked: {
                        editorModel.setQualitySymbol("omit",modelData);
                    }
                }

                boundsBehavior: Flickable.StopAtBounds

                highlight: Rectangle {
                    color: ui.theme.accentColor
                    radius: 3
                }
            }

        }
    }
}
