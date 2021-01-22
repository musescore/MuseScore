import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.12

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
        groupSelected(root.groups[0].id)
    }

    function setFamily(family) {
        familiesBox.currentIndex = familiesBox.indexOfValue(family)
    }

    function focusGroup(group) {
        for (var i in root.groups) {
            if (root.groups[i].id === group) {
                privateProperties.currentGroupIndex = i
                groupsView.positionViewAtIndex(groupsView.currentGroupIndex, ListView.Beginning)
                return
            }
        }
    }

    StyledTextLabel {
        id: familyLabel

        anchors.top: parent.top
        anchors.left: parent.left

        font: ui.theme.bodyBoldFont
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
        anchors.topMargin: 8
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        model: groups

        boundsBehavior: ListView.StopAtBounds
        clip: true

        ScrollBar.vertical: StyledScrollBar {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
        }

        delegate: ListItemBlank {
            isSelected: privateProperties.currentGroupIndex === index

            StyledTextLabel {
                anchors.fill: parent
                anchors.leftMargin: 12

                font: ui.theme.bodyBoldFont
                horizontalAlignment: Text.AlignLeft
                text: modelData.name
            }

            onClicked: {
                privateProperties.currentGroupIndex = index
                groupSelected(modelData.id)
            }
        }
    }
}
