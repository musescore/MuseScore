import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"

StyledPopup {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    InspectorPropertyView {
        id: contentColumn

        titleText: qsTr("Style (between notes)")
        propertyItem: root.model ? root.model.strokeStyle : null

        RadioButtonGroup {
            id: radioButtonList

            height: 30
            width: parent.width

            model: [
                { iconRole: IconCode.TREMOLO_STYLE_DEFAULT, typeRole: TremoloTypes.STYLE_DEFAULT },
                { iconRole: IconCode.TREMOLO_STYLE_ALL_STROKES_ATTACHED, typeRole: TremoloTypes.STYLE_ALL_STROKES_ATTACHED },
                { iconRole: IconCode.TREMOLO_STYLE_SINGLE_STROKE_ATTACHED, typeRole: TremoloTypes.STYLE_SINGLE_STROKE_ATTACHED }
            ]

            delegate: FlatRadioButton {
                ButtonGroup.group: radioButtonList.radioButtonGroup

                checked: root.model && !root.model.strokeStyle.isUndefined ? root.model.strokeStyle.value === modelData["typeRole"]
                                                                         : false

                onToggled: {
                    root.model.strokeStyle.value = modelData["typeRole"]
                }

                StyledIconLabel {
                    iconCode: modelData["iconRole"]
                }
            }
        }
    }
}
