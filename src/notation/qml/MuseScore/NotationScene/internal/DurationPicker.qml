import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls 1.3

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Row {
    id: root

    spacing: 4

    property int buttonWidthBig: 35;
    property int buttonWidthSmall: 20;

    property string currentValue: "q";

    signal valueChanged(var changedValue)

    function setValue(s) {
        noteButton.setOptionFromNote(s[0]);
        dotList.currentValue = s.substr(1);

        root.currentValue = s;
    }

    FlatButton {
        id: noteButton

        icon: IconCode.NOTE_QUARTER

        width: buttonWidthBig
        height: root.height

        accentButton: true

        property string currentValue: "q"

        function setOptionFromNote(note) {
            for(let optionItem in buttonList) {
                let option = buttonList[optionItem];
                if(option.value === note) {
                    noteButton.icon = option.icon;
                    noteButton.currentValue = option.value;
                    return;
                }
            }
        }

        readonly property var buttonList: [
            {id: "0", value: "d", icon: IconCode.NOTE_WHOLE_DOUBLE_VERTICAL, title: qsTrc("global", "Double Whole")},
            {id: "1", value: "w", icon: IconCode.NOTE_WHOLE_VERTICAL, title: qsTrc("global", "Whole")},
            {id: "2", value: "h", icon: IconCode.NOTE_HALF, title: qsTrc("global", "Half")},
            {id: "3", value: "q", icon: IconCode.NOTE_QUARTER, title: qsTrc("global", "Quarter")},
            {id: "4", value: "e", icon: IconCode.NOTE_8TH, title: qsTrc("global", "Eighth")},
            {id: "5", value: "s", icon: IconCode.NOTE_16TH, title: qsTrc("global", "Sixteenth")},
            {id: "6", value: "t", icon: IconCode.NOTE_32ND, title: qsTrc("global", "Thirty-Second")},
        ]

        onClicked: {
            menuLoader.toggleOpened(noteButton.buttonList);
        }

        StyledMenuLoader {
            id: menuLoader
            onHandleMenuItem: {
                noteButton.icon = noteButton.buttonList[parseInt(itemId)]["icon"];
                console.log(parseInt(noteButton.icon));

                noteButton.currentValue =noteButton.buttonList[parseInt(itemId)]["value"];

                root.currentValue = noteButton.currentValue + dotList.currentValue;
                valueChanged(root.currentValue);
            }
        }

    }

    RadioButtonGroup {
        id: dotList

        property string currentValue: ""

        width: buttonWidthSmall * 2

        model: [
            { textRole: "."},
            { textRole: ".."},
        ]

        delegate: FlatRadioButton {
            ButtonGroup.group: dotList.radioButtonGroup

            height: root.height

            checked: dotList.currentValue === modelData["textRole"]

            onClicked: {
                if (dotList.currentValue !== modelData["textRole"])
                    dotList.currentValue = modelData["textRole"]
                else
                    dotList.currentValue = ""

                root.currentValue = noteButton.currentValue + dotList.currentValue;
                valueChanged(root.currentValue);
            }

            StyledTextLabel {
                text: modelData["textRole"]
            }
        }
    }

}
