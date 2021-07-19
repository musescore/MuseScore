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
import MuseScore.NotationScene 1.0

Flickable {
    id: root

    property ChordSymbolEditorModel editorModel: null

    QtObject {
        id: prv
        readonly property int listCellHeight: 56
        readonly property int listCellWidth: 106
        readonly property int listCellSpacing: 10
    }

    contentWidth: width
    contentHeight: content.height

    clip: true
    boundsBehavior: Flickable.StopAtBounds

    ScrollBar.vertical: StyledScrollBar {}

    Column {
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 12

        ListView {
            id: chordSpellingListView
            anchors.left: parent.left
            anchors.right: parent.right

            height: prv.listCellHeight
            spacing: prv.listCellSpacing
            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds

            model: root.editorModel.chordSpellingList
            currentIndex: root.editorModel.chordSpellingIndex

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setChordSpelling(modelData);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
            }
        }

        ListView {
            id: majorSeventhListView
            anchors.left: parent.left
            anchors.right: parent.right

            height: prv.listCellHeight
            spacing: prv.listCellSpacing
            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds

            model: root.editorModel.majorSeventhList
            currentIndex: root.editorModel.majorSeventhIndex

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setQualitySymbol("major7th", modelData);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
            }
        }

        ListView {
            id: halfDiminishedListView
            anchors.left: parent.left
            anchors.right: parent.right

            height: prv.listCellHeight
            spacing: prv.listCellSpacing
            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds

            model: root.editorModel.halfDiminishedList
            currentIndex: root.editorModel.halfDiminishedIndex

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setQualitySymbol("half-diminished", modelData);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
            }
        }

        ListView {
            id: minorListView
            anchors.left: parent.left
            anchors.right: parent.right

            height: prv.listCellHeight
            spacing: prv.listCellSpacing
            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds

            model: root.editorModel.minorList
            currentIndex: root.editorModel.minorIndex

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setQualitySymbol("minor", modelData);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
            }
        }

        ListView {
            id: augmentedListView
            anchors.left: parent.left
            anchors.right: parent.right

            height: prv.listCellHeight
            spacing: prv.listCellSpacing
            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds

            model: root.editorModel.augmentedList
            currentIndex: root.editorModel.augmentedIndex

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setQualitySymbol("augmented", modelData);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
            }
        }

        ListView {
            id: diminishedListView
            anchors.left: parent.left
            anchors.right: parent.right

            height: prv.listCellHeight
            spacing: prv.listCellSpacing
            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds

            model: root.editorModel.diminishedList
            currentIndex: root.editorModel.diminishedIndex

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setQualitySymbol("diminished", modelData);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
            }
        }

        ListView {
            id: omitListView
            anchors.left: parent.left
            anchors.right: parent.right

            height: prv.listCellHeight
            spacing: prv.listCellSpacing
            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds

            model: root.editorModel.omitList
            currentIndex: root.editorModel.omitIndex

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setQualitySymbol("omit", modelData);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
            }
        }

        RadioButtonGroup {
            id: stackModifiers

            height: 30

            anchors.top: omitGridView.bottom

            model: [
                { name: "Stacked", value: 1.0 },
                { name: "Non-Stacked", value: 0.0 },
            ]

            delegate: FlatRadioButton {
                ButtonGroup.group: stackModifiers.radioButtonGroup

                StyledTextLabel{
                    text: qsTrc("notation",modelData["name"])
                }
                checked: editorModel.stackModifiers === modelData["value"]

                onToggled: {
                    editorModel.setProperty("stackModifiers", modelData["value"])
                }
            }
        }

        StyledTextLabel{
            text:editorModel.styleDescription
            font: ui.theme.bodyFont
        }
    }
}
