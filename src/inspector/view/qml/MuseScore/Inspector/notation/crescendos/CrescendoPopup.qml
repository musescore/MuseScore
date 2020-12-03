import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls 1.5
import MuseScore.UiComponents 1.0
import "../../common"
import "internal"

StyledPopup {
    id: root

    property alias model: crescendoTabPanel.model

    height: crescendoTabPanel.implicitHeight + topPadding + bottomPadding
    width: parent.width

    CrescendoTabPanel {
        id: crescendoTabPanel

        width: parent.width
    }
}
