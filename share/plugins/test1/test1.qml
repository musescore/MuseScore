
import QtQuick 2.15

import MuseScore 3.0

MuseScore {

    title: "Test 1 plugin"
    pluginType: "dialog"

    width: 300
    height: 124

    Text {
        text: "Test1 plugin"
    }

    Component.onCompleted: {
        console.info("Component.onCompleted test1")
    }

    onRun: {
        console.info("onRun test1")
    }
}
