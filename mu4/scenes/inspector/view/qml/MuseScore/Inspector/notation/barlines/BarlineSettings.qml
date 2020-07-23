import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"

PopupViewButton {
    id: root

    property alias barlineSettingsModel: barlinePopup.barlineSettingsModel
    property alias staffSettingsModel: barlinePopup.staffSettingsModel

    icon: IconCode.SECTION_BREAK
    text: qsTr("Barlines")

    visible: root.barlineSettingsModel ? !root.barlineSettingsModel.isEmpty : false

    BarlinePopup {
        id: barlinePopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
