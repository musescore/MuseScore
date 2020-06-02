import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls 1.5
import "../../common"
import "internal"

StyledPopup {
    id: root

    property alias model: hairpinTabPanel.model

    height: hairpinTabPanel.implicitHeight + topPadding + bottomPadding
    width: parent.width

    HairpinTabPanel {
        id: hairpinTabPanel

        width: parent.width
    }
}
