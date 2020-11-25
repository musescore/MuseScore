import QtQuick 2.12

import "internal"

Item {
    id: root

    property var model: null
    property int orientation: Qt.Horizontal

    property Component sectionDelegate: Item {}
    property Component itemDelegate: Item {}

    property int cellWidth: 0
    property int cellHeight: 0

    property int sectionWidth: 0
    property int sectionHeight: 0
    property string sectionRole: "sectionRole"

    readonly property int noLimit: -1
    property int rows: noLimit
    property int rowSpacing: 2
    property int columns: noLimit
    property int columnSpacing: 2

    QtObject {
        id: privateProperties

        function modelSections() {
            var _sections = []

            for (var i = 0; i < root.model.count; i++) {
                var element = root.model.get(i)

                var section = element[sectionRole]
                if (!_sections.includes(section)) {
                    _sections.push(section)
                }
            }

            return _sections
        }
    }

    Loader {
        anchors.fill: parent
        sourceComponent: orientation === Qt.Horizontal ? horizontalView : verticalView
    }

    Component {
        id: horizontalView

        Row {
            Repeater {
                model: Boolean(root.model) ? privateProperties.modelSections() : []

                Row {
                    spacing: 2

                    GridViewSection {
                        width: root.sectionWidth
                        height: root.sectionHeight

                        sectionDelegate: root.sectionDelegate
                    }

                    GridViewDelegate {
                        anchors.verticalCenter: parent.verticalCenter

                        model: Boolean(root.model) ? root.model : null

                        itemDelegate: root.itemDelegate
                        sectionRole: root.sectionRole

                        cellWidth: root.cellWidth
                        cellHeight: root.cellHeight

                        sectionWidth: root.sectionWidth
                        sectionHeight: root.sectionHeight

                        rows: root.rows
                        rowSpacing: root.rowSpacing
                        columns: root.columns
                        columnSpacing: root.columnSpacing
                    }
                }
            }
        }
    }

    Component {
        id: verticalView

        Column {
            Repeater {
                model: Boolean(root.model) ? privateProperties.modelSections() : []

                Column {
                    spacing: 2

                    GridViewSection {
                        anchors.left: parent.left
                        anchors.leftMargin: root.columnSpacing

                        width: root.sectionWidth
                        height: root.sectionHeight

                        sectionDelegate: root.sectionDelegate
                    }

                    GridViewDelegate {
                        anchors.horizontalCenter: parent.horizontalCenter

                        model: Boolean(root.model) ? root.model : null

                        itemDelegate: root.itemDelegate
                        sectionRole: root.sectionRole

                        cellWidth: root.cellWidth
                        cellHeight: root.cellHeight

                        sectionWidth: root.sectionWidth
                        sectionHeight: root.sectionHeight

                        rows: root.rows
                        rowSpacing: root.rowSpacing
                        columns: root.columns
                        columnSpacing: root.columnSpacing
                    }
                }
            }
        }
    }
}
