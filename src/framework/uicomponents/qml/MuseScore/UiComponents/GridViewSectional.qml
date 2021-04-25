/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.12

import "internal"

Item {
    id: root

    property var model: null
    property int orientation: Qt.Horizontal
    readonly property bool isHorizontal: orientation === Qt.Horizontal

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

        property int spacingBeforeSection: isHorizontal ? columnSpacing : rowSpacing
        property int spacingAfterSection: spacingBeforeSection

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

    property int idealWidth: {
        if (!isHorizontal) {
            // TODO: This calculation is incorrect, because `columns` does not represent
            // the actual number of columns, calculated in GridViewDelegate.qml.
            return Math.max(cellWidth * columns + columnSpacing * (columns - 1), sectionWidth)
        }

        let result = 0
        let lastSection = ""
        let currentRow = 0

        for (let i = 0; i < root.model.count; i++) {
            let element = root.model.get(i)

            let section = element[sectionRole]
            let justAddedSection = false

            if (lastSection !== section) {
                lastSection = section
                justAddedSection = true
                currentRow = 0

                if (i !== 0) {
                    result += privateProperties.spacingBeforeSection
                }

                result += sectionWidth
                result += privateProperties.spacingAfterSection
            }

            if (currentRow === 0) {
                if (!justAddedSection) {
                    result += columnSpacing
                }

                result += cellWidth
            }

            currentRow++;
            if (currentRow === root.rows) {
                currentRow = 0;
            }
        }

        return result
    }

    property int idealHeight: {
        if (isHorizontal) {
            // TODO: This calculation is incorrect, because `rows` does not represent
            // the actual number of rows, calculated in GridViewDelegate.qml.
            return Math.max(cellHeight * rows + rowSpacing * (rows - 1), sectionHeight)
        }

        let result = 0
        let lastSection = ""
        let currentColumn = 0

        for (let i = 0; i < root.model.count; i++) {
            let element = root.model.get(i)

            let section = element[sectionRole]
            let justAddedSection = false

            if (lastSection !== section) {
                lastSection = section
                justAddedSection = true
                currentColumn = 0

                if (i !== 0) {
                    result += privateProperties.spacingBeforeSection
                }

                result += sectionHeight
                result += privateProperties.spacingAfterSection
            }

            if (currentColumn === 0) {
                if (!justAddedSection) {
                    result += rowSpacing
                }

                result += cellHeight
            }

            currentColumn++;
            if (currentColumn === root.columns) {
                currentColumn = 0;
            }
        }

        return result
    }

    Loader {
        anchors.fill: parent
        sourceComponent: isHorizontal ? horizontalView : verticalView
    }

    Component {
        id: horizontalView

        Row {
            spacing: privateProperties.spacingBeforeSection

            Repeater {
                model: Boolean(root.model) ? privateProperties.modelSections() : []

                Row {
                    spacing: privateProperties.spacingAfterSection
                    height: parent.height

                    GridViewSection {
                        anchors.verticalCenter: parent.verticalCenter

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
            spacing: privateProperties.spacingBeforeSection

            Repeater {
                model: Boolean(root.model) ? privateProperties.modelSections() : []

                Column {
                    spacing: privateProperties.spacingAfterSection
                    width: parent.width

                    GridViewSection {
                        anchors.horizontalCenter: parent.horizontalCenter

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
