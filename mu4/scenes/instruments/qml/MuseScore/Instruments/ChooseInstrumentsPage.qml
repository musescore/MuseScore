import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

Rectangle {
    id: root

    color: ui.theme.backgroundColor

    property int currentGroupIndex: -1
    property int currentInstrumentIndex: -1

    InstrumentListModel {
        id: instrumentsModel
    }

    Component.onCompleted: {
        instrumentsModel.load()
        familyListView.selectFirstGroup()
    }

    RowLayout {

        anchors.fill: parent
        anchors.margins: 16

        spacing: 16

        FamilyListView {
            id: familyListView

            Layout.preferredWidth: root.width / 4
            Layout.fillHeight: true

            families: instrumentsModel.families
            groups: instrumentsModel.groups

            onFamilySelected: {
                instrumentsModel.selectFamily(familyId)
            }

            onGroupSelected: {
                instrumentsModel.selectGroup(groupId)
            }

            Connections {
                target: instrumentsModel

                onSelectedFamilyChanged: {
                    familyListView.setFamily(family)
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: 2
            Layout.fillHeight: true

            color: ui.theme.buttonColor
        }

        InstrumentsListView {
            Layout.preferredWidth: root.width / 4
            Layout.fillHeight: true

            instruments: instrumentsModel.instruments

            onSearchChanged: {
                currentInstrumentIndex = -1
                instrumentsModel.setSearchText(search)
                familyListView.selectFirstGroup()
            }
        }

        Rectangle {
            Layout.preferredWidth: 2
            Layout.fillHeight: true

            color: ui.theme.buttonColor
        }

        FlatButton {
            Layout.preferredWidth: 30

            icon: IconCode.ARROW_RIGHT
        }

        Rectangle {
            Layout.preferredWidth: 2
            Layout.fillHeight: true

            color: ui.theme.buttonColor
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: "blue"
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
                icon: IconCode.ARROW_UP
            }

            FlatButton {
                icon: IconCode.ARROW_DOWN
            }
        }
    }
}
