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

import "utils.js" as Utils

StyledPopup {
    id: moreElementsPopup

    property var poolPalette : null
    property var poolPaletteRootIndex: null
    property PaletteController poolPaletteController: null

    property var customPalette : null
    property var customPaletteRootIndex: null
    property PaletteController customPaletteController: null

    property PaletteElementEditor elementEditor: customPaletteRootIndex && customPaletteController ? customPaletteController.elementEditor(customPaletteRootIndex) : null

    property string paletteName
    readonly property string libraryPaletteName: (poolPalette && poolPaletteRootIndex) ? poolPalette.data(poolPaletteRootIndex, Qt.DisplayRole) : ""
    property bool paletteIsCustom: false
    property bool paletteEditingEnabled: true

    property size cellSize
    property bool drawGrid

    property int maxHeight: 400
    implicitHeight: column.height + topPadding + bottomPadding

    property bool enablePaletteAnimations: false // disabled by default to avoid unnecessary "add" animations on opening this popup at first time
    property bool pinned: false

    signal addElementsRequested(var mimeDataList)
    signal pinPopupRequested(bool pin)

    Column {
        id: column
        width: parent.width
        spacing: 8


        Row {
            StyledButton {
                id: addToPaletteButton
                width: column.width - pinButton.width

                text: qsTr("Add to %1").arg(paletteName)
                enabled: moreElementsPopup.paletteEditingEnabled && (masterPaletteSelectionModel.hasSelection || customPaletteSelectionModel.hasSelection)

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


            StyledToolButton {
                id: pinButton
                height: addToPaletteButton.height
                width: height
                checkable: true
                checked: moreElementsPopup.pinned
                flat: !checked

                onCheckedChanged: moreElementsPopup.pinPopupRequested(checked)

                padding: 4

                contentItem: StyledIcon {
                    source: "icons/pin.png"
                }
            }
        }

        Item {
            id: masterIndexControls
            enabled: moreElementsPopup.paletteIsCustom && poolPalette && poolPaletteRootIndex
            visible: enabled
            anchors { left: parent.left; right: parent.right }
            implicitHeight: prevButton.implicitHeight
            StyledButton {
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
                font: globalStyle.font
                color: globalStyle.windowText
            }
            StyledButton {
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
            id: paletteContainer
            width: parent.width
            height: childrenRect.height
            border { width: 1; color: "black" }
            color: mscore.paletteBackground

            readonly property int availableHeight: moreElementsPopup.maxHeight - addToPaletteButton.height - (masterIndexControls ? masterIndexControls.height : 0) - bottomText.height - (elementEditorButton.visible ? elementEditorButton.height : 0) - 40

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
                    height: Math.max(
                                cellSize.height,
                                Math.min(
                                    implicitHeight,
                                    paletteContainer.availableHeight - (customPalette.visible ? (customPalette.height + customPaletteLabel.height) : 0)
                                    )
                                )
                    width: parent.contentWidth

                    // TODO: change settings to "hidden" model?
                    cellSize: moreElementsPopup.cellSize
                    drawGrid: moreElementsPopup.drawGrid

                    paletteModel: moreElementsPopup.poolPalette
                    paletteRootIndex: moreElementsPopup.poolPaletteRootIndex
                    paletteController: moreElementsPopup.poolPaletteController
                    selectionModel: masterPaletteSelectionModel

                    enableAnimations: moreElementsPopup.enablePaletteAnimations
                }

                ToolSeparator {
                    id: separator
                    visible: !customPalette.empty
                    orientation: Qt.Horizontal
                    width: parent.contentWidth
                }

                Item {
                    width: separator.width
                    implicitHeight: customPaletteLabel.implicitHeight
                    visible: !customPalette.empty

                    Text {
                        id: customPaletteLabel
                        text: qsTr("Custom")
                    }

                    StyledToolButton {
                        id: deleteButton
                        height: customPaletteLabel.height
                        width: height
                        anchors.right: parent.right
                        text: qsTr("Delete element(s)")
                        enabled: customPaletteSelectionModel.hasSelection

                        ToolTip.visible: hovered
                        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                        ToolTip.text: text

                        onHoveredChanged: {
                            if (hovered) {
                                mscore.tooltip.item = deleteButton;
                                mscore.tooltip.text = deleteButton.texst;
                            } else if (mscore.tooltip.item == deleteButton)
                                mscore.tooltip.item = null;
                        }

                        padding: 0

                        contentItem: StyledIcon {
                            source: "icons/delete.png"
                            color: "black"
                            opacity: deleteButton.enabled ? 1.0 : 0.3
                        }

                        onClicked: Utils.removeSelectedItems(moreElementsPopup.customPalette, moreElementsPopup.customPaletteController, customPaletteSelectionModel, moreElementsPopup.customPaletteRootIndex);
                    }
                }

                ItemSelectionModel {
                    id: customPaletteSelectionModel
                    model: customPalette.paletteModel
                }

                Palette {
                    id: customPalette
                    visible: !empty
                    width: parent.contentWidth

                    cellSize: control.cellSize
                    drawGrid: control.drawGrid

                    paletteModel: moreElementsPopup.customPalette
                    paletteRootIndex: moreElementsPopup.customPaletteRootIndex
                    paletteController: moreElementsPopup.customPaletteController
                    selectionModel: customPaletteSelectionModel

                    enableAnimations: moreElementsPopup.enablePaletteAnimations
                }
            }
        }

        Text {
            id: bottomText
            width: parent.width
            text: qsTr("Drag items to the palette or directly on your score")
            color: globalStyle.windowText
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            font.family: globalStyle.font.family
            // make this label's font slightly smaller than other popup text
            font.pointSize: globalStyle.font.pointSize * 0.8
        }

        StyledButton {
            id: elementEditorButton
            visible: moreElementsPopup.elementEditor && moreElementsPopup.elementEditor.valid
            enabled: moreElementsPopup.paletteEditingEnabled
            width: parent.width
            text: moreElementsPopup.elementEditor ? moreElementsPopup.elementEditor.actionName : ""
            onClicked: moreElementsPopup.elementEditor.open()
        }
    }
}
