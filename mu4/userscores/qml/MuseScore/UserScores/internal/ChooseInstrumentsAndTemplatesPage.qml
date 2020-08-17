import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0
import MuseScore.Instruments 1.0

Item {
    id: root

    function result() {
        var result = {}

        if (pagesStack.currentIndex === 0) {
            result["instrumentIds"] = instrumentsPage.selectedInstrumentIds()
        } else if (pagesStack.currentIndex === 1) {
            result["templatePath"] = templatePage.selectedTemplatePath
        }

        return result
    }

    TabBar {
        id: bar

        anchors.top: parent.top
        anchors.topMargin: 24
        anchors.horizontalCenter: parent.horizontalCenter

        contentHeight: 28
        spacing: 0

        StyledTabButton {
            text: qsTrc("userscores", "Choose instruments")
            sideMargin: 22
            isCurrent: bar.currentIndex === 0
        }
        StyledTabButton {
            text: qsTrc("userscores", "Choose from template")
            sideMargin: 22
            isCurrent: bar.currentIndex === 1
        }
    }

    StackLayout {
        id: pagesStack
        anchors.top: bar.bottom
        anchors.topMargin: 24
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentIndex: bar.currentIndex

        ChooseInstrumentsPage {
            id: instrumentsPage

            anchors.fill: parent
        }

        ChooseTemplatePage {
            id: templatePage

            anchors.fill: parent
        }
    }
}
