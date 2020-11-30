import QtQuick 2.9
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.4
import MuseScore.Ui 1.0

TabView {
    id: root

    readonly property int tabBarHeight: 24

    width: parent.width

    Rectangle {
        id: selectionHighlighting

        x: {
            if (root.currentIndex < 0) {
                return
            }

            root.currentIndex * (root.width / count)
        }

        height: 3
        width: parent.width / count

        color: ui.theme.accentColor

        radius: 2

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
                id: titleLabel

                width: parent.width

                text: styleData.title
                font: ui.theme.bodyBoldFont
                opacity: styleData.selected ? ui.theme.buttonOpacityHit : ui.theme.buttonOpacityNormal
            }
        }

        frame: Rectangle {
            id: backgroundRect

            anchors.fill: parent

            color: ui.theme.backgroundPrimaryColor
        }
    }
}
