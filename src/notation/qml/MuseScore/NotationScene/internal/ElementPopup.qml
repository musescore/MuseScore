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

    width: 200

    leftPadding: 0

    function isVisible(type) {
        return type === root.type;
    }

    TempoPopup {
        model: root.model.modelByType(Inspector.TYPE_TEMPO)
        width: root.width
        visible: isVisible("Tempo")
    }
}
