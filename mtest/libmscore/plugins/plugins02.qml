import QtQuick   2.0
import MuseScore 1.0

MuseScore {
    menuPath:    "Plugins.test2"
    version:     "2.0"
    description: "Test Plugin"

    width:  150
    height: 75
    onRun: {
        var score = curScore
        Qt.quit();
        }
    }

