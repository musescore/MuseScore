import QtQuick 2.7

Rectangle {

    id: root

    color: "#0F9D58"

    Loader {
        id: windowLoader
        anchors.fill: parent
        onStatusChanged: {
            if (item && item.anchors) item.anchors.fill = item ? item.parent : null
            item.visible = true
        }
    }

    Component.onCompleted: {
        var comp = Qt.createComponent("Window.KDAB.qml");
        if (comp.status !== Component.Ready) {
            console.debug("qml: show window error: " + comp.errorString())
        }
        windowLoader.sourceComponent = comp
    }

}
