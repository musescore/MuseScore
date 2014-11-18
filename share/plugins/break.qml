import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import MuseScore 1.0

MuseScore {
    menuPath: "Plugins.Break every X measures"
    version: "2.0"
    description: qsTr("This plugin will put a system break every X measures in the selection.")
    pluginType: "dialog"

    id: window
    width: 250
    height: 100
    onRun: {

    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        RowLayout {
            spacing: 6
            Label {
                id: textLabel
                text: qsTr("Number of Measures")
            }

            SpinBox {
                id: nbMeasures
                Layout.fillWidth: true
                Layout.preferredHeight: 26
                minimumValue: 0
                maximumValue: 10
                value: 4
            }
        }
        RowLayout {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            Button {
                id: buttonOk
                text: qsTr("Ok")
                onClicked: {

                    var value = nbMeasures.value
                    console.log(value)
                    var cursor = curScore.newCursor()
                    cursor.rewind(1) // go to start of selection (if any)
                    var endTick
                    console.log(cursor.tick)
                    var i = 1
                    var fullScore = false
                    if (!cursor.segment) { // no selection
                        fullScore = true
                    } else {
                        cursor.rewind(2) // go to end
                        if (cursor.tick == 0) {
                            // this happens when the selection includes
                            // the last measure of the score.
                            // rewind(2) goes behind the last segment (where
                            // there's none) and sets tick=0
                            endTick = curScore.lastSegment.tick + 1
                        } else {
                            endTick = cursor.tick // if no selection, go to end of score
                        }
                    }
                    console.log(endTick)

                    if (fullScore)
                        cursor.rewind(0) // if no selection, beginning of score
                    else
                        cursor.rewind(1) // go to start of selection

                    curScore.startCmd()
                    var m = cursor.measure
                    while (m && (fullScore || cursor.tick < endTick)) {
                        console.log(m)
                        console.log(cursor.tick)
                        if (value != 0) {
                            if (i % value == 0) {
                                m.lineBreak = true
                            } else {
                                m.lineBreak = false
                            }
                        } else {
                            m.lineBreak = false
                        }
                        cursor.nextMeasure()
                        m = cursor.measure
                        i++
                    }
                    curScore.endCmd()
                    Qt.quit()
                }
            }

            Button {
                id: buttonCancel
                text: qsTr("Cancel")
                onClicked: {
                    Qt.quit()
                }
            }
        }
    }
}
