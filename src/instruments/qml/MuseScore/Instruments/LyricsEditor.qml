import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls 2.2
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

import "internal"

Item {
    id: root

    LyricsEditorModel {
        id: lyricsModel
    }

    Rectangle {
        id: background

        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        MouseArea {
            anchors.fill: parent
        }
    }

    ColumnLayout{
        id: contentColumn
        anchors.fill: parent
        readonly property int sideMargin: 12
        spacing: sideMargin

        Column {
            id: tabTitleColumn

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: 24

            spacing: 4

            StyledTextLabel {
                Layout.leftMargin: contentColumn.sideMargin
                Layout.rightMargin: contentColumn.sideMargin

                id: editorTitle

                text: qsTrc("lyrics", "Lyrics")
                font: ui.theme.largeBodyBoldFont
            }

            Rectangle {
                Layout.leftMargin: contentColumn.sideMargin
                Layout.rightMargin: contentColumn.sideMargin

                id: titleHighlighting

                height: 3
                width: editorTitle.width

                color: ui.theme.accentColor

                radius: 2
            }
        }

        ScrollView {
            id: view
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: contentColumn.sideMargin
            Layout.rightMargin: contentColumn.sideMargin

            TextArea {
                id: textArea
                placeholderText: qsTr("Enter lyrics")
                wrapMode: "WordWrap"
                width: view.width
                background: Rectangle {
                    anchors.fill: parent
                    border.color: "transparent"
                }

                Connections {
                    target: lyricsModel
                    onLyricsUpdated: textArea.text = lyricsModel.updateLyrics()
                }
            }
        }

        RowLayout {
            id: buttonsRow
            Layout.fillWidth: true
            readonly property int sideMargin: parent.sideMargin

            Button {
                id: modeButton

                Layout.leftMargin: parent.sideMargin
                Layout.rightMargin: parent.sideMargin

                text: "Mode"
                onClicked: {
                    lyricsModel.mode = !lyricsModel.mode
                    textArea.text = lyricsModel.updateLyrics()
                }

                contentItem: Text {
                    text: modeButton.text
                    font: modeButton.font
                    opacity: enabled ? 1.0 : 0.3
                    color: modeButton.down ? "#17a81a" : "#21be2b"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    opacity: enabled ? 1 : 0.3
                    border.color: modeButton.down ? "#17a81a" : "#21be2b"
                    border.width: 1
                    radius: 2
                }
            }

            Button {
                id: expandButton

                Layout.leftMargin: parent.sideMargin
                Layout.rightMargin: parent.sideMargin

                text: "Expand Repeats"
                onClicked: {
                    lyricsModel.expandRepeats = !lyricsModel.expandRepeats
                    textArea.text = lyricsModel.updateLyrics()
                }

                contentItem: Text {
                    text: expandButton.text
                    font: expandButton.font
                    opacity: enabled ? 1.0 : 0.3
                    color: expandButton.down ? "#17a81a" : "#21be2b"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    opacity: enabled ? 1 : 0.3
                    border.color: expandButton.down ? "#17a81a" : "#21be2b"
                    border.width: 1
                    radius: 2
                }
            }
        }
    }
}
