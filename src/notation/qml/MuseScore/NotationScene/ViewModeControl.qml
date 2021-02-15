import QtQuick 2.7
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

StyledComboBox {
    id: root
    
    opensUpward: true
    maxVisibleItemCount: 3

    textRoleName: "nameRole"
    valueRoleName: "idRole"
    currentIndex: indexOfValue(model.currentViewModeId)

    onValueChanged: {
        model.currentViewModeId = value
    }
    
    model: ViewModeControlModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }
}
