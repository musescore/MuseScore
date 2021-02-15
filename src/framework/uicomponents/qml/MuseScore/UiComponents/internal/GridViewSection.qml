import QtQuick 2.12

Item {
    property Component sectionDelegate: Item {}

    Loader {
        anchors.fill: parent

        property var itemModel: null
        property int itemIndex: 0
        sourceComponent: sectionDelegate

        onStatusChanged: {
            if (status === Loader.Ready) {
                itemModel = Qt.binding( function() { return Boolean(modelData) ? modelData : null });
                itemIndex = index
            }
        }
    }
}
