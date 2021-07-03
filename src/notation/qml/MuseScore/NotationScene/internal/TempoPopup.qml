import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls 1.3

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0


ColumnLayout {
    id: root

    property QtObject model;

    property string currentEquation;
    property int currentParantheses;

    property int startupTab: -1;

    width: 220

    spacing: 12

    property NavigationSection navigationSection: NavigationSection {
        id: navSec
        name: root.objectName != "" ? root.objectName : "StyledDialogView"
        type: NavigationSection.Exclusive
        enabled: true
        order: 1
    }

    function setEquation(left, right) {
        currentEquation = right ? left + " = " + right : left;
        addParantheses();
    }

    function addParantheses() {
        if(!root.model)
            return;

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
    }

    function setActiveTab() {
        root.startupTab = -1;

        if(/[dwhqest].{0,2} = [0-9]+/.test(root.currentEquation)) {
            root.startupTab = 0;
        } else if(/[dwhqest].{0,2} = [dwhqest].{0,2}/.test(root.currentEquation)) {
            root.startupTab  = 1;
        } else {
            root.startupTab  = 2;
        }

        if(root.startupTab != -1)
            bar.currentIndex = root.startupTab;

        tempoRow.startup = true;
        equationRow.startup = true;

    }

    TabBar {
        id: bar

        Layout.alignment: Qt.AlignHCenter
        Layout.leftMargin: 10

        spacing: 0

        StyledTabButton {
            id: tempoBtm
            text: qsTrc("inspector", "Tempo")
            sideMargin: 15
            isCurrent: bar.currentIndex === 0
        }

        StyledTabButton {
            text: qsTrc("inspector", "Equation")
            sideMargin: 15
            isCurrent: bar.currentIndex === 1
        }

        StyledTabButton {
            text: qsTrc("inspector", "Code")
            sideMargin: 15
            isCurrent: bar.currentIndex === 2
        }
    }

    StackLayout {
        id: pagesStack

        Layout.topMargin: 12

        Layout.minimumHeight: 35

        currentIndex: bar.currentIndex

        Item {
            Row {
                id: tempoRow

                property bool startup: true;

                function init() {
                    if(!root.model)
                        return;

                    if(root.startupTab === 0 && tempoRow.startup) {
                        tempoDuration.setValue(root.currentEquation.split("=")[0].trim());
                        tempoBPM.currentValue = parseFloat(root.currentEquation.split("=")[1].trim());
                    } else {
                        root.setEquation(tempoDuration.currentValue, tempoBPM.currentValue.toString());
                    }

                    tempoRow.startup = false;
                }

                onVisibleChanged: {
                    if(bar.currentIndex === 0) {
                        tempoRow.init();
                     }
                }

                Component.onCompleted: {
                    if(root.model) {
                        tempoRow.init();
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

                    iconMode: IncrementalPropertyControl.Hidden

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

                    onActiveFocusChanged: {
                        root.model.hideTextCursor.value = focus;
                    }
                }
            }

        }

        Item {
            Row {
                id: equationRow

                property bool startup: true;

                function init() {
                    if(!root.model)
                        return;

                    if(root.startupTab === 1 && equationRow.startup) {
                        equationLeft.setValue(root.currentEquation.split("=")[0].trim());
                        equationRight.setValue(root.currentEquation.split("=")[1].trim());
                    } else {
                        root.setEquation(equationLeft.currentValue, equationRight.currentValue);
                    }

                    equationRow.startup = false;

                    root.model.hideTextCursor.value = false;
                }

                onVisibleChanged: {
                    if(bar.currentIndex === 1) {
                        equationRow.init();
                    }
                }

                Component.onCompleted: {
                    equationRow.init();
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

        Item {
            Column {
                id: codeColumn

                spacing: 8

                onVisibleChanged: {
                    if(bar.currentIndex === 2) {
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

                    onActiveFocusChanged: {
                        root.model.hideTextCursor.value = activeFocus;
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
        id: visibilityToggle

        width: root.width

        checked: root.model ? root.model.isEquationVisible.value : false;

        text: qsTrc("inspector", "Visibility")
        onClicked: {
            checked = !checked
            root.model.isEquationVisible.value = checked;
        }
    }
}
