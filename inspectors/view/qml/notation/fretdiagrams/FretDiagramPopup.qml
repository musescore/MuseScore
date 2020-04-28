import QtQuick 2.9
import QtQuick.Controls 2.2
import "../../common/"

StyledPopup {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    height: implicitHeight
    width: parent.width

    Component.onCompleted: {
        model.onSelectionChanged.connect(fretCanvas.selectionChanged)
    }

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        FretCanvas {
            id: fretCanvas

            visible: model ? model.canvasVisible() : false

            model: root.model

            signal selectionChanged

            onSelectionChanged: {
                visible = model ? model.canvasVisible() : false
            }
        }
    }
}
