import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"
import "../notes/internal"

StyledPopup {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    function tpcListModel() {
        return [
                    { text: "C♭♭", value: AmbitusTypes.TPC_C_BB },
                    { text: "C♭", value: AmbitusTypes.TPC_C_B },
                    { text: "C", value: AmbitusTypes.TPC_C },
                    { text: "C♯", value: AmbitusTypes.TPC_C_S },
                    { text: "C♯♯", value: AmbitusTypes.TPC_C_SS },
                    { text: "D♭♭", value: AmbitusTypes.TPC_D_BB },
                    { text: "D♭", value: AmbitusTypes.TPC_D_B },
                    { text: "D", value: AmbitusTypes.TPC_D },
                    { text: "D♯", value: AmbitusTypes.TPC_D_S },
                    { text: "D♯♯", value: AmbitusTypes.TPC_D_SS },
                    { text: "E♭♭", value: AmbitusTypes.TPC_E_BB },
                    { text: "E♭", value: AmbitusTypes.TPC_E_B },
                    { text: "E", value: AmbitusTypes.TPC_E },
                    { text: "E♯", value: AmbitusTypes.TPC_E_S },
                    { text: "E♯♯", value: AmbitusTypes.TPC_E_SS },
                    { text: "F♭♭", value: AmbitusTypes.TPC_F_BB },
                    { text: "F♭", value: AmbitusTypes.TPC_F_B },
                    { text: "F", value: AmbitusTypes.TPC_F },
                    { text: "F♯", value: AmbitusTypes.TPC_F_S },
                    { text: "F♯♯", value: AmbitusTypes.TPC_F_SS },
                    { text: "G♭♭", value: AmbitusTypes.TPC_G_BB },
                    { text: "G♭", value: AmbitusTypes.TPC_G_B },
                    { text: "G", value: AmbitusTypes.TPC_G },
                    { text: "G♯", value: AmbitusTypes.TPC_G_S },
                    { text: "G♯♯", value: AmbitusTypes.TPC_G_SS },
                    { text: "A♭♭", value: AmbitusTypes.TPC_A_BB },
                    { text: "A♭", value: AmbitusTypes.TPC_A_B },
                    { text: "A", value: AmbitusTypes.TPC_A },
                    { text: "A♯", value: AmbitusTypes.TPC_A_S },
                    { text: "A♯♯", value: AmbitusTypes.TPC_A_SS },
                    { text: "B♭♭", value: AmbitusTypes.TPC_B_BB },
                    { text: "B♭", value: AmbitusTypes.TPC_B_B },
                    { text: "B", value: AmbitusTypes.TPC_B },
                    { text: "B♯", value: AmbitusTypes.TPC_B_S },
                    { text: "B♯♯", value: AmbitusTypes.TPC_B_SS }
                ]
    }

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        FlatButton {
            width: parent.width

            text: qsTr("Update to match the notes on the staff")

            onClicked: {
                if (root.model) {
                    root.model.matchRangesToStaff()
                }
            }
        }

        NoteheadsGrid {
            id: noteheadGridView

            noteHeadTypesModel: root.model ? root.model.noteheadGroupsModel : null
        }

        Column {
            spacing: 8

            height: childrenRect.height
            width: parent.width

            Item {
                height: childrenRect.height
                width: parent.width

                InspectorPropertyView {
                    titleText: qsTr("Top TPC")
                    propertyItem: root.model ? root.model.topTpc : null

                    StyledComboBox {
                        anchors.left: parent.left
                        anchors.right: parent.horizontalCenter
                        anchors.rightMargin: 2
                        width: parent.width

                        textRoleName: "text"
                        valueRoleName: "value"

                        model: tpcListModel()

                        currentIndex: root.model && !root.model.topTpc.isUndefined ? indexOfValue(root.model.topTpc.value) : -1

                        onValueChanged: {
                            root.model.topTpc.value = value
                        }
                    }
                }

                InspectorPropertyView {
                    titleText: qsTr("Top octave")
                    propertyItem: root.model ? root.model.topOctave : null

                    IncrementalPropertyControl {
                        id: topOctaveControl

                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 2
                        anchors.right: parent.right

                        iconMode: iconModeEnum.hidden
                        isIndeterminate: root.model ? root.model.topOctave.isUndefined : false
                        currentValue: root.model ? root.model.topOctave.value : 0

                        step: 1
                        decimals: 0
                        maxValue: 8
                        minValue: -1
                        validator: IntInputValidator {
                            top: topOctaveControl.maxValue
                            bottom: topOctaveControl.minValue
                        }

                        onValueEdited: { root.model.topOctave.value = newValue }
                    }
                }
            }

            Item {
                height: childrenRect.height
                width: parent.width

                StyledComboBox {
                    anchors.left: parent.left
                    anchors.right: parent.horizontalCenter
                    anchors.rightMargin: 2
                    width: parent.width

                    textRoleName: "text"
                    valueRoleName: "value"

                    model: tpcListModel()

                    currentIndex: root.model && !root.model.bottomTpc.isUndefined ? indexOfValue(root.model.bottomTpc.value) : -1

                    onValueChanged: {
                        root.model.bottomTpc.value = value
                    }
                }

                IncrementalPropertyControl {
                    id: bottomOctaveControl

                    anchors.left: parent.horizontalCenter
                    anchors.leftMargin: 2
                    anchors.right: parent.right

                    iconMode: iconModeEnum.hidden
                    isIndeterminate: root.model ? root.model.bottomOctave.isUndefined : false
                    currentValue: root.model ? root.model.bottomOctave.value : 0

                    step: 1
                    decimals: 0
                    maxValue: 8
                    minValue: -1
                    validator: IntInputValidator {
                        top: bottomOctaveControl.maxValue
                        bottom: bottomOctaveControl.minValue
                    }

                    onValueEdited: { root.model.bottomOctave.value = newValue }
                }
            }

            ExpandableBlank {
                isExpanded: false

                title: isExpanded ? qsTr("Show less") : qsTr("Show more")

                width: parent.width

                contentItemComponent: Column {
                    height: implicitHeight
                    width: parent.width

                    spacing: 16

                    InspectorPropertyView {
                        titleText: qsTr("Direction")
                        propertyItem: root.model ? root.model.direction : null

                        RadioButtonGroup {
                            id: directionButtonList

                            height: 30
                            width: parent.width

                            model: [
                                { iconRole: IconCode.AMBITUS, typeRole: DirectionTypes.HORIZONTAL_AUTO },
                                { iconRole: IconCode.AMBITUS_LEANING_LEFT, typeRole: DirectionTypes.HORIZONTAL_LEFT },
                                { iconRole: IconCode.AMBITUS_LEANING_RIGHT, typeRole: DirectionTypes.HORIZONTAL_RIGHT }
                            ]

                            delegate: FlatRadioButton {

                                ButtonGroup.group: directionButtonList.radioButtonGroup

                                checked: root.model && !root.model.direction.isUndefined ? root.model.direction.value === modelData["typeRole"]
                                                                                         : false

                                onToggled: {
                                    root.model.direction.value = modelData["typeRole"]
                                }

                                StyledIconLabel {
                                    iconCode: modelData["iconRole"]
                                }
                            }
                        }
                    }

                    InspectorPropertyView {
                        titleText: qsTr("Head type (visual only)")
                        propertyItem: root.model ? root.model.noteheadType : null

                        RadioButtonGroup {
                            id: headTypeButtonList

                            height: 30
                            width: parent.width

                            model: [
                                { iconRole: IconCode.AUTO, typeRole: NoteHead.TYPE_AUTO },
                                { iconRole: IconCode.NOTE_HEAD_QUARTER, typeRole: NoteHead.TYPE_QUARTER },
                                { iconRole: IconCode.NOTE_HEAD_HALF, typeRole: NoteHead.TYPE_HALF },
                                { iconRole: IconCode.NOTE_HEAD_WHOLE, typeRole: NoteHead.TYPE_WHOLE },
                                { iconRole: IconCode.NOTE_HEAD_BREVIS, typeRole: NoteHead.TYPE_BREVIS }
                            ]

                            delegate: FlatRadioButton {

                                ButtonGroup.group: headTypeButtonList.radioButtonGroup

                                checked: root.model && !root.model.noteheadType.isUndefined ? root.model.noteheadType.value === modelData["typeRole"]
                                                                                            : false

                                onToggled: {
                                    root.model.noteheadType.value = modelData["typeRole"]
                                }

                                StyledIconLabel {
                                    anchors.fill: parent

                                    iconCode: modelData["iconRole"]
                                }
                            }
                        }
                    }

                    InspectorPropertyView {
                        anchors.left: parent.left
                        anchors.right: parent.horizontalCenter
                        anchors.rightMargin: 2

                        titleText: qsTr("Line thickness")
                        propertyItem: root.model ? root.model.lineThickness : null

                        IncrementalPropertyControl {
                            iconMode: iconModeEnum.hidden

                            isIndeterminate: root.model ? root.model.lineThickness.isUndefined : false
                            currentValue: root.model ? root.model.lineThickness.value : 0
                            step: 0.1
                            maxValue: 10
                            minValue: 0.1
                            decimals: 2

                            onValueEdited: { root.model.lineThickness.value = newValue }
                        }
                    }
                }
            }
        }
    }
}
