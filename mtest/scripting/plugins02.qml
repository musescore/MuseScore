import QtQuick   2.0
import MuseScore 3.0

MuseScore {
    menuPath:    "Plugins.test3"
    version:     "3.0"
    description: "Test Plugin"

    width:  150
    height: 75
    onRun: {
        var score = curScore
        Qt.quit();
        }
    }

