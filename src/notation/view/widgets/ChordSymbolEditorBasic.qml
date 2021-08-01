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

        StyledTextLabel {
            text: qsTrc("notation","Chord spelling")
            font: ui.theme.bodyFont
        }

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

        StyledTextLabel {
            text: qsTrc("notation","Bass Note")
            font: ui.theme.bodyFont
            visible: (root.editorModel.bassNoteIndex != -1)
        }

        ListView {
            id: bassNoteListView
            anchors.left: parent.left
            anchors.right: parent.right

            height: prv.listCellHeight
            spacing: prv.listCellSpacing
            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds

            model: root.editorModel.bassNoteList
            currentIndex: root.editorModel.bassNoteIndex
            visible: (root.editorModel.bassNoteIndex != -1)
            enabled: root.editorModel.usePresets

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setQualitySymbol("bassNote", index);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
                visible: root.editorModel.usePresets
            }
        }

        StyledTextLabel {
            text: qsTrc("notation","Major seventh")
            font: ui.theme.bodyFont
            visible: (root.editorModel.majorSeventhIndex != -1)
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
            visible: (root.editorModel.majorSeventhIndex != -1)
            enabled: root.editorModel.usePresets

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setQualitySymbol("major7th", index);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
                visible: root.editorModel.usePresets
            }
        }

        StyledTextLabel {
            text: qsTrc("notation","Half-diminished")
            font: ui.theme.bodyFont
            visible: (root.editorModel.halfDiminishedIndex != -1)
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
            visible: (root.editorModel.halfDiminishedIndex != -1)            
            enabled: root.editorModel.usePresets

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setQualitySymbol("half-diminished", index);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
                visible: root.editorModel.usePresets
            }
        }

        StyledTextLabel {
            text: qsTrc("notation","Minor")
            font: ui.theme.bodyFont
            visible: (root.editorModel.minorIndex != -1)
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
            visible: (root.editorModel.minorIndex != -1)
            enabled: root.editorModel.usePresets

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setQualitySymbol("minor", index);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
                visible: root.editorModel.usePresets
            }
        }

        StyledTextLabel {
            text: qsTrc("notation","Augmented")
            font: ui.theme.bodyFont
            visible: (root.editorModel.augmentedIndex != -1)
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
            visible: (root.editorModel.augmentedIndex != -1)
            enabled: root.editorModel.usePresets

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setQualitySymbol("augmented", index);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
                visible: root.editorModel.usePresets
            }
        }

        StyledTextLabel {
            text: qsTrc("notation","Diminished")
            font: ui.theme.bodyFont
            visible: (root.editorModel.diminishedIndex != -1)
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
            visible: (root.editorModel.diminishedIndex != -1)
            enabled: root.editorModel.usePresets

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setQualitySymbol("diminished", index);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
                visible: root.editorModel.usePresets
            }
        }

        StyledTextLabel {
            text: qsTrc("notation","Six-nine")
            font: ui.theme.bodyFont
            visible: (root.editorModel.sixNineIndex != -1)
        }

        ListView {
            id: sixNineListView
            anchors.left: parent.left
            anchors.right: parent.right

            height: prv.listCellHeight
            spacing: prv.listCellSpacing
            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds

            model: root.editorModel.sixNineList
            currentIndex: root.editorModel.sixNineIndex
            visible: (root.editorModel.sixNineIndex != -1)
            enabled: root.editorModel.usePresets

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setQualitySymbol("sixNine", index);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
                visible: root.editorModel.usePresets
            }
        }

        StyledTextLabel {
            text: qsTrc("notation","Omissions")
            font: ui.theme.bodyFont
            visible: (root.editorModel.omitIndex != -1)
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
            visible: (root.editorModel.omitIndex != -1)
            enabled: root.editorModel.usePresets

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setQualitySymbol("omit", index);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
                visible: root.editorModel.usePresets
            }
        }

        StyledTextLabel {
            text: qsTrc("notation","Suspensions")
            font: ui.theme.bodyFont
            visible: (root.editorModel.suspensionIndex != -1)
        }

        ListView {
            id: suspensionListView
            anchors.left: parent.left
            anchors.right: parent.right

            height: prv.listCellHeight
            spacing: prv.listCellSpacing
            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds

            model: root.editorModel.suspensionList
            currentIndex: root.editorModel.suspensionIndex
            visible: (root.editorModel.suspensionIndex != -1)
            enabled: root.editorModel.usePresets

            delegate: FlatButton {
                height: prv.listCellHeight
                width: prv.listCellWidth
                text: modelData

                onClicked: {
                    root.editorModel.setQualitySymbol("suspension", index);
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                radius: 3
                visible: root.editorModel.usePresets
            }
        }

        StyledTextLabel {
            text: qsTrc("notation","Alterations")
            font: ui.theme.bodyFont
            opacity: root.editorModel.usePresets ? 1 : 0.5
        }

        RadioButtonGroup {
            id: stackModifiers

            height: prv.listCellHeight

            enabled: root.editorModel.usePresets

            model: [
                { name: "Stacked", value: 1.0 },
                { name: "Non-Stacked", value: 0.0 },
            ]

            delegate: FlatRadioButton {

                height: prv.listCellHeight
                width: prv.listCellWidth

                ButtonGroup.group: stackModifiers.radioButtonGroup

                StyledTextLabel{
                    text: qsTrc("notation",modelData["name"])
                }
                checked: editorModel.stackModifiers === modelData["value"]

                onToggled: {
                    editorModel.setProperty("stackModifiers", modelData["value"])
                }
            }
            highlight: {
                visible: root.editorModel.usePresets
            }
        }

        StyledTextLabel{
            text:editorModel.styleDescription
            font: ui.theme.bodyFont
        }
    }
}
