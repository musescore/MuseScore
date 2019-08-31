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
import MuseScore.Views 3.3

import "utils.js" as Utils

GridView {
    id: paletteView
    clip: true

    interactive: height < contentHeight // TODO: check if it helps on Mac

    keyNavigationEnabled: true
    activeFocusOnTab: true

    highlightFollowsCurrentItem: true
//     preferredHighlightBegin: height / 2 - 100 // 100 being item height / 2
//     preferredHighlightEnd: height / 2 + 100 // 100 being item height / 2
//     highlightRangeMode: GridView.StrictlyEnforceRange

    property size cellSize
    property bool drawGrid: false
    property bool showMoreButton: false

    property var paletteModel
    property var paletteRootIndex
    property PaletteController paletteController
    property var selectionModel: null

    property bool empty: !paletteCellDelegateModel.count

    property int maxWidth: parent.width

    property bool externalMoveBlocked: false;

    // internal property: whether the palette fits to one row
    property bool oneRow: cellWidth * paletteCellDelegateModel.count + moreButton.implicitWidth < maxWidth

    implicitWidth: {
        if (showMoreButton && oneRow)
            return maxWidth;
        return maxWidth - (maxWidth % cellWidth); // TODO or stretch cells instead?
    }
    implicitHeight: {
        const ncells = paletteCellDelegateModel.count
        if (!ncells)
            return cellHeight;
        if (!showMoreButton || oneRow)
            return contentHeight;

        const moreButtonCells = Math.ceil(moreButton.implicitWidth / cellWidth);
        const rowCells = Math.floor(maxWidth / cellWidth);
        const lastRowCells = ncells % rowCells;
        const freeCells = lastRowCells ? rowCells - lastRowCells : 0;

        if (freeCells >= moreButtonCells)
            return contentHeight;
        return contentHeight + cellHeight;
    }

//     cellWidth: stretched(cellSize.width, width) // definition in utils.js
    cellWidth: cellSize.width
    cellHeight: cellSize.height

    signal moreButtonClicked()

    Button {
        id: moreButton
        visible: showMoreButton

        background: Rectangle {
            color: moreButton.down ? "mediumslateblue" : "white"
        }

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: implicitWidth + (parent.width - implicitWidth) % cellWidth - 1 // -1 allows to fit into a cell if palette grid is visible
        height: cellHeight - (paletteView.oneRow ? 0 : 1)

        text: qsTr("More")

        onClicked: paletteView.moreButtonClicked()
    }

    PlaceholderManager {
        id: placeholder
        delegateModel: paletteCellDelegateModel
    }

    PaletteBackground {
        z: -1
        anchors.fill: parent
        drawGrid: parent.drawGrid && !parent.empty
        offsetX: parent.contentX
        offsetY: parent.contentY

        DropArea {
            anchors { fill: parent/*; margins: 10*/ }

//             keys: [ "application/musescore/symbol", "application/musescore/palette/cell" ]

            property var action
            property bool internal: false

            function onDrag(drag) {
                drag.accept(action); // confirm we accept the action we determined inside onEntered

                var idx = paletteView.indexAt(drag.x, drag.y);
                if (idx == -1)
                    idx = paletteView.paletteModel.rowCount(paletteView.paletteRootIndex) - (internal ? 1 : 0);

                if (placeholder.active && placeholder.index == idx)
                    return;
                placeholder.makePlaceholder(idx, { decoration: "#eeeeee", toolTip: "placeholder", mimeData: {} });
            }

            onEntered: {
                // first check if controller allows dropping this item here
                const mimeData = Utils.dropEventMimeData(drag);
                internal = (drag.source.parentModelIndex == paletteView.paletteRootIndex);
                action = paletteView.paletteController.dropAction(mimeData, drag.supportedActions, internal);

                const externalMove = (action == Qt.MoveAction) && !internal;
                const accept = (action & drag.supportedActions) && !(externalMove && paletteView.externalMoveBlocked);

                if (accept)
                    drag.accept(action);
                else
                    drag.accepted = false;

                // If event is accepted, process the drag in a usual way
                if (drag.accepted) {
                    drag.source.internalDrag = internal;
                    onDrag(drag);
                    }
            }

            onPositionChanged: onDrag(drag)

            function onDragOverPaletteFinished() {
                placeholder.removePlaceholder();
                if (drag.source.parentModelIndex == paletteView.paletteRootIndex)
                    drag.source.internalDrag = false;
            }

            onExited: onDragOverPaletteFinished();

            onDropped: {
                if (!action)
                    return;

                const destIndex = placeholder.active ? placeholder.index : paletteView.paletteModel.rowCount(paletteView.paletteRootIndex);
                onDragOverPaletteFinished();

                var success = false;
                if (drag.source.parentModelIndex == paletteView.paletteRootIndex)
                    success = paletteView.moveCell(drag.source.rowIndex, destIndex);
                else
                    success = paletteView.insertCell(destIndex, Utils.dropEventMimeData(drop))

                if (success)
                    drop.accept(action);
                else
                    drop.accepted = false;
            }
        }
    }

    Text {
        visible: parent.empty
        text: paletteController && paletteController.canDropElements
            ? qsTr("Drag and drop any element here")
            : qsTr("No elements")
    }

    function isSelected(modelIndex) {
        if (!selectionModel)
            return false;
        return selectionModel.isSelected(modelIndex);
    }

    function moveCell(srcRow, destRow) {
        return paletteController.move(
            paletteRootIndex, srcRow,
            paletteRootIndex, destRow
        );
    }

    function insertCell(row, mimeData) {
        return paletteController.insert(paletteRootIndex, row, mimeData);
    }

//     function appendCell(mimeData) {
//         const row = paletteView.paletteModel.rowCount(paletteView.paletteRootIndex);
//         insertCell(row, mimeData);
//     }

    function drop(row, mimeData, supportedActions) {
        // TODO
    }

    function removeCell(row) {
        return paletteController.remove(paletteRootIndex, row);
    }

    function removeSelectedCells() {
        Utils.removeSelectedItems(paletteModel, paletteController, paletteSelectionModel, paletteRootIndex);
    }

    Keys.onDeletePressed: {
        removeSelectedCells();
    }
    Keys.onPressed: {
        if (event.key == Qt.Key_Backspace) {
            removeSelectedCells();
            event.accepted = true;
        }
    }

    model: DelegateModel {
        id: paletteCellDelegateModel
//         model: paletteView.visible ? paletteView.paletteModel : null // TODO: use this optimization? TODO: apply it manually where appropriate (Custom palette breaks)
        model: paletteView.paletteModel
        rootIndex: paletteView.paletteRootIndex

        delegate: ItemDelegate {
            id: paletteCell
            property int rowIndex: index
            property var modelIndex: paletteView.model.modelIndex(index)
            property var parentModelIndex: paletteView.paletteRootIndex

            property bool dragged: Drag.active
            property bool internalDrag: false

            property bool selected: (paletteView.selectionModel && paletteView.selectionModel.hasSelection) ? paletteView.isSelected(modelIndex) : false // hasSelection is to trigger property bindings if selection changes, see https://doc.qt.io/qt-5/qml-qtqml-models-itemselectionmodel.html#hasSelection-prop

            highlighted: activeFocus

            width: paletteView.cellWidth
            height: paletteView.cellHeight

            activeFocusOnTab: true

            contentItem: QmlIconView {
                id: icon
                visible: !parent.dragged
                anchors.fill: parent
                icon: model.decoration
                selected: paletteCell.selected
            }

            ToolTip.visible: hovered
            ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
//                             ToolTip.timeout: Qt.styleHints.mousePressAndHoldInterval
            ToolTip.text: model.toolTip

            text: model.toolTip

            onClicked: {
                forceActiveFocus();

                paletteView.currentIndex = index;

                const selection = paletteView.selectionModel;

                if (selection) {
                    const modifiers = mscore.keyboardModifiers(); // TODO: check on moving from a plugin-based widget
                    const rootIndex = paletteView.paletteRootIndex;

                    if (selection.currentIndex.parent != rootIndex)
                        selection.clear();

                    if (modifiers & Qt.ShiftModifier && selection.currentIndex.parent == rootIndex) {
                        const model = paletteView.paletteModel;
                        const firstRow = selection.currentIndex.row;
                        const lastRow = paletteCell.rowIndex;
                        const step = firstRow < lastRow ? 1 : -1;
                        const endRow = lastRow + step;
                        for (var row = firstRow; row != endRow; row += step) {
                            const idx = model.index(row, 0, rootIndex);
                            selection.select(idx, ItemSelectionModel.Select);
                        }
                        // update current index
                        selection.setCurrentIndex(idx, ItemSelectionModel.Current);

                    } else {
                        var cmd = selected ? ItemSelectionModel.Toggle : ItemSelectionModel.ClearAndSelect;
                        if (modifiers & Qt.ControlModifier)
                            cmd = ItemSelectionModel.Toggle;
                        selection.setCurrentIndex(modelIndex, cmd);
                    }
                }
            }

            onDoubleClicked: paletteView.paletteController.applyPaletteElement(paletteCell.modelIndex, mscore.keyboardModifiers());

            MouseArea {
                id: paletteCellDragArea
                anchors.fill: parent
                drag.target: this

                onPressed: icon.grabToImage(function(result) {
                    parent.Drag.imageSource = result.url
                })

                onClicked: parent.onClicked(mouse)
                onDoubleClicked: parent.onDoubleClicked(mouse)
            }

            Keys.onPressed: {
                if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return) {
                    paletteView.paletteController.applyPaletteElement(paletteCell.modelIndex, mscore.keyboardModifiers());
                    event.accepted = true;
                }
            }

            MouseArea {
                id: rightClickArea
                anchors.fill: parent
                acceptedButtons: Qt.RightButton

                onClicked: {
                    contextMenu.modelIndex = paletteCell.modelIndex;
                    if (contextMenu.popup) // Menu.popup() is available since Qt 5.10 only
                        contextMenu.popup();
                    else {
                        contextMenu.x = mouseX;
                        contextMenu.y = mouseY;
                        contextMenu.open();
                    }
                }
            }


            Drag.active: paletteCellDragArea.drag.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.CopyAction | Qt.MoveAction // TODO: it would be good to switch to Qt.MoveAction
                                                 // on drags within a palette or between master and
                                                 // visible palettes but this doesn't seem possible
            Drag.proposedAction: Qt.CopyAction
            Drag.mimeData: Drag.active ? mimeData : {}

            onInternalDragChanged: DelegateModel.inItems = !internalDrag;

            Drag.onDragStarted: DelegateModel.inPersistedItems = true;

            Drag.onDragFinished: {
                internalDrag = false;
                DelegateModel.inPersistedItems = false;
            }
//                             Drag.hotSpot: Qt.point(64, 0) // TODO
        }
    }

    Menu {
        id: contextMenu
        property var modelIndex: null

        MenuItem {
            text: qsTr("Properties…")
            onTriggered: paletteView.paletteController.editCellProperties(contextMenu.modelIndex)
        }
    }
}
