/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import QtQuick 2.15
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.12
import QtQml.Models 2.2

import MuseScore.Palette 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

import "utils.js" as Utils

StyledPopupView {
    id: root

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

    readonly property bool isDragInProgress: masterPalette.state == "drag" || customPalette.state == "drag"

    property int maxHeight: 400
    contentHeight: column.implicitHeight
    contentWidth: 300

    property bool enablePaletteAnimations: false // disabled by default to avoid unnecessary "add" animations on opening this popup at first time

    signal addElementsRequested(var mimeDataList)

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "MoreElementsPopup"
        section: root.navigationSection
        order: 1
        direction: NavigationPanel.Both
    }

    onOpened: {
        masterPalette.focusFirstItem()

        if (masterPalette.count === 0) {
            elementEditorButton.navigation.requestActive()
        }
    }

    Column {
        id: column
        width: parent.width
        spacing: 12

        FlatButton {
            id: addToPaletteButton
            width: parent.width

            text: qsTrc("palette", "Add to %1").arg(root.paletteName)
            enabled: root.paletteEditingEnabled && (masterPaletteSelectionModel.hasSelection || customPaletteSelectionModel.hasSelection)

            navigation.panel: root.navigationPanel
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

                if (mimeMasterPalette.length) {
                    root.addElementsRequested(mimeMasterPalette)
                }

                if (mimeCustomPalette.length) {
                    root.addElementsRequested(mimeCustomPalette)
                }
            }
        }

        RowLayout {
            id: masterIndexControls
            enabled: root.paletteIsCustom && poolPalette && poolPaletteRootIndex
            visible: enabled
            anchors.left: parent.left
            anchors.right: parent.right

            FlatButton {
                id: prevButton
                width: height
                icon: IconCode.ARROW_LEFT
                transparent: true
                enabled: prevIndex && prevIndex.valid

                navigation.panel: root.navigationPanel
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
                    root.poolPaletteRootIndex = prevIndex
                }
            }

            StyledTextLabel {
                text: root.libraryPaletteName

                Layout.alignment: Qt.AlignHCenter
            }

            FlatButton {
                icon: IconCode.ARROW_RIGHT
                transparent: true

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

                navigation.panel: root.navigationPanel
                navigation.name: "nextButton"
                navigation.column: 1
                navigation.row: 3

                onClicked: {
                    root.poolPaletteRootIndex = nextIndex
                }
            }
        }

        Rectangle {
            id: paletteContainer
            width: parent.width
            height: childrenRect.height

            color: ui.theme.backgroundPrimaryColor
            border.color: ui.theme.strokeColor
            border.width: 1

            readonly property int availableHeight: root.maxHeight
                                                   - addToPaletteButton.height
                                                   - (masterIndexControls ? masterIndexControls.height : 0)
                                                   - bottomText.height
                                                   - (elementEditorButton.visible ? elementEditorButton.height : 0)
                                                   - 40

            Column {
                padding: 1
                width: parent.width
                property real contentWidth: width - 2 * padding
                spacing: 0

                ItemSelectionModel {
                    id: masterPaletteSelectionModel
                    model: masterPalette.paletteModel
                }

                PaletteGridView {
                    id: masterPalette
                    height: Math.max(
                                cellSize.height,
                                Math.min(
                                    implicitHeight,
                                    paletteContainer.availableHeight - (customPalette.visible ? (customPalette.height + customPaletteLabel.height) : 0)
                                    )
                                )
                    width: parent.contentWidth

                    ScrollBar.vertical: StyledScrollBar {}

                    // TODO: change settings to "hidden" model?
                    cellSize: root.cellSize
                    drawGrid: root.drawGrid

                    navigationPanel: root.navigationPanel
                    navigationCol: 1
                    navigationRow: 4

                    paletteModel: root.poolPalette
                    paletteRootIndex: root.poolPaletteRootIndex
                    paletteController: root.poolPaletteController
                    selectionModel: masterPaletteSelectionModel

                    enableAnimations: root.enablePaletteAnimations
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
                        transparent: true

                        toolTipTitle: text

                        navigation.panel: root.navigationPanel
                        navigation.name: "deleteButton"
                        navigation.column: 1
                        navigation.row: 100 // Should be more than palette cells

                        onClicked: {
                            Utils.removeSelectedItems(root.customPaletteController, customPaletteSelectionModel, root.customPaletteRootIndex)
                        }
                    }
                }

                ItemSelectionModel {
                    id: customPaletteSelectionModel
                    model: customPalette.paletteModel
                }

                PaletteGridView {
                    id: customPalette
                    visible: !empty
                    width: parent.contentWidth

                    cellSize: root.cellSize
                    drawGrid: root.drawGrid

                    navigationPanel: root.navigationPanel
                    navigationCol: 1
                    navigationRow: 4

                    paletteModel: root.customPalette
                    paletteRootIndex: root.customPaletteRootIndex
                    paletteController: root.customPaletteController
                    selectionModel: customPaletteSelectionModel

                    enableAnimations: root.enablePaletteAnimations
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
            visible: root.elementEditor && root.elementEditor.valid
            enabled: root.paletteEditingEnabled
            width: parent.width
            text: root.elementEditor?.actionName ?? ""
            navigation.panel: root.navigationPanel
            navigation.name: "elementEditorButton"
            navigation.column: 1
            navigation.row: 101 // after deleteButton

            onClicked: {
                Qt.callLater(root.elementEditor.open)
                root.close()
            }
        }
    }
}
