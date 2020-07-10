import QtQuick 2.7
import QtQuick.Controls 2.0
import MuseScore.UiComponents 1.0

Rectangle {

    id: root

    property var currentUri: "musescore://home"
    property var items: [
        {
            title: qsTrc("appshell", "Home"),
            uri: "musescore://home"
        },
        {
            title: qsTrc("appshell", "Notation"),
            uri: "musescore://notation"
        },
        {
            title: qsTrc("appshell", "Sequencer"),
            uri: "musescore://sequencer"
        },
        {
            title: qsTrc("appshell", "Publish"),
            uri: "musescore://publish"
        },
        {
            title: qsTrc("appshell", "Settings"),
            uri: "musescore://settings"
        },
        {
            title: qsTrc("appshell", "DevTools"),
            uri: "musescore://devtools"
        }
    ]

    signal selected(string uri)

    function select(uri) {
        root.selected(uri);
    }

    RadioButtonGroup {
        id: radioButtonList

        height: parent.height
        width: parent.width

        spacing: 0

        model: root.items

        delegate: FlatRadioButton {
            id: radioButtonDelegate

            ButtonGroup.group: radioButtonList.radioButtonGroup

            checked: modelData["uri"] === root.currentUri
            onToggled: {
                root.currentUri = modelData["uri"]
                root.selected(modelData["uri"])
            }

            StyledTextLabel {
                id: label

                text: modelData["title"]

                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
