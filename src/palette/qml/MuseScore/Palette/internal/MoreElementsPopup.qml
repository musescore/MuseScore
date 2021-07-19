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

import QtQuick 2.8
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.12
import QtQml.Models 2.2

import MuseScore.Palette 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

import "utils.js" as Utils

StyledPopupView {
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
    contentHeight: column.implicitHeight
    contentWidth: 300

    property bool enablePaletteAnimations: false // disabled by default to avoid unnecessary "add" animations on opening this popup at first time

    signal addElementsRequested(var mimeDataList)

    navigation.name: "MoreElementsPopup"
    navigation.direction: NavigationPanel.Both

    onOpened: {
        masterPalette.focusFirstItem()
    }

    Column {
        id: column
        width: parent.width
        spacing: 12

        FlatButton {
            id: addToPaletteButton
            width: parent.width

            text: qsTrc("palette", "Add to %1").arg(paletteName)
            enabled: moreElementsPopup.paletteEditingEnabled && (masterPaletteSelectionModel.hasSelection || customPaletteSelectionModel.hasSelection)

            navigation.panel: moreElementsPopup.navigation
            navigation.name: "addToPaletteButton"
            navigation.column: 1
            navigation.row: 1

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

        RowLayout {
            id: masterIndexControls
            enabled: moreElementsPopup.paletteIsCustom && poolPalette && poolPaletteRootIndex
            visible: enabled
            anchors { left: parent.left; right: parent.right }

            FlatButton {
                id: prevButton
                width: height
                icon: IconCode.ARROW_LEFT
                normalStateColor: "transparent"
                enabled: prevIndex && prevIndex.valid

                navigation.panel: moreElementsPopup.navigation
                navigation.name: "prevButton"
                navigation.column: 1
                navigation.row: 2

                Layout.alignment: Qt.AlignLeft

                property var prevIndex: {
                    if (!masterIndexControls.enabled)
                        return null;

                    var idx = poolPalette.sibling(poolPaletteRootIndex.row - 1, 0, poolPaletteRootIndex);
                    if (!idx.valid) {
                        const nrows = poolPalette.rowCount(poolPaletteRootIndex.parent);
                        idx = poolPalette.sibling(nrows - 1, 0, poolPaletteRootIndex)
                    }
                    return idx;
                }

                onClicked: {
                    poolPaletteRootIndex = prevIndex
                }
            }

            StyledTextLabel {
                text: moreElementsPopup.libraryPaletteName

                Layout.alignment: Qt.AlignHCenter
            }

            FlatButton {
                icon: IconCode.ARROW_RIGHT
                normalStateColor: "transparent"

                Layout.alignment: Qt.AlignRight

                property var nextIndex: {
                    if (!masterIndexControls.enabled)
                        return null;

                    var idx = poolPalette.sibling(poolPaletteRootIndex.row + 1, 0, poolPaletteRootIndex);
                    if (!idx.valid)
                        idx = poolPalette.sibling(0, 0, poolPaletteRootIndex)
                    return idx;
                }

                enabled: nextIndex && nextIndex.valid

                navigation.panel: moreElementsPopup.navigation
                navigation.name: "nextButton"
                navigation.column: 1
                navigation.row: 3

                onClicked: poolPaletteRootIndex = nextIndex
            }
        }

        Rectangle {
            id: paletteContainer
            width: parent.width
            height: childrenRect.height
            border { width: 1; color: ui.theme.strokeColor }
            color: ui.theme.backgroundPrimaryColor

            readonly property int availableHeight: moreElementsPopup.maxHeight - addToPaletteButton.height - (masterIndexControls ? masterIndexControls.height : 0) - bottomText.height - (elementEditorButton.visible ? elementEditorButton.height : 0) - 40

            Column {
                padding: 1
                width: parent.width
                property real contentWidth: width - 2 * padding
                spacing: 0

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

                    ScrollBar.vertical: ScrollBar { enabled: masterPalette.height < masterPalette.implicitHeight }

                    // TODO: change settings to "hidden" model?
                    cellSize: moreElementsPopup.cellSize
                    drawGrid: moreElementsPopup.drawGrid

                    navigationPanel: moreElementsPopup.navigation
                    navigationCol: 1
                    navigationRow: 4

                    paletteModel: moreElementsPopup.poolPalette
                    paletteRootIndex: moreElementsPopup.poolPaletteRootIndex
                    paletteController: moreElementsPopup.poolPaletteController
                    selectionModel: masterPaletteSelectionModel

                    enableAnimations: moreElementsPopup.enablePaletteAnimations
                }

                Item {
                    width: parent.width
                    implicitHeight: deleteButton.implicitHeight
                    visible: !customPalette.empty

                    StyledTextLabel {
                        id: customPaletteLabel
                        height: deleteButton.height
                        text: qsTrc("palette", "Custom")
                    }

                    FlatButton {
                        id: deleteButton

                        anchors.right: parent.right
                        width: height

                        icon: IconCode.DELETE_TANK
                        enabled: customPaletteSelectionModel.hasSelection
                        normalStateColor: "transparent"

                        toolTipTitle: text

                        navigation.panel: moreElementsPopup.navigation
                        navigation.name: "deleteButton"
                        navigation.column: 1
                        navigation.row: 100 // Should be more than palette cells

                        onClicked: {
                            Utils.removeSelectedItems(moreElementsPopup.customPaletteController, customPaletteSelectionModel, moreElementsPopup.customPaletteRootIndex)
                        }
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

                    navigationPanel: moreElementsPopup.navigation
                    navigationCol: 1
                    navigationRow: 4

                    paletteModel: moreElementsPopup.customPalette
                    paletteRootIndex: moreElementsPopup.customPaletteRootIndex
                    paletteController: moreElementsPopup.customPaletteController
                    selectionModel: customPaletteSelectionModel

                    enableAnimations: moreElementsPopup.enablePaletteAnimations
                }
            }
        }

        StyledTextLabel {
            id: bottomText
            width: parent.width
            text: qsTrc("palette", "Drag items to the palette or directly on your score")
            wrapMode: Text.WordWrap
        }

        FlatButton {
            id: elementEditorButton
            visible: moreElementsPopup.elementEditor && moreElementsPopup.elementEditor.valid
            enabled: moreElementsPopup.paletteEditingEnabled
            width: parent.width
            text: moreElementsPopup.elementEditor ? moreElementsPopup.elementEditor.actionName : ""
            navigation.panel: moreElementsPopup.navigation
            navigation.name: "elementEditorButton"
            navigation.column: 1
            navigation.row: 101 // after deleteButton
            onClicked: moreElementsPopup.elementEditor.open()
        }
    }
}
