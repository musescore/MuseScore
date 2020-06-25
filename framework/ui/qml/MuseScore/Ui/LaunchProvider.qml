import QtQuick 2.7
import MuseScore.Ui 1.0

Item {

    id: root

    signal requestedPage(var uri)

    Connections {
        target: ui._launchProvider

        onFireOpen: {
            console.log("onFireOpen: " + JSON.stringify(data))
            var uri = data.uri
            root.requestedPage(uri)
        }
    }
}
