import QtQuick 2.7
import QtQuick.Controls 2.2
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

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

         function setCurrentNotation(index) {
            currentIndex = index
            notationListModel.setCurrentNotation(index)
        }

        delegate: NotationSwitchButton {
            id: button

            title: model.title

            ButtonGroup.group: notationsView.radioButtonGroup
            checked: model.index === notationsView.currentIndex

            function resolveNextNotationIndex() {
                var nextIndex = model.index - 1
                if (nextIndex < 0) {
                    return 0
                }

                return nextIndex
            }

            onToggled: {
                notationsView.setCurrentNotation(model.index)
            }

            onCloseRequested: {
                if (model.index !== notationsView.currentIndex) {
                    notationListModel.closeNotation(model.index)
                    return
                }

                var index = button.resolveNextNotationIndex()
                notationListModel.closeNotation(model.index)
                notationsView.setCurrentNotation(index)
            }
        }
    }
}
