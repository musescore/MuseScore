import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

StyledPopup {
    id: root

    height: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    function load(staff) {
        settingsModel.load(staff)
    }

    StaffSettingsModel {
        id: settingsModel
    }

    Column {
        id: contentColumn

        anchors.fill: parent

        spacing: 8

        StyledTextLabel {
            text: qsTrc("instruments", "Staff type")
        }

        StyledComboBox {
            width: parent.width

            textRoleName: "text"
            valueRoleName: "value"

            model: {
                var types = settingsModel.allStaffTypes()
                var result = []

                for (var i = 0; i < types.length; ++i) {
                    result.push({ text: types[i].title, value: types[i].value })
                }

                return result
            }

            onValueChanged: {
                settingsModel.setStaffType(value)
            }
        }

        SeparatorLine {}

        StyledTextLabel {
            text: qsTrc("instruments", "Voices visible in the score")
        }

        ListView {
            height: 30
            width: parent.width

            spacing: 32
            orientation: ListView.Horizontal

            model: settingsModel.voices

            delegate: CheckBox {
                width: 30

                text: modelData.title
                checked: modelData.visible

                onClicked: {
                    settingsModel.setVoiceVisible(model.index, !checked)
                }
            }
        }

        SeparatorLine {}

        CheckBox {
            text: qsTrc("instruments", "Small staff")

            checked: settingsModel.isSmallStaff

            onClicked: {
                settingsModel.setIsSmallStaff(!checked)
            }
        }

        CheckBox {
            text: qsTrc("instruments", "Hide all measures that do not contain notation (cutaway)")
            wrapMode: Text.WordWrap

            checked: settingsModel.cutawayEnabled

            onClicked: {
                settingsModel.setCutawayEnabled(!checked)
            }
        }

        SeparatorLine {}

        Item {
            height: 110
            width: parent.width

            FlatButton {
                anchors.top: parent.top
                anchors.topMargin: 16
                anchors.bottom: noteLabel.top
                anchors.bottomMargin: 16

                width: parent.width

                text: qsTrc("instruments", "Create a linked staff")

                onClicked: {
                    settingsModel.createLinkedStaff()
                }
            }

            StyledTextLabel {
                id: noteLabel

                anchors.bottom: parent.bottom
                anchors.bottomMargin: 16

                width: parent.width
                wrapMode: Text.WordWrap

                text: qsTrc("instruments", "Note: linked staves contain identical information.")
            }
        }
    }
}
