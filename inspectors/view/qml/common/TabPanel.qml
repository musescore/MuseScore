import QtQuick 2.9
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.4

TabView {
    id: root

    readonly property int tabBarHeight: 24

    Rectangle {
        id: activeHighlight

        height: 3
        width: parent.width / count

        color: globalStyle.highlight

        radius: 2

        Connections {
            target: root

            onCurrentIndexChanged: {
                if (currentIndex < 0)
                    return

                activeHighlight.x = currentIndex * (root.width / count)
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
                color: globalStyle.buttonText
                font.bold: true
            }

            Rectangle {
                anchors.horizontalCenter: inspectorTitle.horizontalCenter

                height: 2
                radius: 2
                width: styleData.hovered && !styleData.selected ? inspectorTitle.paintedWidth : 0.0

                color: globalStyle.highlight

                Behavior on width {
                    NumberAnimation {
                        duration: 100
                        easing.type: "InOutQuad"
                    }
                }
            }
        }

        frame: Rectangle {
            id: backgroundRect

            anchors.fill: parent

            color: globalStyle.window
        }
    }
}
