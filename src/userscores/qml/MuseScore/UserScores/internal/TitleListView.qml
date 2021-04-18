import QtQuick 2.7

import MuseScore.UiComponents 1.0

Item {
    id: root

    property alias listTitle: title.text
    property alias model: view.model
    property alias searchEnabled: searchField.visible
    property alias searchText: searchField.searchText

    signal titleClicked(var index)

    StyledTextLabel {
        id: title

        anchors.top: parent.top

        font: ui.theme.bodyBoldFont
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
        spacing: 0

        boundsBehavior: ListView.StopAtBounds
        clip: true

        currentIndex: 0

        delegate: ListItemBlank {
            isSelected: view.currentIndex === model.index

            StyledTextLabel {
                id: titleLabel

                anchors.fill: parent
                anchors.leftMargin: 12

                horizontalAlignment: Text.AlignLeft
                text: modelData
                font: ui.theme.bodyBoldFont
            }

            onClicked: {
                view.currentIndex = model.index
                root.titleClicked(model.index)
            }
        }
    }
}
