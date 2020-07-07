import QtQuick 2.7
import MuseScore.Shortcuts 1.0

Rectangle {

    id: root

    color: "#0F9D58"

    //! NOTE Need only create
    Shortcuts {}

    Loader {
        id: windowLoader
        anchors.fill: parent
        onStatusChanged: {
            item.anchors.fill = item.parent
            item.visible = true
        }
    }

    Component.onCompleted: {
        var comp = Qt.createComponent("Window.qml");
        if (comp.status !== Component.Ready) {
            console.debug("qml: show window error: " + comp.errorString())
        }
        windowLoader.sourceComponent = comp
    }

}
