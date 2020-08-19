import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

import "../common"

InspectorSectionView {
    id: root

    implicitHeight: contentColumn.implicitHeight

    Column {
        id: contentColumn

        width: parent.width

        spacing: 24

        GridLayout {
            id: grid

            width: parent.width

            columns: 2

            rowSpacing: 16
            columnSpacing: 4

            Column {
                spacing: 8

                Layout.fillWidth: true
                Layout.maximumWidth: parent.width/2

                StyledTextLabel {
                    text: qsTr("Page size")
                }

                StyledComboBox {
                    width: parent.width

                    textRoleName: "nameRole"
                    valueRoleName: "idRole"

                    model: root.model ? root.model.pageTypeListModel : null

                    currentIndex: root.model && root.model.pageTypeListModel ? indexOfValue(root.model.pageTypeListModel.currentPageSizeId) : -1

                    onValueChanged: {
                        root.model.pageTypeListModel.currentPageSizeId = value
                    }
                }
            }

            Column {
                spacing: 8

                Layout.fillWidth: true
                Layout.maximumWidth: parent.width/2

                StyledTextLabel {
                    text: qsTr("Orientation")
                }

                RadioButtonGroup {
                    id: orientationType

                    height: 30
                    width: parent.width

                    model: [
                        { iconRole: IconCode.ORIENTATION_PORTRAIT, typeRole: ScoreAppearanceTypes.ORIENTATION_PORTRAIT },
                        { iconRole: IconCode.ORIENTATION_LANDSCAPE, typeRole: ScoreAppearanceTypes.ORIENTATION_LANDSCAPE }
                    ]

                    delegate: FlatRadioButton {

                        ButtonGroup.group: orientationType.radioButtonGroup

                        checked: root.model ? root.model.orientationType === modelData["typeRole"]
                                            : false

                        onToggled: {
                            root.model.orientationType = modelData["typeRole"]
                        }

                        StyledIconLabel {
                            iconCode: modelData["iconRole"]
                        }
                    }
                }
            }

            Column {
                spacing: 8

                Layout.fillWidth: true
                Layout.maximumWidth: parent.width/2

                StyledTextLabel {
                    text: qsTr("Staff spacing")
                }

                IncrementalPropertyControl {
                    iconMode: iconModeEnum.hidden

                    currentValue: root.model ? root.model.staffSpacing : 0
                    measureUnitsSymbol: qsTr("mm")

                    step: 0.1
                    decimals: 3
                    minValue: 0.1
                    maxValue: 100

                    onValueEdited: { root.model.staffSpacing = newValue }
                }
            }

            Column {
                Layout.fillWidth: true
                Layout.maximumWidth: parent.width/2

                spacing: 8

                StyledTextLabel {
                    text: qsTr("Staff distance")
                }

                IncrementalPropertyControl {
                    iconMode: iconModeEnum.hidden

                    currentValue: root.model ? root.model.staffDistance : 0

                    step: 0.1
                    minValue: 0.1
                    maxValue: 10

                    onValueEdited: { root.model.staffDistance = newValue }
                }
            }
        }

        Column {
            width: parent.width

            spacing: 8

            FlatButton {
                width: parent.width

                text: qsTr("More page settings")

                onClicked: {
                    if (root.model) {
                        root.model.showPageSettings()
                    }
                }
            }

            FlatButton {
                width: parent.width

                text: qsTr("Style settings")

                onClicked: {
                    if (root.model) {
                        root.model.showStyleSettings()
                    }
                }
            }
        }
    }
}
