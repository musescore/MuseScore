pragma ComponentBehavior: Bound

import QtQuick

import Muse.UiComponents
import Muse.Extensions

Rectangle {
    color: ui.theme.backgroundSecondaryColor

    DevExtensionsListModel {
        id: devModel
    }

    StyledListView {
        anchors.fill: parent

        model: devModel.extensionsList()

        delegate: ListItemBlank {
            id: itemDelegate

            required property string title
            required property string uri
            required property string type
            required property int index

            anchors.left: parent ? parent.left : undefined
            anchors.right: parent ? parent.right : undefined
            height: 96

            StyledTextLabel {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignLeft
                text: (itemDelegate.index + 1) + ": " + itemDelegate.title
                + "\n uri: " + itemDelegate.uri
                + "\n type: " + itemDelegate.type
            }

            onClicked: devModel.clicked(itemDelegate.uri)
        }
    }
}
