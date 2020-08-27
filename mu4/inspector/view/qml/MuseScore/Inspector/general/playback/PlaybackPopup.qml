import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.UiComponents 1.0
import "../../common"

StyledPopup {
    id: root

    property alias proxyModel: playbackTabPanel.proxyModel

    height: playbackTabPanel.implicitHeight + topPadding + bottomPadding
    width: parent.width

    PlaybackTabPanel {
        id: playbackTabPanel

        width: parent.width
    }
}
