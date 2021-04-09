import QtQuick 2.15
import QtQuick.Controls 2.15

import "./docksystem"
import "./HomePage"

import com.kdab.dockwidgets 1.0 as KDDW

ApplicationWindow {
    id: root

    width: 800
    height: 600

    visible: true

    title: qsTrc("appshell", "MuseScore 4")
}
