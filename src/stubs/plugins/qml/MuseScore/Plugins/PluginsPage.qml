import QtQuick 2.15

import MuseScore.UiComponents 1.0

Rectangle {
    property string search: ""
    property string selectedCategory: ""
    property string backgroundColor: ui.theme.backgroundPrimaryColor

    function categories() {
        return {}
    }

    color: backgroundColor

    StyledTextLabel {
        anchors.centerIn: parent
        text: "Plugins Page Stub"
    }
}
