import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

Item {
    id:root

    property var model

    QtObject {
        id: privateProperties

        property var leftMargin: 12
    }

    Column {
        id: header

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: privateProperties.leftMargin

        spacing: 16

        Row {
            width: parent.width
            height: childrenRect.height

            CheckBox {
                id: selectAllScores

                onClicked: {
                    checked = !checked
                    isIndeterminate = false

                    root.model.toggleAllSelections(checked);
                }

                width: parent.width * 1 / 6
                height: checkBoxHeight

                anchors.left: parent.left
                anchors.leftMargin: (width - checkBoxWidth) / 2
                anchors.verticalCenter: parent.verticalCenter
            }

            StyledTextLabel {
                text: "Scores to export"

                width: parent.width * 5 / 6

                font.family: ui.theme.bodyBoldFont
                horizontalAlignment: Qt.AlignLeft
                font.capitalization: Font.AllUppercase

                anchors.left: selectAllScores.right
            }
        }
    }

    RoundedRectangle {
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: privateProperties.leftMargin
        anchors.rightMargin: 8
        anchors.topMargin: 6

        color: ui.theme.backgroundPrimaryColor

        ListView {
            id: scoresButtonGroup

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.width

            spacing: 0

            model: root.model

            boundsBehavior: Flickable.StopAtBounds
            interactive: height < contentHeight
            clip: true

            ScrollBar.vertical: StyledScrollBar {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.rightMargin: 8
            }

            Connections {
                target: root.model

                function onSelectionChanged() {
                    var selections = root.model.selectionLength();

                    selectAllScores.checked = selections === root.model.rowCount();
                    selectAllScores.isIndeterminate = selections > 0 && !selectAllScores.checked;
                }
            }

            delegate: ExportScoreListDelegate {
                title: model.title
                isSelected: model.isSelected
                isMain: model.isMain

                onScoreClicked: {
                    root.model.toggleSelection(model.index);
                }
            }
        }
    }
}
