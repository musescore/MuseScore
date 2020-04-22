import QtQuick 2.0
import QtQuick.Controls 2.0
import MuseScore.Inspectors 3.3
import "../../common"


FocusableItem {
    id: root

    //@note Current design assumes that stems and hooks should be represented at the same tab,
    //      but semantically it's different things, so they should have different models
    property QtObject stemModel: null
    property QtObject hookModel: null
    property QtObject beamModel: null

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        height: implicitHeight
        width: root.width

        spacing: 16

        CheckBox {
            isIndeterminate: stemModel && beamModel ? stemModel.isStemHidden.isUndefined || beamModel.isBeamHidden.isUndefined : false
            checked: stemModel && !isIndeterminate && beamModel ? stemModel.isStemHidden.value && beamModel.isBeamHidden.value : false
            text: qsTr("Hide stem (also hides beam)")

            onClicked: {
                var isHidden = !checked
                stemModel.isStemHidden.value = isHidden
                beamModel.isBeamHidden.value = isHidden
            }
        }

        StyledTextLabel {
            id: stedDirectionLabel

            text: qsTr("Stem direction")
        }

        RadioButtonGroup {
            id: radioButtonList

            height: 30
            width: parent.width

            model: [
                { iconRole: "qrc:/resources/icons/orientation_auto.svg", typeRole: Stem.AUTO },
                { iconRole: "qrc:/resources/icons/orientation_arrow_down.svg", typeRole: Stem.DOWN },
                { iconRole: "qrc:/resources/icons/orientation_arrow_up.svg", typeRole: Stem.UP }
            ]

            delegate: FlatRadioButton {

                ButtonGroup.group: radioButtonList.radioButtonGroup

                checked: root.stemModel && !root.stemModel.stemDirection.isUndefined ? root.stemModel.stemDirection.value === modelData["typeRole"]
                                                                                     : false

                onToggled: {
                    root.stemModel.stemDirection.value = modelData["typeRole"]
                }

                StyledIcon {
                    anchors.centerIn: parent

                    height: 16
                    width: 16

                    icon: modelData["iconRole"]
                }
            }
        }

        ExpandableBlank {
            isExpanded: false

            title: isExpanded ? qsTr("Show less") : qsTr("Show more")

            width: parent.width

            contentItemComponent: Column {
                height: implicitHeight
                width: root.width

                spacing: 16

                Item {
                    height: childrenRect.height
                    width: parent.width

                    Column {
                        anchors.left: parent.left
                        anchors.right: parent.horizontalCenter
                        anchors.rightMargin: 2

                        spacing: 8

                        StyledTextLabel {
                            text: qsTr("Thickness")
                        }

                        IncrementalPropertyControl {
                            enabled: stemModel ? !stemModel.isEmpty : false
                            isIndeterminate: stemModel ? stemModel.thickness.isUndefined : false
                            currentValue: stemModel ? stemModel.thickness.value : 0
                            iconMode: iconModeEnum.hidden

                            step: 0.01

                            onValueEdited: { stemModel.thickness.value = newValue }
                        }
                    }

                    Column {
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 2
                        anchors.right: parent.right

                        spacing: 8

                        StyledTextLabel {
                            text: qsTr("Length")
                        }

                        IncrementalPropertyControl {
                            enabled: stemModel ? !stemModel.isEmpty : false
                            isIndeterminate: stemModel ? stemModel.length.isUndefined : false
                            currentValue: stemModel ? stemModel.length.value : 0
                            iconMode: iconModeEnum.hidden

                            onValueEdited: { stemModel.length.value = newValue }
                        }
                    }
                }

                Column {
                    spacing: 8

                    height: implicitHeight
                    width: parent.width

                    StyledTextLabel {
                        anchors.left: parent.left

                        text: qsTr("Stem offset")
                    }

                    Item {
                        height: childrenRect.height
                        width: parent.width

                        IncrementalPropertyControl {
                            anchors.left: parent.left
                            anchors.right: parent.horizontalCenter
                            anchors.rightMargin: 4

                            icon: "qrc:/resources/icons/horizontal_adjustment.svg"
                            enabled: stemModel ? !stemModel.isEmpty : false
                            isIndeterminate: stemModel ? stemModel.horizontalOffset.isUndefined : false
                            currentValue: stemModel ? stemModel.horizontalOffset.value : 0

                            onValueEdited: { stemModel.horizontalOffset.value = newValue }
                        }

                        IncrementalPropertyControl {
                            anchors.left: parent.horizontalCenter
                            anchors.leftMargin: 4
                            anchors.right: parent.right

                            icon: "qrc:/resources/icons/vertical_adjustment.svg"
                            enabled: stemModel ? !stemModel.isEmpty : false
                            isIndeterminate: stemModel ? stemModel.verticalOffset.isUndefined : false
                            currentValue: stemModel ? stemModel.verticalOffset.value : 0

                            onValueEdited: { stemModel.verticalOffset.value = newValue }
                        }
                    }
                }

                Column {
                    spacing: 8

                    height: childrenRect.height
                    width: parent.width

                    StyledTextLabel {
                        anchors.left: parent.left

                        text: qsTr("Flag offset")
                    }

                    Item {
                        height: childrenRect.height
                        width: parent.width

                        IncrementalPropertyControl {
                            anchors.left: parent.left
                            anchors.right: parent.horizontalCenter
                            anchors.rightMargin: 4

                            enabled: hookModel ? !hookModel.isEmpty : false
                            isIndeterminate: hookModel ? hookModel.horizontalOffset.isUndefined : false
                            icon: "qrc:/resources/icons/horizontal_adjustment.svg"
                            currentValue: hookModel ? hookModel.horizontalOffset.value : 0.00

                            onValueEdited: { hookModel.horizontalOffset.value = newValue }
                        }

                        IncrementalPropertyControl {
                            anchors.left: parent.horizontalCenter
                            anchors.leftMargin: 4
                            anchors.right: parent.right

                            enabled: hookModel ? !hookModel.isEmpty : false
                            isIndeterminate: hookModel ? hookModel.verticalOffset.isUndefined : false
                            icon: "qrc:/resources/icons/vertical_adjustment.svg"
                            currentValue: hookModel ? hookModel.verticalOffset.value : 0.00

                            onValueEdited: { hookModel.verticalOffset.value = newValue }
                        }
                    }
                }
            }
        }
    }
}
