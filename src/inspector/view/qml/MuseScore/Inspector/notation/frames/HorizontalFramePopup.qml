import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import MuseScore.UiComponents 1.0
import "../../common"
import "internal"

StyledPopup {
    id: root

    property QtObject model: undefined

    height: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        height: implicitHeight
        width: parent.width

        spacing: 16

        WidthSection {
            widthProperty: model ? model.frameWidth : null
        }

        SeparatorLine { anchors.margins: -10 }

        HorizontalGapsSection {
            leftGap: model ? model.leftGap : null
            rightGap: model ? model.rightGap: null
        }

        SeparatorLine { anchors.margins: -10 }

        CheckBox {
            isIndeterminate: model ? model.shouldDisplayKeysAndBrackets.isUndefined : false
            checked: model && !isIndeterminate ? model.shouldDisplayKeysAndBrackets.value : false
            text: qsTr("Display key, brackets and braces")

            onClicked: { model.shouldDisplayKeysAndBrackets.value = !checked }
        }
    }
}
