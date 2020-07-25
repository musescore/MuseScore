import QtQuick 2.7
import MuseScore.UiComponents 1.0

Column {
    spacing: 32

    StyledTextLabel {
        text: qsTrc("cloud", "What are the benefits of a MuseScore account?")

        font.pixelSize: 32
        font.bold: true
    }

    StyledTextLabel {
        text: qsTrc("cloud", "A MuseScore profile allows you to save & publish your scores on MuseScore.com. It's free.")
    }

    ListView {
        spacing: 32
        height: contentHeight
        width: contentWidth

        model: [
            qsTrc("cloud", "Save your scores to private cloud area"),
            qsTrc("cloud", "Share links with other musicians, who add comments"),
            qsTrc("cloud", "Create a portfolio for your music and gain followers"),
            qsTrc("cloud", "Upload high quality audio for superb score playback")
        ]

        delegate: Row {
            spacing: 38

            Rectangle {
                width: 9
                height: width
                color: ui.theme.accentColor
                radius: width / 2
                anchors.verticalCenter: parent.verticalCenter
            }

            StyledTextLabel {
                text: modelData
            }
        }
    }
}
