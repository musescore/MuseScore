import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

Rectangle {
    id: root

    color: ui.theme.backgroundPrimaryColor

    function selectedInstrumentIds() {
        return instrumentsModel.selectedInstrumentIds()
    }

    InstrumentListModel {
        id: instrumentsModel
    }

    Component.onCompleted: {
        instrumentsModel.load()
        Qt.callLater(familyView.selectFirstGroup)
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16

        spacing: 16

        FamilyView {
            id: familyView

            Layout.preferredWidth: root.width / 4
            Layout.fillHeight: true

            families: instrumentsModel.families
            groups: instrumentsModel.groups

            onFamilySelected: {
                instrumentsModel.selectFamily(familyId)
                Qt.callLater(selectFirstGroup)
            }

            onGroupSelected: {
                instrumentsModel.selectGroup(groupId)
                Qt.callLater(instrumentsView.selectFirstInstrument)
            }

            Connections {
                target: instrumentsModel

                onSelectedFamilyChanged: {
                    familyView.setFamily(family)
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: 2
            Layout.fillHeight: true

            color: ui.theme.buttonColor
        }

        InstrumentsView {
            id: instrumentsView

            Layout.preferredWidth: root.width / 4
            Layout.fillHeight: true

            instruments: instrumentsModel.instruments

            onSearchChanged: {
                instrumentsModel.setSearchText(search)
                Qt.callLater(familyView.selectFirstGroup)
            }
        }

        Rectangle {
            Layout.preferredWidth: 2
            Layout.fillHeight: true

            color: ui.theme.buttonColor
        }

        FlatButton {
            Layout.preferredWidth: 30

            enabled: instrumentsView.isInstrumentSelected

            icon: IconCode.ARROW_RIGHT

            onClicked: {
                var currentSelect = instrumentsView.currentInstrument()
                instrumentsModel.selectInstrument(currentSelect.instrument.id, currentSelect.transposition)
            }
        }

        Rectangle {
            Layout.preferredWidth: 2
            Layout.fillHeight: true

            color: ui.theme.buttonColor
        }

        SelectedInstrumentsView {
            id: selectedInstrumentsView

            Layout.fillWidth: true
            Layout.fillHeight: true

            instruments: instrumentsModel.selectedInstruments
            instrumentOrderTypes: instrumentsModel.instrumentOrderTypes

            onUnselectInstrumentRequested: {
                instrumentsModel.unselectInstrument(id)
            }

            onOrderChanged: {
                instrumentsModel.selectOrderType(id)
            }
        }

        Rectangle {
            Layout.preferredWidth: 2
            Layout.fillHeight: true

            color: ui.theme.buttonColor
        }

        Column {
            Layout.preferredWidth: 30
            anchors.verticalCenter: parent.verticalCenter

            spacing: 4

            FlatButton {
                enabled: selectedInstrumentsView.canLiftInstrument
                icon: IconCode.ARROW_UP

                onClicked: {
                    instrumentsModel.swapSelectedInstruments(selectedInstrumentsView.currentInstrumentIndex,
                                                             selectedInstrumentsView.currentInstrumentIndex - 1)
                    selectedInstrumentsView.currentInstrumentIndex--
                }
            }

            FlatButton {
                enabled: selectedInstrumentsView.canLowerInstrument
                icon: IconCode.ARROW_DOWN

                onClicked: {
                    instrumentsModel.swapSelectedInstruments(selectedInstrumentsView.currentInstrumentIndex,
                                                             selectedInstrumentsView.currentInstrumentIndex + 1)
                    selectedInstrumentsView.currentInstrumentIndex++
                }
            }
        }
    }
}
