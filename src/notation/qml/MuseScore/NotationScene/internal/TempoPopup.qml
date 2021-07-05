import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls 1.3

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0


TabPanel {
    id: tabPanel

    height: 50
    width: 200

    property QtObject model;

    Tab {
        id: firstTab

        title: "Tempo"
    }

    Tab {
        id: secondTab

        title: "Equation"

    }

    Tab {
        id: thirdTab

        title: "Code"

    }
}
