import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

import "../common"

InspectorSectionView {
    id: root

    implicitHeight: contentRow.implicitHeight

    RowLayout {
        id: contentRow

        width: parent.width

        spacing: 4

        BarSettings {
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width * 0.75

            popupPositionX: mapToGlobal(contentRow.x, contentRow.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model
            onPopupContentHeightChanged: {
                updateContentHeight(popupContentHeight)
            }
        }

        FlatButton {
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width * 0.25

            icon: IconCode.DELETE_TANK

            visible: root.model ? !root.model.isEmpty : false

            onClicked: {
                if (model) {
                    model.removeSelectedBars()
                }
            }
        }
    }
}
