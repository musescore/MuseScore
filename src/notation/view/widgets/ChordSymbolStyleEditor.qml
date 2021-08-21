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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.NotationScene 1.0

Rectangle {
    id: root
    color: ui.theme.backgroundPrimaryColor

    property EditStyle editStyleDialog

    ChordSymbolEditorModel {
        id: chordSymbolEditorModel
    }

    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        // TODO: margins 24

        StyledTextLabel {
            text: qsTrc("notation", "Chord symbols")
            font: ui.theme.headerBoldFont
            anchors.bottomMargin: 24
        }

        StyledTextLabel {
            text: qsTrc("notation", "Choose a style")
            font: ui.theme.bodyBoldFont
            anchors.bottomMargin: 12
        }

        ListView {
            id: chordSymbolStyleList
            Layout.fillWidth: true

            height: 122
            spacing: 21

            clip: true
            orientation: Qt.Horizontal

            model: chordSymbolEditorModel
            currentIndex: chordSymbolEditorModel.currentStyleIndex

            delegate: Column {
                FlatRadioButton {
                    id: button

                    width: 170
                    height: 76

                    anchors.bottomMargin: 8

                    ButtonGroup.group: chordSymbolStyleList.children

                    opacity: 0.7

                    StyledTextLabel {
                        text: fileName
                    }

                    checked: chordSymbolEditorModel.currentStyleIndex === index

                    onClicked: {
                        chordSymbolEditorModel.setChordStyle(index)
                    }
                }
                StyledTextLabel{
                    anchors.topMargin: 8
                    width: 170
                    height: 14
                    horizontalAlignment: Text.AlignHCenter
                    text: styleName
                }
            }

            ScrollBar.horizontal: StyledScrollBar{
                anchors.bottom: chordSymbolStyleList.bottom
            }
        }

        SeparatorLine {}

        StyledTextLabel {
            anchors.topMargin: 20
            font: ui.theme.bodyBoldFont
            text: qsTrc("notation", "Adjustments")
        }

        TabBar {
            id: bar
            width: 280
            height: 40
            anchors.topMargin: 7

            StyledTabButton {
                text: qsTrc("notation", "Basic")
                width: bar.width/2
                height: bar.height
                isCurrent: bar.currentIndex === 0
                font: ui.theme.bodyBoldFont
            }

            StyledTabButton {
                text: qsTrc("notation", "Advanced")
                width: bar.width/2
                height: bar.height
                isCurrent: bar.currentIndex === 1
                font: ui.theme.bodyBoldFont
            }
        }

        StackLayout {
            id: editorStack

            Layout.fillWidth: true
            Layout.fillHeight: true

            currentIndex: bar.currentIndex

            ChordSymbolEditorBasic {
                editorModel: chordSymbolEditorModel
            }

            ChordSymbolEditorAdvanced {
                editorModel: chordSymbolEditorModel
                editStyleInstance: root.editStyleDialog
            }
        }
    }
}
