import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

Item {
    id: root

    property var families: null
    property var groups: null

    QtObject {
        id: privateProperties

        property int currentGroupIndex: -1
    }

    signal familySelected(string familyId)
    signal groupSelected(string groupId)

    function selectFirstGroup() {
        if (groupsView.count == 0) {
            privateProperties.currentGroupIndex = -1
            return
        }

        privateProperties.currentGroupIndex = 0
        groupSelected(groups[0].id)
    }

    function setFamily(family) {
        familiesBox.currentIndex = familiesBox.indexOfValue(family)
    }

    StyledTextLabel {
        id: familyLabel

        anchors.top: parent.top
        anchors.left: parent.left

        font.bold: true
        text: qsTrc("instruments", "Family")
    }

    StyledComboBox {
        id: familiesBox

        anchors.top: familyLabel.bottom
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right

        textRoleName: "text"
        valueRoleName: "value"

        model: {
            var resultList = []

            var _families = families

            for (var i = 0; i < _families.length; ++i) {
                resultList.push({"text" : _families[i].name, "value" : _families[i].id})
            }

            return resultList
        }

        onValueChanged: {
            familySelected(value)
        }
    }

    ListView {
        id: groupsView

        anchors.top: familiesBox.bottom
        anchors.topMargin: 20
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        model: groups

        boundsBehavior: ListView.StopAtBounds
        clip: true

        delegate: Item {
            width: parent.width
            height: 40

            Rectangle {
                anchors.fill: parent

                color: privateProperties.currentGroupIndex === index ? ui.theme.accentColor : ui.theme.backgroundPrimaryColor
                opacity: privateProperties.currentGroupIndex === index ? 0.3 : 1
            }

            StyledTextLabel {
                anchors.fill: parent
                anchors.leftMargin: 12

                font.bold: true
                horizontalAlignment: Text.AlignLeft
                text: modelData.name
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    privateProperties.currentGroupIndex = index
                    groupSelected(modelData.id)
                }
            }
        }
    }
}
