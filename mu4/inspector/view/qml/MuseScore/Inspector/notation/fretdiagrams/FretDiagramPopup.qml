import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import "../../common/"
import "internal"

StyledPopup {
    id: root

    property alias model: fretDiagramTabPanel.model

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        FretDiagramTabPanel {
            id: fretDiagramTabPanel

            width: parent.width
        }

        Column {
            height: childrenRect.height
            width: parent.width

            spacing: 12

            FlatButton {
                width: parent.width

                visible: root.model ? root.model.areSettingsAvailable : false

                text: qsTr("Reset")

                onClicked: {
                    fretCanvas.clear()
                }
            }

            FretCanvas {
                id: fretCanvas

                diagram: root.model ? root.model.fretDiagram : null
                isBarreModeOn: root.model ? root.model.isBarreModeOn : false
                isMultipleDotsModeOn: root.model ? root.model.isMultipleDotsModeOn : false
                currentFretDotType: root.model ? root.model.currentFretDotType : false
                visible: root.model ? root.model.areSettingsAvailable : false

                width: parent.width
            }
        }
    }

    StyledTextLabel {
        anchors.fill: parent

        wrapMode: Text.Wrap
        text: qsTr("You have multiple fretboard diagrams selected. Select a single diagram to edit its settings")
        visible: root.model ? !root.model.areSettingsAvailable : false
    }
}
