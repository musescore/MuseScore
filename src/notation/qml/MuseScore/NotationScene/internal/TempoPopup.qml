import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls 1.3

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0


Column {
    id: root

    property QtObject model;

    property string currentEquation;
    property int currentParantheses;

    property int startupTab: 0;

    width: 200

    spacing: 10

    function setEquation(left, right) {
        currentEquation = right ? left + " = " + right : left;
        addParantheses();
    }

    function addParantheses() {
        switch(paranthesesList.currentValue) {
        case 0:
            root.model.equation.value = root.currentEquation;
            break;
        case 1:
            root.model.equation.value = "(" + root.currentEquation + ")";
            break;
        case 2:
            root.model.equation.value = "[" + root.currentEquation + "]";
            break;
        }
    }

    function parseEquation() {
        let eq = root.model.equation.value;
        let l = eq.length;

        if(eq[0] === "(" && eq[l - 1] === ")") {
            root.currentEquation = eq.substr(1, l - 2);
            root.currentParantheses = 1;
        }
        else if(eq[0] === "[" && eq[l - 1] === "]") {
            root.currentEquation = eq.substr(1, l - 2);
            root.currentParantheses = 2;
        } else {
            root.currentEquation = eq;
            root.currentParantheses = 0;
        }

        console.log(root.model.equation.value);
        console.log(root.currentEquation);
    }

    function setActiveTab() {
        if(/[d,w,h,q,e,s,t].{0,2} = [0-9]+/.test(root.currentEquation)) {
            root.startupTab = 0;
        } else if(/[d,w,h,q,e,s,t].{0,2} = [d,w,h,q,e,s,t].{0,2}/.test(root.currentEquation)) {
            root.startupTab  = 1;
        } else {
            root.startupTab  = 2;
        }

        tabPanel.currentIndex = root.startupTab;
    }

    TabPanel {
        id: tabPanel

        height: 90

        Tab {
            id: tempoTab

            title: "Tempo"

            Row {
                id: tempoRow

                onVisibleChanged: {
                    if(tabPanel.currentIndex === 0) {
                        root.setEquation(tempoDuration.currentValue, tempoBPM.currentValue.toString());
                     }
                }

                Component.onCompleted: {
                    if(root.startupTab === 0) {
                        tempoDuration.setValue(root.currentEquation.split("=")[0].trim());
                        tempoBPM.currentValue = root.currentEquation.split("=")[1].trim();
                    } else {
                        tempoBPM.currentValue = root.model.tempo.value.toString();
                        root.setEquation(tempoDuration.currentValue, tempoBPM.currentValue.toString());
                    }
                }

                DurationPicker {
                    id: tempoDuration

                    height: 35

                    anchors.verticalCenter: parent.verticalCenter

                    onValueChanged: {
                        root.setEquation(tempoDuration.currentValue, tempoBPM.currentValue.toString());
                    }

                }

                StyledTextLabel {
                    text: "="

                    anchors.verticalCenter: parent.verticalCenter

                    width: root.width - tempoDuration.width - tempoBPM.width

                    horizontalAlignment: Qt.AlignHCenter
                }

                IncrementalPropertyControl {
                    id: tempoBPM

                    iconMode: iconModeEnum.hidden

                    anchors.verticalCenter: parent.verticalCenter

                    width: tempoDuration.width

                    maxValue: 999
                    minValue: 0
                    step:1

                    decimals: 0

                    onValueEdited: {
                        currentValue = newValue;
                        root.setEquation(tempoDuration.currentValue, tempoBPM.currentValue.toString());
                    }
                }
            }

        }

        Tab {
            id: equationTab

            title: "Equation"

            Row {
                id: equationRow

                onVisibleChanged: {
                    if(tabPanel.currentIndex === 1) {
                        root.setEquation(equationLeft.currentValue, equationRight.currentValue);
                    }
                }

                Component.onCompleted: {
                    if(root.startupTab === 1) {
                        equationLeft.setValue(root.currentEquation.split("=")[0].trim());
                        equationRight.setValue(root.currentEquation.split("=")[1].trim());
                    } else {
                        root.setEquation(equationLeft.currentValue, equationRight.currentValue);
                    }
                }

                DurationPicker {
                    id: equationLeft

                    height: 35

                    anchors.verticalCenter: parent.verticalCenter

                    onValueChanged: {
                        root.setEquation(equationLeft.currentValue, equationRight.currentValue);
                    }
                }

                StyledTextLabel {
                    text: "="

                    anchors.verticalCenter: parent.verticalCenter

                    width: root.width - equationLeft.width - equationRight.width

                    horizontalAlignment: Qt.AlignHCenter
                }

                DurationPicker {
                    id: equationRight

                    height: 35

                    anchors.verticalCenter: parent.verticalCenter

                    onValueChanged: {
                        root.setEquation(equationLeft.currentValue, equationRight.currentValue);
                    }
                }

            }

        }

        Tab {
            id: codeTab

            title: "Code"

            Column {
                spacing: 8

                onVisibleChanged: {
                    if(tabPanel.currentIndex === 2) {
                        root.setEquation(codeInput.currentText);
                    }
                }

                Component.onCompleted: {
                    root.setEquation(codeInput.currentText);
                }

                StyledTextLabel {
                    text: qsTrc("inspector", "Enter Code") + " (eg: e = 100)"
                }

                TextInputField {
                    id: codeInput

                    height: 30
                    width: root.width

                    onCurrentTextEdited: {
                        root.setEquation(newTextValue);
                    }
                }
            }

        }
    }

    SeparatorLine {
    }

    StyledTextLabel {
        text: qsTrc("Inspector", "Parentheses")
    }

    RadioButtonGroup {
        id: paranthesesList

        property int currentValue: root.currentParantheses

        height: 30
        width: root.width

        model: [
            { textRole: "None", valueRole: 0 },
            { textRole: "( )", valueRole: 1 },
            { textRole: "[ ]", valueRole: 2 }
        ]

        delegate: FlatRadioButton {
            ButtonGroup.group: paranthesesList.radioButtonGroup

            checked: paranthesesList.currentValue === modelData["valueRole"]

            onToggled: {
                paranthesesList.currentValue = modelData["valueRole"]
                root.addParantheses();
            }

            StyledTextLabel {
                text: modelData["textRole"]
            }
        }
    }

    SeparatorLine {
    }

    CheckBox {
        width: root.width

        checked: root.model.isEquationVisible.value;

        text: qsTrc("inspector", "Visibility")
        onClicked: {
            checked = !checked
            root.model.isEquationVisible.value = checked;
        }
    }

}
