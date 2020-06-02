import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspectors 3.3
import "../../common"

StyledPopup {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        CheckBox {
            isIndeterminate: model ? model.shouldShowCourtesy.isUndefined : false
            checked: model && !isIndeterminate ? model.shouldShowCourtesy.value : false
            text: qsTr("Show courtesy clef on previous measure")

            onClicked: { model.shouldShowCourtesy.value = !checked }
        }
    }
}
