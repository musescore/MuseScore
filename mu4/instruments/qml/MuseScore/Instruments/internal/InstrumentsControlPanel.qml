import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

RowLayout {
    id: root

    property bool isMovingUpAvailable: false
    property bool isMovingDownAvailable: false
    property bool isRearrangementAvailable: false
    property bool isRemovingAvailable: false

    signal moveUpRequested()
    signal moveDownRequested()
    signal removingRequested()

    anchors.left: parent.left
    anchors.right: parent.right

    spacing: 8

    FlatButton {
        Layout.fillWidth: true
        text: qsTrc("instruments", "Add")

        onClicked: {

        }
    }

    FlatButton {
        Layout.preferredWidth: implicitWidth

        enabled: root.isMovingUpAvailable

        icon: IconCode.ARROW_UP

        onClicked: {
            root.moveUpRequested()
        }
    }

    FlatButton {
        Layout.preferredWidth: implicitWidth

        enabled: root.isMovingDownAvailable

        icon: IconCode.ARROW_DOWN

        onClicked: {
            root.moveDownRequested()
        }
    }

    FlatButton {
        Layout.preferredWidth: implicitWidth

        enabled: root.isRemovingAvailable

        icon: IconCode.DELETE_TANK

        onClicked: {
            root.removingRequested()
        }
    }
}
