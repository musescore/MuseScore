import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0

import "../common"

InspectorSectionView {
    id: root

    implicitHeight: grid.implicitHeight

    GridLayout {
        id: grid

        width: parent.width

        columns: 2

        rowSpacing: 12
        columnSpacing: 4

        CheckBox {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width/2
            text: qsTr("Invisible")
            checked: model ? model.shouldShowInvisible : false
            onClicked: { model.shouldShowInvisible = !model.shouldShowInvisible }
        }

        CheckBox {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width/2
            text: qsTr("Unprintable")
            checked: model ? model.shouldShowUnprintable : false
            onClicked: { model.shouldShowUnprintable = !model.shouldShowUnprintable }
        }

        CheckBox {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width/2
            text: qsTr("Frames")
            checked: model ? model.shouldShowFrames : false
            onClicked: { model.shouldShowFrames = !model.shouldShowFrames }
        }

        CheckBox {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width/2
            text: qsTr("Page margins")
            checked: model ? model.shouldShowPageMargins : false
            onClicked: { model.shouldShowPageMargins = !model.shouldShowPageMargins }
        }
    }
}
