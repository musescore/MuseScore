import QtQuick 2.7
import QtQuick.Controls 2.2
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    property alias notationsCount: notationsView.count

    height: 30

    NotationSwitchListModel {
        id: notationSwitchModel
    }

    Component.onCompleted: {
        notationSwitchModel.load()
    }

    RadioButtonGroup {
        id: notationsView

        width: contentWidth

        model: notationSwitchModel
        currentIndex: 0
        spacing: 0

         function setCurrentNotation(index) {
            currentIndex = index
            notationSwitchModel.setCurrentNotation(index)
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
                    notationSwitchModel.closeNotation(model.index)
                    return
                }

                var index = button.resolveNextNotationIndex()
                notationSwitchModel.closeNotation(model.index)
                notationsView.setCurrentNotation(index)
            }
        }
    }
}
