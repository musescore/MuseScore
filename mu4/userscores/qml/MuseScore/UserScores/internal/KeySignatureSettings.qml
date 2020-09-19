import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

FlatButton {
    id: root

    height: 96

    property var model: null

    property var arrowX
    property var popupPositionX
    property var popupPositionY: height
    property alias oppened: popup.visible

    backgroundColor: oppened ? ui.theme.accentColor : ui.theme.buttonColor

    Column {
        anchors.horizontalCenter: root.horizontalCenter
        anchors.verticalCenter: root.verticalCenter

        spacing: 10

        StyledIconLabel {
            anchors.horizontalCenter: parent.horizontalCenter
            height: 50

            font.pixelSize: 65
            iconCode: model.keySignature.icon
        }

        StyledTextLabel {
            font.pixelSize: 12
            text: model.keySignature.title
        }
    }

    onClicked: {
        if (!popup.isOpened) {
            popup.open()
        } else {
            popup.close()
        }
    }

    StyledPopup {
        id: popup

        implicitHeight: column.implicitHeight + topPadding + bottomPadding + 40
        implicitWidth: 724

        arrowX: root.arrowX
        x: popupPositionX
        y: popupPositionY

        Column {
            id: column

            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 20

            spacing: 20

            TabBar {
                id: bar

                anchors.horizontalCenter: parent.horizontalCenter

                contentHeight: 28
                spacing: 0

                StyledTabButton {
                    text: qsTrc("userscores", "Major")
                    sideMargin: 22
                    isCurrent: bar.currentIndex === 0
                }
                StyledTabButton {
                    text: qsTrc("appshell", "Minor")
                    sideMargin: 22
                    isCurrent: bar.currentIndex === 1
                }
            }

            StackLayout {
                id: pagesStack

                anchors.left: parent.left
                anchors.right: parent.right

                height: childrenRect.height

                currentIndex: bar.currentIndex

                KeySignatureListView {
                    model: root.model.keySignatureMajorList()
                    currentSignature: root.model.keySignature

                    onSignatureSelected: {
                        root.model.keySignature = signature
                    }
                }

                KeySignatureListView {
                    model: root.model.keySignatureMinorList()
                    currentSignature: root.model.keySignature

                    onSignatureSelected: {
                        root.model.keySignature = signature
                    }
                }
            }
        }
    }
}
