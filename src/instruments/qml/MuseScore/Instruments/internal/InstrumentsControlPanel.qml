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

    property alias keynav: keynavSub

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

    KeyNavigationSubSection {
        id: keynavSub
        name: "InstrumentsHeader"
    }

    FlatButton {
        Layout.fillWidth: true

        keynav.subsection: keynavSub
        keynav.order: 1

        text: qsTrc("instruments", "Add")

        enabled: root.isAddingAvailable

        onClicked: {
            root.addRequested()
        }
    }

    FlatButton {
        Layout.preferredWidth: width

        keynav.subsection: keynavSub
        keynav.order: 2

        enabled: root.isMovingUpAvailable

        icon: IconCode.ARROW_UP

        onClicked: {
            root.moveUpRequested()
        }
    }

    FlatButton {
        Layout.preferredWidth: width

        keynav.subsection: keynavSub
        keynav.order: 3

        enabled: root.isMovingDownAvailable

        icon: IconCode.ARROW_DOWN

        onClicked: {
            root.moveDownRequested()
        }
    }

    FlatButton {
        Layout.preferredWidth: width

        keynav.subsection: keynavSub
        keynav.order: 4

        enabled: root.isRemovingAvailable

        icon: IconCode.DELETE_TANK

        onClicked: {
            root.removingRequested()
        }
    }
}
