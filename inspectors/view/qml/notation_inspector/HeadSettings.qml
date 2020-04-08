import QtQuick 2.0
import QtQuick.Controls 2.0
import MuseScore.Inspectors 3.3
import "../common"
import "internal"

FocusableItem {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        CheckBox {
            isIndeterminate: model ? model.isHeadHidden.isUndefined : false
            checked: model && !isIndeterminate ? model.isHeadHidden.value : false
            text: qsTr("Hide notehead")

            onClicked: { model.isHeadHidden.value = !checked }
        }

        StyledTextLabel {
            id: noteheadsLabel

            text: qsTr("Noteheads")
        }

        NoteheadsGrid {
            id: noteheadGridView

            noteHeadTypesModel: root.model ? root.model.noteheadTypesModel : null
            isIndeterminate: root.model ? root.model.headGroup.isUndefined : false
        }

        StyledTextLabel {
            id: notePositionLabel

            text: qsTr("Dotted note position")
        }

        RadioButtonGroup {
            id: notePositionButtonList

            height: 30
            width: parent.width

            model: [
                { iconRole: "qrc:/resources/icons/orientation_auto.svg", typeRole: NoteHead.DOT_POSITION_AUTO },
                { iconRole: "qrc:/resources/icons/dot_position_below.svg", typeRole: NoteHead.DOT_POSITION_DOWN },
                { iconRole: "qrc:/resources/icons/dot_position_above.svg", typeRole: NoteHead.DOT_POSITION_UP }
            ]

            delegate: FlatRadioButton {

                ButtonGroup.group: notePositionButtonList.radioButtonGroup

                checked: root.model && !root.model.dotPosition.isUndefined ? root.model.dotPosition.value === modelData["typeRole"]
                                                                           : false

                onToggled: {
                    root.model.dotPosition.value = modelData["typeRole"]
                }

                StyledIcon {
                    icon: modelData["iconRole"]
                }
            }
        }

        ExpandableBlank {
            isExpanded: false

            title: isExpanded ? qsTr("Show less") : qsTr("Show more")

            width: parent.width

            contentItemComponent: Column {
                spacing: 24

                height: implicitHeight
                width: parent.width

                Column {
                    width: root.width
                    height: implicitHeight

                    spacing: 12

                    StyledTextLabel {
                        id: headTypeLabel

                        text: qsTr("Head type (visual only)")
                    }

                    RadioButtonGroup {
                        id: headTypeButtonList

                        height: 30
                        width: parent.width

                        model: [
                            { iconRole: "qrc:/resources/icons/orientation_auto.svg", typeRole: NoteHead.TYPE_AUTO },
                            { iconRole: "qrc:/resources/icons/notehead_quarter.svg", typeRole: NoteHead.TYPE_QUARTER },
                            { iconRole: "qrc:/resources/icons/notehead_half.svg", typeRole: NoteHead.TYPE_HALF },
                            { iconRole: "qrc:/resources/icons/notehead_whole.svg", typeRole: NoteHead.TYPE_WHOLE },
                            { iconRole: "qrc:/resources/icons/notehead_brevis.svg", typeRole: NoteHead.TYPE_BREVIS }
                        ]

                        delegate: FlatRadioButton {

                            ButtonGroup.group: headTypeButtonList.radioButtonGroup

                            checked: root.model && !root.model.headType.isUndefined ? root.model.headType.value === modelData["typeRole"]
                                                                                    : false

                            onToggled: {
                                root.model.headType.value = modelData["typeRole"]
                            }

                            StyledIcon {
                                anchors.fill: parent

                                icon: modelData["iconRole"]
                            }
                        }
                    }
                }

                Column {
                    width: root.width
                    height: implicitHeight

                    spacing: 12

                    StyledTextLabel {
                        id: noteDirectionLabel

                        text: qsTr("Note direction")
                    }

                    RadioButtonGroup {
                        id: noteDirectionButtonList

                        height: 30
                        width: parent.width

                        model: [
                            { iconRole: "qrc:/resources/icons/orientation_auto.svg", typeRole: NoteHead.DIRECTION_H_AUTO },
                            { iconRole: "qrc:/resources/icons/orientation_arrow_left.svg", typeRole: NoteHead.DIRECTION_H_LEFT },
                            { iconRole: "qrc:/resources/icons/orientation_arrow_right.svg", typeRole: NoteHead.DIRECTION_H_RIGHT }
                        ]

                        delegate: FlatRadioButton {

                            ButtonGroup.group: noteDirectionButtonList.radioButtonGroup

                            checked: root.model && !root.model.headDirection.isUndefined ? root.model.headDirection.value === modelData["typeRole"]
                                                                                         : false

                            onToggled: {
                                root.model.headDirection.value = modelData["typeRole"]
                            }

                            StyledIcon {
                                anchors.fill: parent

                                icon: modelData["iconRole"]
                            }
                        }
                    }
                }

                Column {
                    spacing: 8

                    height: childrenRect.height
                    width: parent.width

                    StyledTextLabel {
                        anchors.left: parent.left

                        text: qsTr("Notehead offset")
                    }

                    Item {
                        height: childrenRect.height
                        width: parent.width

                        IncrementalPropertyControl {
                            anchors.left: parent.left
                            anchors.right: parent.horizontalCenter
                            anchors.rightMargin: 4

                            icon: "qrc:/resources/icons/horizontal_adjustment.svg"
                            enabled: model ? !model.isEmpty : false
                            isIndeterminate: model ? model.horizontalOffset.isUndefined : false
                            currentValue: model ? model.horizontalOffset.value : 0

                            onValueEdited: { model.horizontalOffset.value = newValue }
                        }

                        IncrementalPropertyControl {
                            anchors.left: parent.horizontalCenter
                            anchors.leftMargin: 4
                            anchors.right: parent.right

                            icon: "qrc:/resources/icons/vertical_adjustment.svg"
                            enabled: model ? !model.isEmpty : false
                            isIndeterminate: model ? model.verticalOffset.isUndefined : false
                            currentValue: model ? model.verticalOffset.value : 0

                            onValueEdited: { model.verticalOffset.value = newValue }
                        }
                    }
                }
            }
        }
    }
}
