import QtQuick 2.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Item {
    id: root

    property string headerTitle: ""
    property alias spacing: row.spacing
    property bool isSorterEnabled: false
    property int sortOrder: Qt.AscendingOrder

    signal clicked()

    Row {
        id: row

        anchors.fill: parent
        spacing: root.spacing

        StyledTextLabel {
            anchors.verticalCenter: parent.verticalCenter
            width: implicitWidth

            text: headerTitle
            horizontalAlignment: Text.AlignLeft
            font.capitalization: Font.AllUppercase
        }

        StyledIconLabel {
            anchors.verticalCenter: parent.verticalCenter

            visible: isSorterEnabled
            iconCode: sortOrder === Qt.AscendingOrder ? IconCode.SMALL_ARROW_UP : IconCode.SMALL_ARROW_DOWN
        }
    }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            root.clicked()
        }
    }
}
