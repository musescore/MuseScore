import QtQuick 2.4
import "style.js" as Style

Item {
    property alias title: pageTitleText.text
    property alias content: pageLoader.source
    property int layoutLeftMargin: Style.layoutMargin
    property int layoutRightMargin: Style.layoutMargin
    property int layoutBottomMargin: Style.layoutMargin

    Rectangle {
        z: -1
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#24486c" }
            GradientStop { position: 1.0; color: "#040b13" }
        }
    }

    Text {
        id: pageTitleText
        text: qsTr("Text")
        color: Style.textColor
        font.pointSize: Style.pageTitleFontSize
        anchors.left: parent.left
        anchors.leftMargin: layoutLeftMargin
        anchors.rightMargin: layoutRightMargin
        anchors.top: parent.top
        anchors.topMargin: Style.layoutMargin
    }

    Rectangle {
        id: pageTitleUnderline
        height: 1
        color: Style.borderLinesColor
        anchors.left: parent.left
        anchors.leftMargin: layoutLeftMargin
        anchors.right: parent.right
        anchors.rightMargin: layoutRightMargin
        anchors.top: pageTitleText.bottom
        anchors.topMargin: 6
    }

    Loader {
        id: pageLoader
        anchors.left: parent.left
        anchors.leftMargin: layoutLeftMargin
        anchors.right: parent.right
        anchors.rightMargin: layoutRightMargin
        anchors.top: pageTitleUnderline.bottom
        anchors.topMargin: 6
        anchors.bottom: parent.bottom
        anchors.bottomMargin: layoutBottomMargin
    }
}
