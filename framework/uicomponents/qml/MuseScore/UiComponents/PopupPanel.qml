import QtQuick 2.9

import MuseScore.Ui 1.0

Rectangle {
    id: root

    property alias content: loader.sourceComponent

    signal closed()

    height: loader.height + 84
    color: ui.theme.popupBackgroundColor
    border.width: 1
    border.color: ui.theme.strokeColor
    radius: 20

    anchors.bottomMargin: -20

    function setContentData(data) {
        if (loader.status === Loader.Ready) {
            loader.item.setData(data)
        }
    }

    function show(data) {
        setContentData(data)
        visible = true
    }

    function hide() {
        visible = false

        closed()
    }

    Loader {
        id: loader

        readonly property int sideMargin: 68

        anchors.left: parent.left
        anchors.leftMargin: sideMargin
        anchors.right: parent.right
        anchors.rightMargin: sideMargin
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 42

        height: sourceComponent.height
    }

    StyledIconLabel {
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.right: parent.right
        anchors.rightMargin: 20

        width: 32
        height: width

        font.pixelSize: 32

        iconCode: IconCode.CLOSE_X_ROUNDED

        MouseArea {
            anchors.fill: parent

            onClicked: {
                hide()
            }
        }
    }
}
