import QtQuick 2.7
import QtQuick.Controls 2.2

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    height: 30
    visible: notationsView.count > 0
    color: ui.theme.backgroundSecondaryColor

    border.width: 1
    border.color: ui.theme.strokeColor

    NotationSwitchListModel {
        id: notationSwitchModel

        onCurrentNotationIndexChanged: {
            notationsView.currentIndex = index
        }
    }

    Component.onCompleted: {
        notationSwitchModel.load()
    }

    RadioButtonGroup {
        id: notationsView

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 1

        width: Math.min(contentWidth, parent.width)

        model: notationSwitchModel
        currentIndex: 0
        spacing: 0
        interactive: width < contentWidth
        boundsBehavior: Flickable.StopAtBounds

        delegate: NotationSwitchButton {
            id: button

            title: model.title
            needSave: model.needSave

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
                notationSwitchModel.setCurrentNotation(model.index)
            }

            onCloseRequested: {
                if (model.index !== notationsView.currentIndex) {
                    notationSwitchModel.closeNotation(model.index)
                    return
                }

                var index = button.resolveNextNotationIndex()
                notationSwitchModel.closeNotation(model.index)
                notationSwitchModel.setCurrentNotation(index)
            }
        }
    }
}
