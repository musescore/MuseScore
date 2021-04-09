import QtQuick 2.15
import QtQuick.Controls 2.15

import "./HomePage"

ApplicationWindow {
    id: root

    width: 800
    height: 600

    visible: true

    title: qsTrc("appshell", "MuseScore 4")

    HomePage {}
}
