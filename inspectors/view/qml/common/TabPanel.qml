import QtQuick 2.9
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.4

TabView {
    id: root

    readonly property int tabBarHeight: 24

    width: parent.width

    Rectangle {
        id: selectionHighlighting

        height: 3
        width: parent.width / count

        color: globalStyle.voice1Color

        radius: 2

        Connections {
            target: root

            onCurrentIndexChanged: {
                if (currentIndex < 0)
                    return

                selectionHighlighting.x = currentIndex * (root.width / count)
            }
        }

        Behavior on x {
            NumberAnimation {
                duration: 150
            }
        }
    }

    style: TabViewStyle {
        frameOverlap: 1

        tab: Column {

            height: tabBarHeight
            width: styleData.availableWidth / count

            StyledTextLabel {
                id: inspectorTitle

                width: parent.width

                text: styleData.title
                horizontalAlignment: Text.AlignHCenter
                color: styleData.selected ? globalStyle.buttonText : "#AEAEAE"
                font.bold: true
            }
        }

        frame: Rectangle {
            id: backgroundRect

            anchors.fill: parent

            color: globalStyle.window
        }
    }
}
