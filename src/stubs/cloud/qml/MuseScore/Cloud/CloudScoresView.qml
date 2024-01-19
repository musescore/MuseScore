import QtQuick 2.15

import MuseScore.UiComponents 1.0

Rectangle {
    id: stub

    color: ui.theme.backgroundPrimaryColor

    property var viewType
    property var searchText

    property alias backgroundColor: stub.color
    property var sideMargin

    property var navigationSection
    property var navigationOrder

    signal createNewScoreRequested()
    signal openScoreRequested(var scorePath, var displayName)

    StyledTextLabel {
        anchors.centerIn: parent
        text: "CloudScoresView Stub"
    }
}
