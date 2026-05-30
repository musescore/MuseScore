/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 */

import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Playback

StyledDialogView {
    id: root

    title: qsTrc("playback", "Reassign Muse Sounds")

    contentWidth: 920
    contentHeight: 520
    margins: 24
    modal: true

    Component.onCompleted: {
        reassignModel.load()
    }

    MuseSoundsReassignModel {
        id: reassignModel
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        StyledListView {
            id: choicesView

            Layout.fillWidth: true
            Layout.fillHeight: true

            model: reassignModel
            spacing: 8

            navigation.name: "MuseSoundsReassignChoices"
            accessible.name: root.title

            delegate: ListItemBlank {
                id: choiceItem

                required property string staffName
                required property string currentSound
                required property var candidateTitles
                required property int selectedCandidateIndex
                required property int candidateCount
                required property int index

                height: 64

                navigation.panel: choicesView.navigation
                navigation.row: index
                navigation.accessible.name: staffName

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    anchors.topMargin: 8
                    anchors.bottomMargin: 8
                    spacing: 16

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 300
                        spacing: 2

                        StyledTextLabel {
                            Layout.fillWidth: true
                            text: choiceItem.staffName
                            font: ui.theme.bodyBoldFont
                            horizontalAlignment: Text.AlignLeft
                        }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            text: choiceItem.currentSound
                            opacity: 0.7
                            horizontalAlignment: Text.AlignLeft
                        }
                    }

                    StyledDropdown {
                        id: soundDropdown

                        Layout.preferredWidth: 500
                        Layout.alignment: Qt.AlignVCenter
                        height: 32

                        enabled: choiceItem.candidateCount > 0
                        model: choiceItem.candidateTitles
                        currentIndex: choiceItem.selectedCandidateIndex

                        navigation.name: "MuseSoundsChoice"
                        navigation.panel: choicesView.navigation
                        navigation.row: choiceItem.index
                        navigation.column: 1

                        onActivated: function(index, value) {
                            reassignModel.setSelectedCandidate(choiceItem.index, index)
                        }
                    }
                }
            }
        }

        StyledTextLabel {
            Layout.fillWidth: true
            visible: !reassignModel.hasChoices
            text: qsTrc("playback", "No matching Muse Sounds were found for this score.")
            horizontalAlignment: Text.AlignHCenter
        }

        ButtonBox {
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom

            buttons: [ ButtonBoxModel.Cancel ]

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            FlatButton {
                text: qsTrc("global", "OK")
                buttonRole: ButtonBoxModel.AcceptRole
                buttonId: ButtonBoxModel.Ok
                enabled: reassignModel.hasChoices
                accentButton: true

                onClicked: {
                    reassignModel.apply()
                    root.hide()
                }
            }

            onStandardButtonClicked: function(buttonId) {
                if (buttonId === ButtonBoxModel.Cancel) {
                    root.reject()
                }
            }
        }
    }
}
