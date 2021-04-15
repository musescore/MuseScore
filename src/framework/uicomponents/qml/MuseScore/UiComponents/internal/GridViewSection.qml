import QtQuick 2.15

Item {
    id: root
    property alias sectionDelegate: loader.sourceComponent

    Loader {
        id: loader
        anchors.fill: parent

        property var itemModel: null
        property int itemIndex: 0

        onLoaded: {
            itemModel = Qt.binding( function() { return Boolean(modelData) ? modelData : null });
            itemIndex = index
            root.visible = Qt.binding( function() { return Boolean(item) ? item.visible : false })
        }
    }
}
