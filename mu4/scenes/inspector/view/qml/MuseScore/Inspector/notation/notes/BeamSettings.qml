import QtQuick 2.0
import QtQuick.Controls 2.0
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"
import "internal"

FocusableItem {
    id: root

    property QtObject model: null
    property QtObject beamModesModel: model ? model.beamModesModel : null

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        InspectorPropertyView {

            titleText: qsTr("Beam types")
            propertyItem: beamModesModel ? beamModesModel.mode : null

            BeamTypesGrid {
                id: beamTypesGridView

                beamTypesModel: beamModesModel ? beamModesModel.modeListModel : null
                enabled: beamModesModel ? !beamModesModel.isEmpty : false
            }
        }

        Column {
            spacing: 16

            height: implicitHeight
            width: parent.width

            enabled: model ? !model.isEmpty : false

            SeparatorLine {
                anchors.margins: -10
                visible: featheringControlsColumn.visible
            }

            Column {
                id: featheringControlsColumn

                spacing: 12

                height: implicitHeight
                width: parent.width

                visible: root.beamModesModel ? root.beamModesModel.isFeatheringAvailable.isUndefined || root.beamModesModel.isFeatheringAvailable.value
                                             : false

                StyledTextLabel {
                    text: qsTr("Feathered beams")
                }

                RadioButtonGroup {
                    id: radioButtonList

                    height: 30
                    width: parent.width

                    model: [
                        { iconRole: IconCode.NONE, typeRole: Beam.FEATHERING_NONE },
                        { iconRole: IconCode.FEATHERED_LEFT_HEIGHT, typeRole: Beam.FEATHERING_LEFT },
                        { iconRole: IconCode.FEATHERED_RIGHT_HEIGHT, typeRole: Beam.FEATHERING_RIGHT }
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

                        StyledIconLabel {
                            iconCode: modelData["iconRole"]
                        }
                    }
                }

                Item {
                    height: childrenRect.height
                    width: parent.width

                    InspectorPropertyView {
                        anchors.left: parent.left
                        anchors.right: parent.horizontalCenter
                        anchors.rightMargin: 4

                        titleText: qsTr("Feathering left")
                        propertyItem: model ? model.featheringHeightLeft : null

                        IncrementalPropertyControl {
                            icon: IconCode.FEATHERED_LEFT_HEIGHT
                            enabled: beamModesModel ? beamModesModel.isFeatheringAvailable : false
                            isIndeterminate: model ? model.featheringHeightLeft.isUndefined : false
                            currentValue: model ? model.featheringHeightLeft.value : 0
                            maxValue: 4
                            minValue: 0
                            step: 0.1

                            onValueEdited: { model.featheringHeightLeft.value = newValue }
                        }
                    }

                    InspectorPropertyView {
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 4
                        anchors.right: parent.right

                        titleText: qsTr("Feathering right")
                        propertyItem: model ? model.featheringHeightRight : null

                        IncrementalPropertyControl {
                            icon: IconCode.FEATHERED_RIGHT_HEIGHT
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
            }

            SeparatorLine {
                anchors.margins: -10
                visible: featheringControlsColumn.visible
            }

            FlatButton {
                width: parent.width

                text: qsTr("Force horizontal")

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

                    InspectorPropertyView {

                        titleText: qsTr("Beam height")
                        propertyItem: model ? model.beamVectorX : null

                        Item {
                            height: childrenRect.height
                            width: parent.width

                            IncrementalPropertyControl {
                                id: beamHightLeftControl

                                anchors.left: parent.left
                                anchors.right: lockButton.left
                                anchors.rightMargin: 6

                                icon: IconCode.BEAM_RIGHT_Y_POSITION
                                isIndeterminate: model ? model.beamVectorX.isUndefined : false
                                currentValue: model ? model.beamVectorX.value : 0

                                onValueEdited: { model.beamVectorX.value = newValue }
                            }

                            FlatToogleButton {
                                id: lockButton

                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.verticalCenter: beamHightLeftControl.verticalCenter

                                height: 20
                                width: 20

                                icon: checked ? IconCode.LOCK_CLOSED : IconCode.LOCK_OPEN

                                checked: model ? model.isBeamHeightLocked : false

                                onToggled: {
                                    model.isBeamHeightLocked = !model.isBeamHeightLocked
                                }
                            }

                            IncrementalPropertyControl {
                                anchors.left: lockButton.right
                                anchors.leftMargin: 6
                                anchors.right: parent.right

                                icon: IconCode.BEAM_LEFT_Y_POSITION
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
}
