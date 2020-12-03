import QtQuick 2.9
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0
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

    InspectorPropertyView {
        titleText: qsTr("Offset")
        propertyItem: horizontalOffset

        Item {
            height: childrenRect.height
            width: parent.width

            IncrementalPropertyControl {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 4

                icon: IconCode.HORIZONTAL

                enabled: horizontalOffset ? horizontalOffset.isEnabled : false
                isIndeterminate: horizontalOffset && enabled ? horizontalOffset.isUndefined : false
                currentValue: horizontalOffset ? horizontalOffset.value : 0

                onValueEdited: { horizontalOffset.value = newValue }
            }

            IncrementalPropertyControl {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 4
                anchors.right: parent.right

                icon: IconCode.VERTICAL

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
        width: parent.width

        text: qsTr("Configure grid")

        visible: snapToGridCheckbox.checked

        onClicked: {
            root.configureGridRequested()
        }
    }
}
