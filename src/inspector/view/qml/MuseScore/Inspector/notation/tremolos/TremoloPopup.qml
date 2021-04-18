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

        titleText: qsTrc("inspector", "Style (between notes)")
        propertyItem: root.model ? root.model.style : null

        RadioButtonGroup {
            id: radioButtonList

            height: 30
            width: parent.width

            model: [
                { iconRole: IconCode.TREMOLO_STYLE_DEFAULT, typeRole: TremoloTypes.STYLE_DEFAULT },
                { iconRole: IconCode.TREMOLO_STYLE_TRADITIONAL, typeRole: TremoloTypes.STYLE_TRADITIONAL },
                { iconRole: IconCode.TREMOLO_STYLE_TRADITIONAL_ALTERNATE, typeRole: TremoloTypes.STYLE_TRADITIONAL_ALTERNATE }
            ]

            delegate: FlatRadioButton {
                ButtonGroup.group: radioButtonList.radioButtonGroup

                checked: root.model && !root.model.style.isUndefined ? root.model.style.value === modelData["typeRole"]
                                                                     : false

                onToggled: {
                    root.model.style.value = modelData["typeRole"]
                }

                StyledIconLabel {
                    iconCode: modelData["iconRole"]
                }
            }
        }
    }
}
