import QtQuick 2.0
import QtQuick.Controls 2.0
import MuseScore.Inspectors 3.3
import "../common"
import "internal"

FocusableItem {
    id: root

    property QtObject model: null
    property QtObject beamModesModel: model ? model.beamModesModel : null

    implicitHeight: contentColumn.height
    width: parent.width

    enabled: model ? !model.isEmpty : false

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        StyledTextLabel {
            id: beamTypesLabel

            text: qsTr("Beam types")
        }

        BeamTypesGrid {
            id: beamTypesGridView

            beamTypesModel: beamModesModel ? beamModesModel.modeListModel : null
            isIndeterminate: beamModesModel ? beamModesModel.mode.isUndefined : false
        }

        Column {
            spacing: 16

            height: implicitHeight
            width: parent.width

            SeparatorLine {
                anchors.margins: -10
                visible: featheringControlsColumn.visible
            }

            Column {
                id: featheringControlsColumn

                spacing: 12

                height: implicitHeight
                width: parent.width

                enabled: root.beamModesModel ? root.beamModesModel.isFeatheringAvailable.isUndefined || root.beamModesModel.isFeatheringAvailable.value
                                             : false

                StyledTextLabel {
                    id: stedDirectionLabel

                    text: qsTr("Feathered beams")
                }

                RadioButtonGroup {
                    id: radioButtonList

                    height: 30
                    width: parent.width

                    model: [
                        { iconRole: "qrc:/resources/icons/beams/beam_feathered_none.svg", typeRole: Beam.FEATHERING_NONE },
                        { iconRole: "qrc:/resources/icons/beams/beam_feathered_left.svg", typeRole: Beam.FEATHERING_LEFT },
                        { iconRole: "qrc:/resources/icons/beams/beam_feathered_right.svg", typeRole: Beam.FEATHERING_RIGHT }
                    ]

                    delegate: FlatRadioButton {

                        ButtonGroup.group: radioButtonList.radioButtonGroup

                        checked: root.beamModesModel &&
                                 !(root.model.featheringHeightLeft.isUndefined
                                 ||root.model.featheringHeightRight.isUndefined) ? root.model.featheringMode === modelData["typeRole"]
                                                                                 : false

                        onToggled: {
                            root.model.featheringMode = modelData["typeRole"]
                        }

                        StyledIcon {
                            anchors.centerIn: parent

                            height: 16
                            width: 16

                            icon: modelData["iconRole"]
                        }
                    }
                }

                StyledTextLabel {
                    anchors.left: parent.left

                    text: qsTr("Feathering height")
                }

                Item {
                    height: childrenRect.height
                    width: parent.width

                    IncrementalPropertyControl {
                        anchors.left: parent.left
                        anchors.right: parent.horizontalCenter
                        anchors.rightMargin: 4

                        icon: "qrc:/resources/icons/feathering_height_left.svg"
                        enabled: beamModesModel ? beamModesModel.isFeatheringAvailable : false
                        isIndeterminate: model ? model.featheringHeightLeft.isUndefined : false
                        currentValue: model ? model.featheringHeightLeft.value : 0
                        maxValue: 4
                        minValue: 0
                        step: 0.1

                        onValueEdited: { model.featheringHeightLeft.value = newValue }
                    }

                    IncrementalPropertyControl {
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 4
                        anchors.right: parent.right

                        icon: "qrc:/resources/icons/feathering_height_right.svg"
                        enabled: beamModesModel ? beamModesModel.isFeatheringAvailable : false
                        isIndeterminate: model ? model.featheringHeightRight.isUndefined : false
                        iconMode: iconModeEnum.right
                        currentValue: model ? model.featheringHeightRight.value : 0
                        maxValue: 4
                        minValue: 0
                        step: 0.1

                        onValueEdited: { model.featheringHeightRight.value = newValue }
                    }
                }
            }

            SeparatorLine {
                anchors.margins: -10
                visible: featheringControlsColumn.visible
            }

            FlatButton {
                text: qsTr("Forse horizontal")

                onClicked: {
                    if (!model)
                        return

                    model.forceHorizontal()
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

                    StyledTextLabel {
                        anchors.left: parent.left

                        text: qsTr("Beam height")
                    }

                    Item {
                        height: childrenRect.height
                        width: parent.width

                        IncrementalPropertyControl {
                            id: beamHightLeftControl

                            anchors.left: parent.left
                            anchors.right: lockButton.left
                            anchors.rightMargin: 6

                            icon: "qrc:/resources/icons/beam_hight_left.svg"
                            isIndeterminate: model ? model.beamVectorX.isUndefined : false
                            currentValue: model ? model.beamVectorX.value : 0

                            onValueEdited: { model.beamVectorX.value = newValue }
                        }

                        FlatToogleButton {
                            id: lockButton

                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: beamHightLeftControl.verticalCenter

                            iconPixelSize: 12

                            icon: "qrc:/resources/icons/beams/beam_pos_lock.svg"

                            pressed: model ? model.isBeamHeightLocked : false

                            onToggled: {
                                model.isBeamHeightLocked = !model.isBeamHeightLocked
                            }
                        }

                        IncrementalPropertyControl {
                            anchors.left: lockButton.right
                            anchors.leftMargin: 6
                            anchors.right: parent.right

                            icon: "qrc:/resources/icons/beam_hight_right.svg"
                            iconMode: iconModeEnum.right
                            isIndeterminate: model ? model.beamVectorY.isUndefined : false
                            currentValue: model ? model.beamVectorY.value : 0

                            onValueEdited: { model.beamVectorY.value = newValue }
                        }
                    }
                }
            }
        }
    }
}
