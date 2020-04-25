import QtQuick 2.9

Rectangle {

    id: tool

    property var currentItem: "home"
    property var items: ["home", "notation", "sequencer", "publish"]

    signal selected(string item)

    function select(item) {
        tool.currentItem = item;
        tool.selected(item);
    }

    Row {
        width: parent.width
        height: parent.height
        Repeater {
            model: tool.items
            Rectangle {
                height: parent.height
                width: 60
                color: (tool.items[model.index] === tool.currentItem) ? "#34C1FF" : tool.color
                Text {
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.family: "Roboto"
                    font.capitalization: Font.Capitalize
                    //color: "#ffffff"
                    text: tool.items[model.index]
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: tool.select(tool.items[model.index])
                }
            }
        }
    }
}
