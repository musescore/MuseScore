import QtQuick 2.9
import QtGraphicalEffects 1.0

import MuseScore.Ui 1.0

Rectangle {
    id: root

    property alias content: loader.sourceComponent
    property alias background: effectSource.sourceItem

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

    function show() {
        visible = true
    }

    function hide() {
        visible = false

        closed()
    }

    Loader {
        id: loader

        z: 1000

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

        font.pixelSize: 16

        iconCode: IconCode.CLOSE_X_ROUNDED

        MouseArea {
            anchors.fill: parent

            onClicked: {
                hide()
            }
        }
    }

    ShaderEffectSource {
        id: effectSource

        readonly property int blurTopMargin: 68

        anchors.top: parent.top
        anchors.topMargin: blurTopMargin
        anchors.left: parent.left
        anchors.leftMargin: Boolean(sourceItem) ? sourceItem.x : 0
        anchors.right: parent.right
        anchors.rightMargin: anchors.leftMargin

        height: root.height / 3
        z: -1

        sourceRect: Qt.rect(0, root.y + blurTopMargin, width, height)
    }

    GaussianBlur {
        anchors.fill: effectSource
        source: effectSource
        radius: 100
        samples: 90
        transparentBorder: true
    }
}
