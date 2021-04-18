import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.UiComponents 1.0

Flickable {
    contentWidth: width

    clip: true
    boundsBehavior: Flickable.StopAtBounds
    interactive: height < contentHeight

    ScrollBar.vertical: StyledScrollBar {}

    signal hideRequested()

    function apply() {
        return true
    }
}
