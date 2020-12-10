import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

RowLayout {
    id: root

    property bool isMovingUpAvailable: false
    property bool isMovingDownAvailable: false
    property bool isRemovingAvailable: false
    property bool isAddingAvailable: value

    signal addRequested()
    signal moveUpRequested()
    signal moveDownRequested()
    signal removingRequested()

    spacing: 6

    focus: true

    Keys.onShortcutOverride: {
        if (event.key === Qt.Key_Delete) {
            root.removingRequested()
        }
    }

    FlatButton {
        Layout.fillWidth: true

        text: qsTrc("instruments", "Add")

        enabled: root.isAddingAvailable

        onClicked: {
            root.addRequested()
        }
    }

    FlatButton {
        Layout.preferredWidth: width

        enabled: root.isMovingUpAvailable

        icon: IconCode.ARROW_UP

        onClicked: {
            root.moveUpRequested()
        }
    }

    FlatButton {
        Layout.preferredWidth: width

        enabled: root.isMovingDownAvailable

        icon: IconCode.ARROW_DOWN

        onClicked: {
            root.moveDownRequested()
        }
    }

    FlatButton {
        Layout.preferredWidth: width

        enabled: root.isRemovingAvailable

        icon: IconCode.DELETE_TANK

        onClicked: {
            root.removingRequested()
        }
    }
}
