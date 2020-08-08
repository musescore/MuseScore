import QtQuick 2.7
import QtQuick.Controls 2.2


Item {

    id: root

    property var selectedSoundFonts: []
    property var avalaibleSoundFonts: []

    signal selectedUpClicked(var index)
    signal selectedDownClicked(var index)
    signal selectedRemoveClicked(var index)
    signal addClicked(var index)

    onSelectedSoundFontsChanged: {
        selectedView.model = 0;
        selectedView.model = root.selectedSoundFonts
    }

    Item {
        id: avalaiblePanel

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 8
        width: (parent.width / 2) - 40

        Text {
            id: headerAvalaible
            anchors.left: parent.left
            anchors.leftMargin: 8
            width: parent.width
            height: 40
            verticalAlignment: Text.AlignVCenter
            font.bold: true
            text: "Avalaible"
        }

        ListView {
            anchors.top: headerAvalaible.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            model: root.avalaibleSoundFonts

            delegate: Item {
                width: parent.width
                height: 40

                Text {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: addBtn.left
                    anchors.leftMargin: 8
                    verticalAlignment: Text.AlignVCenter

                    text: modelData
                }

                Button {
                    id: addBtn
                    height: parent.height
                    anchors.right: parent.right
                    width: 40
                    text: "→"
                    onClicked: root.addClicked(model.index)
                }
            }
        }
    }

    Rectangle {
        id: separator
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: avalaiblePanel.right
        width: 4
        color: "#666666"
    }

    Item {
        id: selectedPanel
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: separator.right
        anchors.right: parent.right
        anchors.margins: 8

        Text {
            id: headerSelecetd
            anchors.left: parent.left
            anchors.leftMargin: 8
            width: parent.width
            height: 40
            verticalAlignment: Text.AlignVCenter
            font.bold: true
            text: "Selected"
        }

        ListView {
            id: selectedView
            anchors.top: headerSelecetd.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            model: root.avalaibleSoundFonts

            delegate: Item {
                width: parent.width
                height: 40

                Text {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: btns.left
                    anchors.leftMargin: 8

                    verticalAlignment: Text.AlignVCenter

                    text: modelData
                }

                Row {

                    id: btns

                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    width: childrenRect.width

                    Button {
                        id: upBtn
                        height: parent.height
                        width: 40
                        text: "↑"
                        onClicked: root.selectedUpClicked(model.index)
                    }

                    Button {
                        id: downBtn
                        height: parent.height
                        width: 40
                        text: "↓"
                        onClicked: root.selectedDownClicked(model.index)
                    }

                    Button {
                        id: remBtn
                        height: parent.height
                        width: 40
                        text: "-"
                        onClicked: root.selectedRemoveClicked(model.index)
                    }
                }
            }
        }
    }
}
