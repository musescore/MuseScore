import QtQuick

import Muse.Ui
import Muse.UiComponents

Rectangle {
    id: stub

    color: ui.theme.backgroundPrimaryColor

    property string searchText

    property int viewType

    property alias backgroundColor: stub.color
    property real sideMargin

    property NavigationSection navigationSection
    property int navigationOrder

    signal createNewScoreRequested()
    signal openScoreRequested(var scorePath, var displayName)

    StyledTextLabel {
        anchors.centerIn: parent
        text: "CloudScoresView Stub"
    }
}
