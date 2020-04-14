import QtQuick 2.9
import QtQuick.Controls 2.2
import "../common"

StyledPopup {
    id: root

    property alias proxyModel: settingsTabPanel.proxyModel

    height: settingsTabPanel.implicitHeight + topPadding + bottomPadding
    width: parent.width

    NoteSettingsTabPanel {
        id: settingsTabPanel

        width: parent.width
    }
}
