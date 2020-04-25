import QtQuick 2.7
import MuseScore.NotationScene 1.0

Rectangle {

    id: root

    function cmd(name) {
        console.info("cmd: " + name)
        view.cmd(name);
    }

    NotationPaintView {
        id: view
        anchors.fill: parent
    }
}
