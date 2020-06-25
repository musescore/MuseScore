import QtQuick 2.7

Rectangle {

    id: tool

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
        }
    ]

    signal selected(string uri)

    function select(uri) {
        tool.selected(uri);
    }

    Row {
        width: parent.width
        height: parent.height
        Repeater {

            model: tool.items

            Rectangle {

                property var item: tool.items[model.index]

                height: parent.height
                width: 60
                color: (item.uri === tool.currentUri) ? "#34C1FF" : tool.color
                Text {
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.family: "Roboto"
                    font.capitalization: Font.Capitalize
                    //color: "#ffffff"
                    text: item.title
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: tool.select(item.uri)
                }
            }
        }
    }
}
