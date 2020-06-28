import QtQuick 2.0
import QtQuick.Controls 2.0
import MuseScore.Inspectors 3.3
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"
import "internal"

FocusableItem {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 16

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
        }

        CheckBox {
            isIndeterminate: {
                if (!model)
                    return false;
                if (model.isUndefined)
                    return false;

                return model.parenthesesNotehead.value === NoteHead.PARENTHESIS_RIGHT || model.parenthesesNotehead.value === NoteHead.PARENTHESIS_LEFT;
            }
            checked: (model && !isIndeterminate) ? (model.parenthesesNotehead.value === NoteHead.PARENTHESIS_ALL) : false
            text: qsTr("Bracket Notehead")

            onClicked: {
                if (isIndeterminate) {
                    model.parenthesesNotehead.value = NoteHead.PARENTHESIS_ALL;
                    return;
                }
                model.parenthesesNotehead.value = (checked ? NoteHead.PARENTHESIS_NONE : NoteHead.PARENTHESIS_ALL);
            }
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
                { iconRole: IconCode.AUTO, typeRole: NoteHead.DOT_POSITION_AUTO },
                { iconRole: IconCode.DOT_ABOVE_LINE, typeRole: NoteHead.DOT_POSITION_DOWN },
                { iconRole: IconCode.DOT_BELOW_LINE, typeRole: NoteHead.DOT_POSITION_UP }
            ]

            delegate: FlatRadioButton {

                ButtonGroup.group: notePositionButtonList.radioButtonGroup

                checked: root.model && !root.model.dotPosition.isUndefined ? root.model.dotPosition.value === modelData["typeRole"]
                                                                           : false

                onToggled: {
                    root.model.dotPosition.value = modelData["typeRole"]
                }

                StyledIconLabel {
                    iconCode: modelData["iconRole"]
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

                            Icon {
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
                            { iconRole: IconCode.AUTO, typeRole: NoteHead.DIRECTION_H_AUTO },
                            { iconRole: IconCode.ARROW_LEFT, typeRole: NoteHead.DIRECTION_H_LEFT },
                            { iconRole: IconCode.ARROW_RIGHT, typeRole: NoteHead.DIRECTION_H_RIGHT }
                        ]

                        delegate: FlatRadioButton {

                            ButtonGroup.group: noteDirectionButtonList.radioButtonGroup

                            checked: root.model && !root.model.headDirection.isUndefined ? root.model.headDirection.value === modelData["typeRole"]
                                                                                         : false

                            onToggled: {
                                root.model.headDirection.value = modelData["typeRole"]
                            }

                            StyledIconLabel {
                                iconCode: modelData["iconRole"]
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

                            icon: IconCode.HORIZONTAL
                            enabled: model ? !model.isEmpty : false
                            isIndeterminate: model ? model.horizontalOffset.isUndefined : false
                            currentValue: model ? model.horizontalOffset.value : 0

                            onValueEdited: { model.horizontalOffset.value = newValue }
                        }

                        IncrementalPropertyControl {
                            anchors.left: parent.horizontalCenter
                            anchors.leftMargin: 4
                            anchors.right: parent.right

                            icon: IconCode.VERTICAL
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
