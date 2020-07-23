import QtQuick 2.9
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import "../../../common"

ExpandableBlank {
    id: root

    property QtObject model: null

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: InspectorPropertyView {
        width: root.width

        titleText: qsTr("Style")
        propertyItem: root.model ? root.model.styleType : null

        StyledComboBox {
            width: parent.width

            textRoleName: "text"
            valueRoleName: "value"

            model: [
                { text: "Chromatic", value: Glissando.STYLE_CHROMATIC },
                { text: "White keys", value: Glissando.STYLE_WHITE_KEYS },
                { text: "Black keys", value: Glissando.STYLE_BLACK_KEYS },
                { text: "Diatonic", value: Glissando.STYLE_DIATONIC }
            ]

            currentIndex: root.model && !root.model.styleType.isUndefined ? indexOfValue(root.model.styleType.value) : -1

            onValueChanged: {
                root.model.styleType.value = value
            }
        }
    }
}
