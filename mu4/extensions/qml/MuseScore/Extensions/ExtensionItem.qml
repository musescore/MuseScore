import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.Extensions 1.0

Rectangle {
    id: root

    color: ui.theme.popupBackgroundColor
    radius: 12

    property string code: ""
    property alias name: nameLabel.text
    property string description: ""
    property int status: 0
    property bool selected: false

    signal clicked(string code)

    RowLayout {
        anchors.fill: parent

        Item {
            width: parent.width / 2.1
            height: parent.height
            clip: true

            Rectangle {
                anchors.fill: parent
                anchors.rightMargin: -12 // NOTE: for a rounded corner on the left side only
                radius: root.radius
                border.width: root.border.width
                border.color: root.border.color

                color: "#595959" // TODO

                StyledTextLabel {
                    anchors.fill: parent

                    text: qsTrc("extensions", "Placeholder")
                    font.pixelSize: 30
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            Column {
                anchors.fill: parent
                anchors.margins: 58

                spacing: 20

                StyledTextLabel {
                    id: nameLabel

                    width: parent.width

                    font.pixelSize: 22
                    font.bold: true
                    horizontalAlignment: Text.AlignLeft
                }

                StyledTextLabel {
                    id: descriptionLabel

                    width: parent.width

                    wrapMode: Text.WordWrap
                    maximumLineCount: 5
                    horizontalAlignment: Text.AlignLeft

                    text: {
                        var breakIndex = description.indexOf('.') // NOTE: first sentence
                        if (breakIndex === -1) {
                            return description
                        }
                        return root.description.substring(0, breakIndex + 1)
                    }

                    font.pixelSize: 12
                }
            }
        }
    }

    StyledTextLabel {
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20

        text: qsTrc("extensions", "FREE") // TODO: get from model
        font.pixelSize: 16
        font.bold: true
    }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            root.clicked(code)
        }
    }

    states: [
        State {
            name: "SELECTED"
            when: selected

            PropertyChanges {
                target: root
                border.width: 2
                border.color: ui.theme.accentColor
            }
        },
        State {
            name: "NORMAL"
            when: !selected

            PropertyChanges {
                target: root
                border.width: 0
            }
        }
    ]

}
