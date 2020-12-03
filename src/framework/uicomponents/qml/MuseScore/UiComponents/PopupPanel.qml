import QtQuick 2.9
import QtGraphicalEffects 1.0

import MuseScore.Ui 1.0

Rectangle {
    id: root

    property alias content: loader.sourceComponent
    property alias background: effectSource.sourceItem
    property alias canClose: closeButton.visible

    signal closed()

    color: ui.theme.popupBackgroundColor
    border.width: 1
    border.color: ui.theme.strokeColor

    radius: 20
    anchors.bottomMargin: -radius

    function setContentData(data) {
        if (loader.status === Loader.Ready) {
            loader.item.setData(data)
        }
    }

    function open() {
        visible = true
    }

    function close() {
        if (!canClose) {
            return
        }

        visible = false

        closed()
    }

    Loader {
        id: loader
        anchors.fill: parent
        z: 1
    }

    StyledIconLabel {
        id: closeButton

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
                close()
            }
        }
    }

    ShaderEffectSource {
        id: effectSource

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: Boolean(sourceItem) ? sourceItem.x : 0
        anchors.right: parent.right
        anchors.rightMargin: anchors.leftMargin

        height: root.height
        z: -1

        sourceRect: Qt.rect(0, root.y, width, height)

        Rectangle {
            anchors.fill: parent

            color: ui.theme.popupBackgroundColor
            opacity: 0.75
        }
    }

    FastBlur {
        anchors.fill: effectSource
        anchors.topMargin: 30

        source: effectSource
        radius: 100
        transparentBorder: true
    }
}
