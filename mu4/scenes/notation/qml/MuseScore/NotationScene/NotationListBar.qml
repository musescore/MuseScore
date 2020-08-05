import QtQuick 2.7
import QtQuick.Controls 2.2
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Rectangle {
    id: root

    property alias notationsCount: notationsView.count

    height: 30

    NotationListModel {
        id: notationListModel
    }

    Component.onCompleted: {
        notationListModel.load()
    }

    RadioButtonGroup {
        id: notationsView

        width: contentWidth

        model: notationListModel
        currentIndex: 0

        delegate: FlatRadioButton {
            ButtonGroup.group: notationsView.radioButtonGroup
            checked: model.index === notationsView.currentIndex

            onToggled: {
                parent.currentIndex = model.index
                notationListModel.setCurrentNotation(parent.currentIndex)
            }

            StyledTextLabel {
                anchors.fill: parent
                text: model.title
            }
        }
    }
}
