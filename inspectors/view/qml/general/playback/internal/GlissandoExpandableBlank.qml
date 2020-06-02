import QtQuick 2.9
import MuseScore.Inspectors 3.3
import "../../../common"

ExpandableBlank {
    id: root

    property QtObject model: null

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: Column {
        spacing: 8

        height: implicitHeight
        width: root.width

        StyledTextLabel {
            text: qsTr("Style")
        }

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
