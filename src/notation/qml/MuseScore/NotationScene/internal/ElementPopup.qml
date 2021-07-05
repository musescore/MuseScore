import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls 1.3

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0

StyledPopup {
    id: root

    property var model;
    property string type;

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    leftPadding: 10
    rightPadding: 10

    function isVisible(type) {
        return type === root.type;
    }

    function popupWidth() {
        return root.width - root.leftPadding - root.rightPadding;
    }

    function preOpening() {
        switch(root.type) {
        case "Tempo":
            tempoPopup.parseEquation();
            tempoPopup.setActiveTab();
            break;
        default:
            break;
        }
    }

    TempoPopup {
        id: tempoPopup

        model: root.model.modelByType(Inspector.TYPE_TEMPO)
        visible: isVisible("Tempo")
    }
}
