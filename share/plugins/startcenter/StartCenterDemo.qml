import QtQuick 2.12
import MuseScore 3.0
import MuseScore.Private 3.1

MuseScore {
    id: mscore
    menuPath: "Plugins." + qsTr("Start Center")
    description: "Demonstration of the new start center design"
    version: "1.0"
    requiresScore: false

    StartCenter {
        id: startcenter
        width: 800
        height: 600
        // TODO: minumum height/width?
        modality: Qt.ApplicationModal
        visible: false
    }

    onRun: startcenter.visible = true

    function openScore(url) {
        mscore.readScore(url);
        startcenter.close();
        Qt.quit();
    }

    function openScoreDialog() {
        const files = MsPrivate.getOpenScoreNames();
        if (files.length == 0)
            return;

        for (let i = 0; i < files.length; ++i)
            mscore.readScore(files[i]);

        startcenter.close();
        Qt.quit();
    }
}
