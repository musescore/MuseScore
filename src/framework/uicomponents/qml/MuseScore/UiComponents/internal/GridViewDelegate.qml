import QtQuick 2.12

import MuseScore.UiComponents 1.0

Item {
    id: root

    property var model: null

    property Component itemDelegate: Item {}
    property string sectionRole: "sectionRole"

    property int cellWidth: 0
    property int cellHeight: 0

    property int sectionWidth: 0
    property int sectionHeight: 0

    property int rows: 0
    property int rowSpacing: 2
    property int columns: 0
    property int columnSpacing: 2

    width: gridView.width
    height: gridView.height

    FilterProxyModel {
        id: filterModel

        sourceModel: root.model

        filters: [
            FilterValue {
                roleName: root.sectionRole
                roleValue: modelData
                compareType: CompareType.Equal
            }
        ]
    }

    GridView {
        id: gridView

        property int columns: root.columns !== -1 ? root.columns : gridView.count
        property int rows: root.rows !== -1 ? root.rows : Math.ceil(gridView.count / gridView.columns)

        width: gridView.columns * gridView.cellWidth
        height: gridView.rows * gridView.cellHeight

        cellWidth: root.cellWidth + root.columnSpacing * 2
        cellHeight: root.cellHeight + root.rowSpacing * 2

        model: filterModel

        interactive: false
        clip: true

        delegate: Item {
            width: gridView.cellWidth
            height: gridView.cellHeight

            Loader {
                width: root.cellWidth
                height: root.cellHeight

                anchors.centerIn: parent

                property var itemModel: null
                sourceComponent: root.itemDelegate

                onStatusChanged: {
                    if (status === Loader.Ready) {
                        itemModel = Qt.binding( function() { return Boolean(model) ? model : null });
                    }
                }
            }
        }
    }
}
