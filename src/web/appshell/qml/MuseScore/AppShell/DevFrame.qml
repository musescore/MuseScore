// import QtQuick

// Item {
//     width: 200; height: 200

//     Rectangle {
//         anchors.centerIn: parent
//         width: text.implicitWidth + 20; height: text.implicitHeight + 10
//         color: "green"
//         radius: 5

//         Drag.dragType: Drag.Automatic
//         Drag.supportedActions: Qt.CopyAction
//         Drag.mimeData: {
//             "text/plain": "Copied text"
//         }

//         Text {
//             id: text
//             anchors.centerIn: parent
//             text: "Drag me"
//         }

//         DragHandler {
//             id: dragHandler
//             onActiveChanged:
//                 if (active) {
//                     parent.grabToImage(function(result) {
//                         parent.Drag.imageSource = result.url
//                         parent.Drag.active = true
//                     }, Qt.size(50, 50))
//                 } else {
//                     parent.Drag.active = false
//                 }
//         }
//     }
// }


import QtQuick

Item {
    width: 200; height: 200

    DropArea {
        x: 75; y: 75
        width: 50; height: 50

        Rectangle {
            anchors.fill: parent
            color: parent.containsDrag ? "green" : "grey"
        }
    }

    Rectangle {
        x: 10; y: 10
        width: 20; height: 20
        color: "red"

        Drag.active: dragArea.drag.active
        Drag.hotSpot.x: 10
        Drag.hotSpot.y: 10

        MouseArea {
            id: dragArea
            anchors.fill: parent

            drag.target: parent
        }
    }
}
