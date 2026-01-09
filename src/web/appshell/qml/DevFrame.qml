
import QtQuick

import Muse.UiComponents
import MuseScore.AppShell

Item {
    width: 200; height: 200

    DevTestsModel {
        id: testModel
    }

    FlatButton {
        text: "Btn1"
        onClicked: testModel.click1()
    }
}
