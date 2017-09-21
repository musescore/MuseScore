import QtQuick 2.8
import MuseScore 3.0


MuseScore {
    menuPath:    "Plugins.helloQml"
    version:     "2.0"
    description: qsTr("This demo plugin shows some basic tasks.")
    pluginType: "dialog"

    width:  150
    height: 75
    onRun: {
        console.log(qsTr("hello world"));
        if (typeof curScore === 'undefined')
            Qt.quit();

        var score = curScore
        console.log(curScore)
        console.log(score.title)
        var m;
        m = score.firstMeasure;
        while (m) {
            console.log(qsTr("measure"))
            var segment = m.first()
            while (segment) {
                var element
                element = segment.elementAt(0);
                if (element && element.type == Ms.CHORD) {
                    console.log(qsTr("    element"));
                    var notes=element.notes;
                    if (notes.length > 0) {
                        console.log(qsTr("  notes=")+notes.length);
                    }
                }    
                segment = segment.next();
                }
            m = m.nextMeasure();
            }
        }
     
    Rectangle {
        color: "grey"
        anchors.fill: parent

        Text {
            anchors.centerIn: parent
            text: qsTr("Hello Qml")
            }

        MouseArea {
            anchors.fill: parent
            onClicked: Qt.quit()
            }
        }
    }

