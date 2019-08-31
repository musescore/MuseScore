//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

import QtQuick 2.8
import QtQuick.Controls 2.1
import QtQml.Models 2.2
import MuseScore.Palette 3.3

StyledPopup {
    id: moreElementsPopup

    property var poolPalette : null
    property var poolPaletteRootIndex: null
    property PaletteController poolPaletteController: null

    property var customPalette : null
    property var customPaletteRootIndex: null
    property PaletteController customPaletteController: null

    property string paletteName
    readonly property string libraryPaletteName: (poolPalette && poolPaletteRootIndex) ? poolPalette.data(poolPaletteRootIndex, Qt.DisplayRole) : ""
    property bool paletteIsCustom: false

    property size cellSize
    property bool drawGrid

    signal addElementsRequested(var mimeDataList)

    Column {
        width: parent.width
        spacing: 8

        Button {
            width: parent.width

            text: qsTr("Add to %1").arg(paletteName)
            enabled: masterPaletteSelectionModel.hasSelection || customPaletteSelectionModel.hasSelection

            onClicked: {
                function collectMimeData(palette, selection) {
                    const selectedList = selection.selectedIndexes;
                    var mimeArr = [];
                    for (var i = 0; i < selectedList.length; i++) {
                        const mimeData = palette.paletteModel.data(selectedList[i], PaletteTreeModel.MimeDataRole);
                        mimeArr.push(mimeData);
                    }
                    return mimeArr;
                }

                const mimeMasterPalette = collectMimeData(masterPalette, masterPaletteSelectionModel);
                const mimeCustomPalette = collectMimeData(customPalette, customPaletteSelectionModel);

                masterPaletteSelectionModel.clear();
                customPaletteSelectionModel.clear();

                if (mimeMasterPalette.length)
                    addElementsRequested(mimeMasterPalette);
                if (mimeCustomPalette.length)
                    addElementsRequested(mimeCustomPalette);
            }
        }

        Item {
            id: masterIndexControls
            enabled: moreElementsPopup.paletteIsCustom && poolPalette && poolPaletteRootIndex
            visible: enabled
            anchors { left: parent.left; right: parent.right }
            implicitHeight: prevButton.implicitHeight
            Button {
                id: prevButton
                width: height
                anchors.left: parent.left
                flat: true
                text: "<" // TODO: replace?

                property var prevIndex: masterIndexControls.enabled ? poolPalette.sibling(poolPaletteRootIndex.row - 1, 0, poolPaletteRootIndex) : null
                enabled: prevIndex && prevIndex.valid

                onClicked: poolPaletteRootIndex = prevIndex;
            }
            Text {
                anchors.centerIn: parent
                text: moreElementsPopup.libraryPaletteName
            }
            Button {
                width: height
                anchors.right: parent.right
                flat: true
                text: ">" // TODO: replace?

                property var nextIndex: masterIndexControls.enabled ? poolPalette.sibling(poolPaletteRootIndex.row + 1, 0, poolPaletteRootIndex) : null
                enabled: nextIndex && nextIndex.valid

                onClicked: poolPaletteRootIndex = nextIndex
            }
        }

        Rectangle {
            width: parent.width
            height: childrenRect.height
            border { width: 1; color: "black" }

            Column {
                width: parent.width
                padding: 8
                property real contentWidth: width - 2 * padding

                ItemSelectionModel {
                    id: masterPaletteSelectionModel
                    model: masterPalette.paletteModel
                }

                Palette {
                    id: masterPalette
                    height: 100
                    maxWidth: parent.contentWidth

                    // TODO: change settings to "hidden" model?
                    cellSize: moreElementsPopup.cellSize
                    drawGrid: moreElementsPopup.drawGrid

                    paletteModel: moreElementsPopup.poolPalette
                    paletteRootIndex: moreElementsPopup.poolPaletteRootIndex
                    paletteController: moreElementsPopup.poolPaletteController
                    selectionModel: masterPaletteSelectionModel
                }

                ToolSeparator {
                    visible: !customPalette.empty
                    orientation: Qt.Horizontal
                    width: parent.contentWidth
                }

                Text {
                    id: customPaletteLabel
                    visible: !customPalette.empty
                    text: qsTr("Custom")
                }

                ItemSelectionModel {
                    id: customPaletteSelectionModel
                    model: customPalette.paletteModel
                }

                Palette {
                    id: customPalette
                    visible: !empty
                    height: 50
                    maxWidth: parent.contentWidth

                    cellSize: control.cellSize
                    drawGrid: control.drawGrid

                    paletteModel: moreElementsPopup.customPalette
                    paletteRootIndex: moreElementsPopup.customPaletteRootIndex
                    paletteController: moreElementsPopup.customPaletteController
                    selectionModel: customPaletteSelectionModel
                }
            }
        }

        Text {
            width: parent.width
            text: qsTr("Drag items to the palette or directly on your score")
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            // make this label's font slightly smaller than other popup text
            font.pointSize: Qt.application.font.pointSize * 0.8
        }
    }
}
