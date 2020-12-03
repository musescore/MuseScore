import QtQuick 2.7

import MuseScore.UiComponents 1.0

Item {
    id: root

    property alias listTitle: title.text
    property alias model: view.model
    property alias searchEnabled: searchField.visible
    property alias searchText: searchField.searchText

    property bool boldFont: false

    signal titleClicked(var index)

    StyledTextLabel {
        id: title

        anchors.top: parent.top

        font.bold: true
    }

    SearchField {
        id: searchField

        anchors.top: title.bottom
        anchors.topMargin: 16

        width: parent.width
    }

    ListView {
        id: view

        anchors.top: searchEnabled ? searchField.bottom : title.bottom
        anchors.topMargin: 16
        anchors.bottom: parent.bottom

        width: parent.width
        spacing: 8

        boundsBehavior: ListView.StopAtBounds
        clip: true

        currentIndex: 0

        delegate: Item {
            width: parent.width
            height: 30

            property bool isCurrent: view.currentIndex === model.index

            Rectangle {
                anchors.fill: parent
                color: isCurrent ? ui.theme.accentColor : ui.theme.backgroundPrimaryColor
                opacity: isCurrent ? 0.3 : 1
            }

            StyledTextLabel {
                id: titleLabel

                anchors.fill: parent
                anchors.leftMargin: 12

                horizontalAlignment: Text.AlignLeft
                text: modelData
                font.bold: root.boldFont
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    view.currentIndex = model.index
                    root.titleClicked(model.index)
                }
            }
        }
    }
}
