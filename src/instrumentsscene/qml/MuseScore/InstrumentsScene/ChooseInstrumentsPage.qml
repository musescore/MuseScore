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

    property string initiallySelectedPartIds: ""
    property string currentScoreOrderId: ""
    property bool hasSelectedInstruments: instrumentsModel.selectedInstruments.length > 0
    property bool canSelectMultipleInstruments: true
    property string currentInstrumentId: ""
    property var selectedScoreOrder: selectedInstrumentsView.selectedScoreOrder()

    property NavigationSection navigationSection: null

    function selectedInstruments() {
        var instruments = instrumentsModel.selectedInstruments
        var result = []

        for (var i = 0; i < instruments.length; ++i) {
            var obj = {}
            obj["isExistingPart"] = instruments[i].isExistingPart
            obj["id"] = instruments[i].id
            obj["name"] = instruments[i].name
            obj["isSoloist"] = instruments[i].isSoloist
            obj["instrument"] = instruments[i].config

            result.push(obj)
        }

        return result
    }

    color: ui.theme.backgroundPrimaryColor

    InstrumentListModel {
        id: instrumentsModel
    }

    Component.onCompleted: {
        instrumentsModel.load(canSelectMultipleInstruments, currentInstrumentId, currentScoreOrderId, initiallySelectedPartIds)

        var groupId = instrumentsModel.selectedGroupId()
        familyView.focusGroup(groupId)

        focusOnCurrentInstrument()
    }

    function focusOnCurrentInstrument() {
        if (Boolean(currentInstrumentId)) {
            Qt.callLater(instrumentsView.focusInstrument, currentInstrumentId)
        }
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

            families: instrumentsModel.families
            groups: instrumentsModel.groups

            onFamilySelected: {
                instrumentsModel.selectFamily(familyId)
                instrumentsView.clearSearch()
            }

            onGroupSelected: {
                instrumentsModel.selectGroup(groupId)
                instrumentsView.clearSearch()
            }

            Connections {
                target: instrumentsModel

                function onSelectedFamilyChanged(familyId) {
                    Qt.callLater(familyView.setFamily, familyId)
                }

                function onSelectedGroupChanged(groupId) {
                    Qt.callLater(familyView.focusGroup, groupId)
                }
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

            instruments: instrumentsModel.instruments

            onSearchChanged: {
                familyView.clearSelection()
                instrumentsModel.setSearchText(search)
            }

            onSelectInstrumentRequested: {
                instrumentsModel.selectInstrument(instrumentName, traitName)
                Qt.callLater(selectedInstrumentsView.scrollViewToEnd)
            }

            onInstrumentClicked: {
                var currentSelection = instrumentsView.currentInstrument()
                familyView.focusGroup(currentSelection.instrument.groupId)

                if (!root.canSelectMultipleInstruments) {
                    instrumentsModel.selectInstrument(currentSelection.instrument.name, currentSelection.traitName)
                }
            }

            Connections {
                target: familyView

                function onGroupSelected(groupId) {
                    Qt.callLater(instrumentsView.resetSelectedInstrument)
                }
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

            enabled: instrumentsView.isInstrumentSelected

            icon: IconCode.ARROW_RIGHT

            onClicked: {
                var currentSelect = instrumentsView.currentInstrument()
                instrumentsModel.selectInstrument(currentSelect.instrument.name, currentSelect.traitName)
            }
        }

        SeparatorLine {
            visible: root.canSelectMultipleInstruments
            orientation: Qt.Vertical
        }

        SelectedInstrumentsView {
            id: selectedInstrumentsView

            navigation.section: root.navigationSection
            navigation.order: 5

            visible: root.canSelectMultipleInstruments

            Layout.fillWidth: true
            Layout.fillHeight: true

            instruments: instrumentsModel.selectedInstruments
            instrumentsModel: instrumentsModel

            onUnselectInstrumentRequested: {
                instrumentsModel.unselectInstrument(index)
            }

            onOrderChanged: {
                instrumentsModel.selectScoreOrder(id)
            }

            onSoloistChanged: {
                instrumentsModel.toggleSoloist(id)
            }
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
                enabled: selectedInstrumentsView.canLiftInstrument
                icon: IconCode.ARROW_UP

                navigation.name: "Up"
                navigation.panel: navUpDownPanel
                navigation.row: 1

                onClicked: {
                    instrumentsModel.swapSelectedInstruments(selectedInstrumentsView.currentInstrumentIndex,
                                                             selectedInstrumentsView.currentInstrumentIndex - 1)
                    selectedInstrumentsView.currentInstrumentIndex--
                }
            }

            FlatButton {
                enabled: selectedInstrumentsView.canLowerInstrument
                icon: IconCode.ARROW_DOWN

                navigation.name: "Down"
                navigation.panel: navUpDownPanel
                navigation.row: 2

                onClicked: {
                    instrumentsModel.swapSelectedInstruments(selectedInstrumentsView.currentInstrumentIndex,
                                                             selectedInstrumentsView.currentInstrumentIndex + 1)
                    selectedInstrumentsView.currentInstrumentIndex++
                }
            }
        }
    }
}
