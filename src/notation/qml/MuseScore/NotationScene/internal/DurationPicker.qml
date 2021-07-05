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
            for(let optionItem in privateProperties.items) {
                let option = privateProperties.items[optionItem];
                if(option.value === note) {
                    noteButton.icon = option.icon;
                    noteButton.currentValue = option.value;
                    return;
                }
            }
        }

        QtObject {
            id: privateProperties

            readonly property var items: [
                {code: "0", value: "d", icon: IconCode.NOTE_WHOLE_DOUBLE, title: qsTrc("global", "Double Whole")},
                {code: "1", value: "w", icon: IconCode.NOTE_WHOLE, title: qsTrc("global", "Whole")},
                {code: "2", value: "h", icon: IconCode.NOTE_HALF, title: qsTrc("global", "Half")},
                {code: "3", value: "q", icon: IconCode.NOTE_QUARTER, title: qsTrc("global", "Quarter")},
                {code: "4", value: "e", icon: IconCode.NOTE_8TH, title: qsTrc("global", "Eighth")},
                {code: "5", value: "s", icon: IconCode.NOTE_16TH, title: qsTrc("global", "Sixteenth")},
                {code: "6", value: "t", icon: IconCode.NOTE_32TH, title: qsTrc("global", "Thirty-Second")},
            ]
        }

        onClicked: {
            menuLoader.toggleOpened(privateProperties.items)
        }

        StyledMenuLoader {
            id: menuLoader
            onHandleAction: {
                noteButton.icon = privateProperties.items[parseInt(actionCode)]["icon"];

                noteButton.currentValue = privateProperties.items[parseInt(actionCode)]["value"];

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
