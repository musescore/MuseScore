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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.InstrumentsScene 1.0

import "internal"

Rectangle {
    id: root

    property bool canSelectMultipleInstruments: true
    property string currentInstrumentId: ""

    property bool hasSelectedInstruments: instrumentsOnScoreView.hasInstruments

    property NavigationSection navigationSection: null

    function instruments() {
        if (root.canSelectMultipleInstruments) {
            return instrumentsOnScoreView.instruments()
        }

        return instrumentsModel.selectedInstruments()
    }

    function currentOrder() {
        if (root.canSelectMultipleInstruments) {
            return instrumentsOnScoreView.currentOrder()
        }

        return undefined
    }

    color: ui.theme.backgroundPrimaryColor

    InstrumentListModel {
        id: instrumentsModel

        onFocusRequested: {
            familyView.focusGroup(groupIndex)
            instrumentsView.focusInstrument(instrumentIndex)
        }
    }

    QtObject {
        id: prv

        function addSelectedInstrumentsToScore() {
            var selectedInstruments = instrumentsModel.selectedInstruments()
            instrumentsOnScoreView.addInstruments(selectedInstruments)

            Qt.callLater(instrumentsOnScoreView.scrollViewToEnd)
        }
    }

    Component.onCompleted: {
        instrumentsModel.load(canSelectMultipleInstruments, currentInstrumentId)
    }

    RowLayout {
        anchors.fill: parent

        spacing: 12

        FamilyView {
            id: familyView

            Layout.preferredWidth: root.canSelectMultipleInstruments ? root.width / 5 : 0
            Layout.fillWidth: !root.canSelectMultipleInstruments
            Layout.fillHeight: true

            navigation.section: root.navigationSection
            navigation.order: 2

            genres: instrumentsModel.genres
            groups: instrumentsModel.groups

            currentGenreIndex: instrumentsModel.currentGenreIndex
            currentGroupIndex: instrumentsModel.currentGroupIndex

            onGenreSelected: {
                instrumentsModel.currentGenreIndex = newIndex
                instrumentsView.clearSearch()
            }

            onGroupSelected: {
                instrumentsModel.currentGroupIndex = newIndex
                instrumentsView.clearSearch()
            }
        }

        SeparatorLine { orientation: Qt.Vertical }

        InstrumentsView {
            id: instrumentsView

            Layout.preferredWidth: root.canSelectMultipleInstruments ? root.width / 4 : 0
            Layout.fillWidth: !root.canSelectMultipleInstruments
            Layout.fillHeight: true

            navigation.section: root.navigationSection
            navigation.order: 3

            instrumentsModel: instrumentsModel

            onAddSelectedInstrumentsToScoreRequested: {
                prv.addSelectedInstrumentsToScore()
            }
        }

        SeparatorLine {
            visible: root.canSelectMultipleInstruments
            orientation: Qt.Vertical
        }

        FlatButton {
            NavigationPanel {
                id: navSelectPanel
                name: "SelectPanel"
                section: root.navigationSection
                order: 4
                enabled: root.visible
            }

            navigation.name: "Select"
            navigation.panel: navSelectPanel
            navigation.order: 1

            visible: root.canSelectMultipleInstruments

            Layout.preferredWidth: 30

            enabled: instrumentsModel.hasSelection

            icon: IconCode.ARROW_RIGHT

            onClicked: {
                prv.addSelectedInstrumentsToScore()
            }
        }

        SeparatorLine {
            visible: root.canSelectMultipleInstruments
            orientation: Qt.Vertical
        }

        InstrumentsOnScoreView {
            id: instrumentsOnScoreView

            navigation.section: root.navigationSection
            navigation.order: 5

            visible: root.canSelectMultipleInstruments

            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        SeparatorLine {
            visible: root.canSelectMultipleInstruments
            orientation: Qt.Vertical
        }

        Column {
            visible: root.canSelectMultipleInstruments

            Layout.preferredWidth: 30
            Layout.alignment: Qt.AlignVCenter

            spacing: 12

            NavigationPanel {
                id: navUpDownPanel
                name: "UpDownSelected"
                section: root.navigationSection
                order: 6
                direction: NavigationPanel.Vertical
                enabled: root.visible
            }

            FlatButton {
                enabled: instrumentsOnScoreView.isMovingUpAvailable
                icon: IconCode.ARROW_UP

                navigation.name: "Up"
                navigation.panel: navUpDownPanel
                navigation.row: 1

                onClicked: {
                    instrumentsOnScoreView.moveSelectedInstrumentsUp()
                }
            }

            FlatButton {
                enabled: instrumentsOnScoreView.isMovingDownAvailable
                icon: IconCode.ARROW_DOWN

                navigation.name: "Down"
                navigation.panel: navUpDownPanel
                navigation.row: 2

                onClicked: {
                    instrumentsOnScoreView.moveSelectedInstrumentsDown()
                }
            }
        }
    }
}
