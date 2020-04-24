import QtQuick 2.9
import MuseScore.Inspectors 3.3
import "../../../common"

Column {
    id: root

    property QtObject horizontalOffset: undefined
    property QtObject verticalOffset: undefined
    property bool isSnappedToGrid: false

    signal snapToGridToggled(var snap)
    signal configureGridRequested()

    height: implicitHeight
    width: parent.width

    spacing: 16

    Column {
        spacing: 8

        height: implicitHeight
        width: parent.width

        StyledTextLabel {
            anchors.left: parent.left

            text: qsTr("Offset")
        }

        Item {
            height: childrenRect.height
            width: parent.width

            IncrementalPropertyControl {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 4

                icon: IconNameTypes.HORIZONTAL

                enabled: horizontalOffset ? horizontalOffset.isEnabled : false
                isIndeterminate: horizontalOffset && enabled ? horizontalOffset.isUndefined : false
                currentValue: horizontalOffset ? horizontalOffset.value : 0

                onValueEdited: { horizontalOffset.value = newValue }
            }

            IncrementalPropertyControl {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 4
                anchors.right: parent.right

                icon: IconNameTypes.VERTICAL

                enabled: verticalOffset ? verticalOffset.isEnabled : false
                isIndeterminate: verticalOffset && enabled ? verticalOffset.isUndefined : false
                currentValue: verticalOffset ? verticalOffset.value : 0

                onValueEdited: { verticalOffset.value = newValue }
            }
        }
    }

    CheckBox {
        id: snapToGridCheckbox

        text: qsTr("Snap to grid")

        checked: isSnappedToGrid

        onClicked: { root.snapToGridToggled(!checked) }
    }

    FlatButton {
        text: qsTr("Configure grid")

        visible: snapToGridCheckbox.checked

        onClicked: {
            root.configureGridRequested()
        }
    }
}
