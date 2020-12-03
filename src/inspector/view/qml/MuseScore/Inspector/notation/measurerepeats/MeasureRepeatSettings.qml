import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"

PopupViewButton {
    id: root

    property alias model: measureRepeatPopup.model

    icon: IconCode.QUESTION_MARK /*TODO*/
    text: qsTr("Measure repeats")

    visible: root.model ? !root.model.isEmpty : false

    MeasureRepeatPopup {
        id: measureRepeatPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
