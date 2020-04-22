import QtQuick 2.9
import QtQuick.Controls 2.2
import "../../common"

StyledPopup {
    id: root

    property alias model: noteSettingsTabPanel.proxyModel

    height: noteSettingsTabPanel.implicitHeight + topPadding + bottomPadding
    width: parent.width

    NoteSettingsTabPanel {
        id: noteSettingsTabPanel

        width: parent.width
    }
}
