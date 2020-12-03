import QtQuick 2.9
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.0


ListView {
    id: root

    property alias radioButtonGroup: buttonGroup

    implicitHeight: contentHeight
    implicitWidth: contentWidth

    spacing: 4

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled
    orientation: ListView.Horizontal
    interactive: false

    ButtonGroup {
        id: buttonGroup
    }
}
