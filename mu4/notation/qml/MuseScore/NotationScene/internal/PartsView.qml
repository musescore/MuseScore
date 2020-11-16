import QtQuick 2.9

import MuseScore.UiComponents 1.0

Item {
    id: root

    property var model

    Column {
        id: header

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 36

        spacing: 16

        Row {
            width: parent.width
            height: childrenRect.height

            StyledTextLabel {
                width: parent.width / 2

                text: qsTrc("notation", "NAME")

                horizontalAlignment: Qt.AlignLeft
                font.capitalization: Font.AllUppercase
            }

            StyledTextLabel {
                id: voicesVisibilityHeader

                width: parent.width / 2

                text: qsTrc("notation", "VISIBLE VOICES")

                horizontalAlignment: Qt.AlignLeft
                font.capitalization: Font.AllUppercase
            }
        }

        SeparatorLine { anchors.margins: -36 }
    }

    ListView {
        id: view

        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        width: parent.width

        spacing: 0

        model: root.model

        boundsBehavior: Flickable.StopAtBounds
        interactive: height < contentHeight
        clip: true

        delegate: PartDelegate {
            readonly property int sideMargin: 24

            anchors.left: parent.left
            anchors.leftMargin: sideMargin
            anchors.right: parent.right
            anchors.rightMargin: sideMargin

            title: model.title
            maxTitleWidth: voicesVisibilityHeader.x
            currentPartIndex: view.currentIndex
            isSelected: model.isSelected
            isMain: model.isMain
            voicesVisibility: model.voicesVisibility
            voicesTitle: model.voicesTitle

            onPartClicked: {
                root.model.selectPart(model.index)
                view.currentIndex = model.index
            }

            onTitleChanged: {
                root.model.setPartTitle(model.index, title)
            }

            onVoicesVisibilityChangeRequested: {
                root.model.setVoicesVisibility(model.index, voicesVisibility)
            }

            onRemovePartRequested: {
                root.model.removePart(model.index)
            }

            onCopyPartRequested: {
                root.model.copyPart(model.index)
            }
        }
    }
}
