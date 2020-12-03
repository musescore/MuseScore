import QtQuick 2.12

Item {
    property Component sectionDelegate: Item {}

    Loader {
        anchors.fill: parent

        property var itemModel: null
        sourceComponent: sectionDelegate

        onStatusChanged: {
            if (status === Loader.Ready) {
                itemModel = Qt.binding( function() { return Boolean(modelData) ? modelData : null });
            }
        }
    }
}
