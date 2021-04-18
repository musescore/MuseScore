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
import QtQuick.Controls 2.15
import QtQml.Models 2.2

import MuseScore.Palette 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

import "utils.js" as Utils

GridView {
    id: paletteView
    clip: true

    interactive: height < contentHeight // TODO: check if it helps on Mac
    boundsBehavior: Flickable.StopAtBounds

    property size cellSize
    property bool drawGrid: false
    property bool showMoreButton: false

    property var paletteModel
    property var paletteRootIndex
    property PaletteController paletteController
    property var selectionModel: null

    property int ncells: paletteCellDelegateModel.count
    property bool empty: !ncells

    property bool externalDropBlocked: false

    property bool enableAnimations: true

    property KeyNavigationSubSection keynavSubSection: null
    property int keynavRow: 0
    property int keynavCol: 1

    states: [
        State {
            name: "default"
        },
        State {
            name: "drag"
            PropertyChanges {
                target: paletteView
                // unbind width stretching from the delegate model's state while dragging
                explicit: true
                stretchWidth: paletteCellDelegateModel.stretchWidth
                restoreEntryValues: true
            }
        }
    ]

    state: "default"

    // internal property: whether the palette fits to one row
    readonly property bool oneRow: cellDefaultWidth * ncells + moreButtonRect.implicitWidth < width

    implicitHeight: {
        if (!ncells) {
            return cellHeight;
        }

        if (!showMoreButton || oneRow) {
            return contentHeight;
        }

        const moreButtonCells = Math.ceil(moreButtonRect.implicitWidth / cellWidth);
        const rowCells = Math.floor(width / cellWidth);
        const lastRowCells = ncells % rowCells;
        const freeCells = lastRowCells ? rowCells - lastRowCells : 0;

        if (freeCells >= moreButtonCells) {
            return contentHeight;
        }

        return contentHeight + cellHeight;
    }

    readonly property int cellDefaultWidth: cellSize.width
    property bool stretchWidth: !(oneRow && showMoreButton)
    cellWidth: stretchWidth ? Math.floor(Utils.stretched(cellDefaultWidth, width)) : cellDefaultWidth
    cellHeight: cellSize.height

    readonly property int nrows: Math.max(0, Math.floor(height / cellHeight))
    readonly property int ncolumns: Math.max(0, Math.floor(width / cellWidth))
    readonly property real lastColumnCellWidth : cellWidth + (width % cellWidth) // width of last cell in a row: might be stretched to avoid a gap at row end

    signal moreButtonClicked()

    MouseArea {
        // Dummy MouseArea to prevent propagation of clicks on empty place to palette's parent
        z: -1000
        anchors.fill: parent
    }

    Rectangle {
        id: background
        anchors.fill: parent
        z: -1
        color: ui.theme.textFieldColor
    }

    StyledTextLabel {
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left; leftMargin: 8
            right: moreButtonRect.left
        }

        visible: parent.empty
        opacity: 0.5

        text: paletteController && paletteController.canDropElements
              ? qsTrc("palette", "Drag and drop any element here")
              : qsTrc("palette", "No elements")

        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignLeft
    }

    Rectangle {
        id: moreButtonRect

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        implicitWidth: 64
        width: {
            if (paletteView.empty) {
                return implicitWidth;
            }

            // align to the left border of some palette cell
            var addition = (parent.width - implicitWidth) % cellWidth - 1; // -1 allows to fit into a cell if palette grid is visible
            if (addition < 0) {
                addition += cellWidth;
            }

            return implicitWidth + addition;
        }

        height: cellHeight - (paletteView.oneRow ? 0 : 1)
        visible: showMoreButton
        color: background.color

        z: grid.z + 1

        FlatButton {
            id: moreButton

            anchors.fill: parent

            keynav.subsection: paletteView.keynavSubSection
            //! NOTE Just Up/Down navigation now
            keynav.row: paletteView.ncells + paletteView.keynavRow
            keynav.column: 1
            keynav.enabled: paletteView.visible

            onActiveFocusChanged: {
                if (activeFocus) {
                    paletteTree.currentTreeItem = this;

                    if (ui.keyboardModifiers() === Qt.NoModifier) {
                        paletteView.selectionModel.clearSelection();
                    }
                }
            }

            text: qsTrc("palette", "More")

            normalStateColor: "transparent"
            hoveredStateColor: ui.theme.accentColor
            pressedStateColor: ui.theme.accentColor

            onClicked: paletteView.moreButtonClicked()
        }
    }

    PlaceholderManager {
        id: placeholder
        delegateModel: paletteCellDelegateModel
    }

    Timer {
        id: dragDropReorderTimer
        interval: 400
    }

    PaletteGrid {
        id: grid
        z: 1
        anchors.fill: parent
        drawGrid: parent.drawGrid && !parent.empty
        offsetX: parent.contentX
        offsetY: parent.contentY
        cellWidth: parent.cellWidth
        cellHeight: parent.cellHeight

        DropArea {
            id: paletteDropArea
            anchors.fill: parent

            property var action
            property var proposedAction: Qt.IgnoreAction
            property bool internal: false

            function onDrag(drag) {
                if (drag.proposedAction !== proposedAction) {
                    onEntered(drag);
                    return;
                }

                if (drag.source.dragged) {
                    drag.source.internalDrag = internal;
                    drag.source.dragCopy = action === Qt.CopyAction;
                    paletteView.state = "drag";
                    drag.source.paletteDrag = true;
                } else if (typeof drag.source.paletteDrag !== "undefined") { // if this is a palette and not, e.g., scoreview
                    return;
                }

                drag.accept(action); // confirm we accept the action we determined inside onEntered

                var idx = paletteView.indexAt(drag.x, drag.y);
                if (idx === -1) {
                    idx = paletteView.paletteModel.rowCount(paletteView.paletteRootIndex) - (internal ? 1 : 0);
                }

                if (placeholder.active && placeholder.index === idx) {
                    return;
                }

                placeholder.makePlaceholder(idx, { decoration: ui.theme.textFieldColor, toolTip: "placeholder", accessibleText: "", cellActive: false, mimeData: {} });
            }

            onEntered: {
                onDragOverPaletteFinished();

                // first check if controller allows dropping this item here
                const mimeData = Utils.dropEventMimeData(drag);
                internal = (drag.source.parentModelIndex === paletteView.paletteRootIndex);
                action = paletteView.paletteController.dropAction(mimeData, drag.proposedAction, paletteView.paletteRootIndex, internal);
                proposedAction = drag.proposedAction;

                if (action !== Qt.MoveAction) {
                    internal = false;
                }

                const accept = (action & drag.supportedActions) && (internal || !externalDropBlocked);

                if (accept) {
                    drag.accept(action);
                } else {
                    drag.accepted = false;
                }

                // If event is accepted, process the drag in a usual way
                if (drag.accepted) {
                    onDrag(drag);
                }
            }

            onPositionChanged: onDrag(drag)

            function onDragOverPaletteFinished() {
                if (placeholder.active) {
                    placeholder.removePlaceholder();
                    paletteView.state = "default";
                }

                if (drag.source && drag.source.parentModelIndex === paletteView.paletteRootIndex) {
                    drag.source.internalDrag = false;
                }
            }

            onExited: onDragOverPaletteFinished();

            onDropped: {
                if (!action) {
                    onDragOverPaletteFinished();
                    return;
                }

                const destIndex = placeholder.active ? placeholder.index : paletteView.paletteModel.rowCount(paletteView.paletteRootIndex);
                onDragOverPaletteFinished();

                // Moving cells here causes Drag.onDragFinished be not called properly.
                // Therefore record the necessary information to move cells later.
                const data = {
                    action: action,
                    srcParentModelIndex: drag.source.parentModelIndex,
                    srcRowIndex: drag.source.rowIndex,
                    paletteView: paletteView,
                    destIndex: destIndex,
                    mimeData: Utils.dropEventMimeData(drop)
                };

                if (typeof data.srcParentModelIndex !== "undefined") {
                    drag.source.dropData = data;
                } else {
                    data.paletteView.insertCell(data.destIndex, data.mimeData, data.action);
                }

                drop.accept(action);
            }
        }
    }

    add: Transition {
        id: addTransition
        enabled: paletteView.enableAnimations
        readonly property bool unresolvedItem: ViewTransition.item && ViewTransition.item.DelegateModel.isUnresolved;
        ParallelAnimation {
            NumberAnimation { property: "scale"; from: 0; to: 1; duration: addTransition.unresolvedItem ? 0 : 150; easing.type: Easing.InOutQuad }
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: addTransition.unresolvedItem ? 0 : 250 }
        }
    }

    remove: Transition {
        id: removeTransition
        enabled: paletteView.enableAnimations && !paletteDropArea.containsDrag
        readonly property bool unresolvedItem: ViewTransition.item && ViewTransition.item.DelegateModel.isUnresolved;
        ParallelAnimation {
            NumberAnimation { property: "scale"; to: 0; duration: removeTransition.unresolvedItem ? 0 : 150; easing.type: Easing.InOutQuad }
            NumberAnimation { property: "opacity"; to: 0; duration: removeTransition.unresolvedItem ? 0 : 250 }
        }
    }

    displaced: Transition {
        enabled: paletteView.enableAnimations
        NumberAnimation { properties: "x,y"; duration: 150 }
    }

    function isSelected(modelIndex) {
        if (!selectionModel) {
            return false;
        }

        return selectionModel.isSelected(modelIndex);
    }

    function moveCell(srcRow, destRow) {
        return paletteController.move(
                    paletteRootIndex, srcRow,
                    paletteRootIndex, destRow
                    );
    }

    function insertCell(row, mimeData, action) {
        return paletteController.insert(paletteRootIndex, row, mimeData, action);
    }

    function drop(row, mimeData, supportedActions) {
        // TODO
    }

    function removeCell(row) {
        return paletteController.remove(model.modelIndex(row));
    }

    function removeSelectedCells() {
        Utils.removeSelectedItems(paletteController, selectionModel, paletteRootIndex);
    }

    function focusFirstItem() {
        if (count == 0 && moreButton.visible) {
            moreButton.forceActiveFocus();
        } else {
            currentIndex = 0;
            currentItem.forceActiveFocus();
        }
    }

    function focusLastItem() {
        if (moreButton.visible) {
            moreButton.forceActiveFocus();
        } else {
            currentIndex = count - 1;
            currentItem.forceActiveFocus();
        }
    }

    function focusNextMatchingItem(str, startIndex) {
        const modelIndex = paletteModel.index(startIndex, 0, paletteRootIndex);
        const matchedIndexList = paletteModel.match(modelIndex, Qt.ToolTipRole, str);
        if (matchedIndexList.length) {
            currentIndex = matchedIndexList[0].row;
            currentItem.forceActiveFocus();
            return true;
        }
        return false;
    }

    function typeAheadFind(chr) {
        if (paletteTree.typeAheadStr.length) {
            // continue search on current item
            const sameChr = chr === paletteTree.typeAheadStr;
            paletteTree.typeAheadStr += chr;
            const found = focusNextMatchingItem(paletteTree.typeAheadStr, currentIndex);
            if (found || !sameChr) {
                return;
            }
        }
        // start new search on next item
        paletteTree.typeAheadStr = chr;
        const nextIndex = (currentIndex === count - 1) ? 0 : currentIndex + 1;
        focusNextMatchingItem(chr, nextIndex);
    }

    function updateSelection(itemPressed) {
        if (itemPressed === undefined) {
            itemPressed = false; // reason function was called
        }

        const modifiers = ui.keyboardModifiers();
        const shiftHeld = modifiers & Qt.ShiftModifier;
        const ctrlHeld = modifiers & Qt.ControlModifier;
        const herePreviously = selectionModel.currentIndex.parent === paletteRootIndex;

        if (!ctrlHeld || !herePreviously) {
            selectionModel.clearSelection();
        }

        if (shiftHeld && herePreviously) {
            selectionModel.selectRange(currentItem.modelIndex);
        } else if (ctrlHeld) {
            selectionModel.setCurrentIndex(currentItem.modelIndex, itemPressed ? ItemSelectionModel.Toggle : ItemSelectionModel.NoUpdate);
        } else {
            selectionModel.setCurrentIndex(currentItem.modelIndex, ItemSelectionModel.Select);
        }
    }

    Rectangle {
        id: draggedIcon

        width: paletteView.cellWidth
        height: paletteView.cellHeight

        color: ui.theme.textFieldColor
        visible: false

        property alias source: view.icon

        IconView {
            id: view

            anchors.fill: parent
        }
    }

    model: DelegateModel {
        id: paletteCellDelegateModel
        //         model: paletteView.visible ? paletteView.paletteModel : null // TODO: use this optimization? TODO: apply it manually where appropriate (Custom palette breaks)
        model: paletteView.paletteModel
        rootIndex: paletteView.paletteRootIndex

        delegate: ListItemBlank {
            id: paletteCell
            property int rowIndex: index
            property var modelIndex: paletteView.model.modelIndex(index)
            property var parentModelIndex: paletteView.paletteRootIndex

            //! NOTE Please, don't remove (igor.korsukov@gmail.com)
            //property int cellRow: paletteView.ncolumns == 0 ? 0 : Math.floor(model.index / paletteView.ncolumns)
            //property int cellCol: model.index - (cellRow * paletteView.ncolumns)

            onActiveFocusChanged: {
                if (activeFocus) {
                    paletteTree.currentTreeItem = this;
                    paletteView.updateSelection(false);
                }
            }

            readonly property bool dragged: Drag.active && !dragDropReorderTimer.running
            property bool paletteDrag: false
            property bool internalDrag: false
            property bool dragCopy: false

            isSelected: (paletteView.selectionModel && paletteView.selectionModel.hasSelection) ? paletteView.isSelected(modelIndex) : false // hasSelection is to trigger property bindings if selection changes, see https://doc.qt.io/qt-5/qml-qtqml-models-itemselectionmodel.html#hasSelection-prop

            width: paletteView.cellWidth
            height: paletteView.cellHeight

            keynav.subsection: paletteView.keynavSubSection

            //! NOTE Please, don't remove (igor.korsukov@gmail.com)
            //keynav.row: paletteCell.cellRow + paletteView.keynavRow
            //keynav.column: paletteCell.cellCol + paletteView.keynavCol

            //! NOTE Just Up/Down navigation now
            keynav.row: model.index + paletteView.keynavRow
            keynav.column: 1
            keynav.enabled: paletteView.visible
            keynav.onTriggered: paletteCell.doClicked()

            IconView {
                anchors.fill: parent
                icon: model.decoration
                visible: !parent.paletteDrag || parent.dragCopy
            }

            hint: model.toolTip

            Accessible.name: model.accessibleText;

            // leftClickArea
            mouseArea.drag.target: this
            mouseArea.onPressed: {
                paletteView.currentIndex = paletteCell.rowIndex;
                paletteCell.forceActiveFocus();
                paletteView.updateSelection(true);
                paletteCell.beginDrag();
            }

            function doClicked() {
                if (paletteView.paletteController.applyPaletteElement(paletteCell.modelIndex, ui.keyboardModifiers())) {
                    paletteView.selectionModel.setCurrentIndex(paletteCell.modelIndex, ItemSelectionModel.Current);
                }
            }

            onClicked: paletteCell.doClicked()

            onDoubleClicked: {
                const index = paletteCell.modelIndex;
                paletteView.selectionModel.setCurrentIndex(index, ItemSelectionModel.Current);
                paletteView.paletteController.applyPaletteElement(index, mouseArea.mouse.modifiers);
            }

            MouseArea {
                id: rightClickArea
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: showCellMenu(true)
            }

            Drag.active: mouseArea.drag.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.CopyAction | (model.editable ? Qt.MoveAction : 0)
            Drag.mimeData: Drag.active ? mimeData : {}

            onInternalDragChanged: {
                if (internalDrag && dragDropReorderTimer.running) {
                    return;
                }

                DelegateModel.inItems = !internalDrag;
            }
            onDraggedChanged: DelegateModel.inItems = !internalDrag;

            property var dropData: null

            Drag.onDragStarted: {
                paletteView.state = "drag";
                DelegateModel.inPersistedItems = true;
            }

            Drag.onDragFinished: {
                paletteView.state = "default";
                paletteDrag = false;
                internalDrag = false;
                DelegateModel.inPersistedItems = false;

                if (dropData) {
                    var data = dropData;
                    if (data.action === Qt.MoveAction && data.srcParentModelIndex === data.paletteView.paletteRootIndex) {
                        data.paletteView.moveCell(data.srcRowIndex, data.destIndex);
                    } else {
                        data.paletteView.insertCell(data.destIndex, data.mimeData, data.action);
                    }

                    dropData = null;
                }
            }

            function beginDrag() {
                draggedIcon.source = model.decoration

                draggedIcon.grabToImage(function(result) {
                    Drag.imageSource = result.url
                    dragDropReorderTimer.restart();
                })
            }

            function showCellMenu() {
                contextMenu.modelIndex = modelIndex
                contextMenu.canEdit = paletteView.paletteController.canEdit(paletteView.paletteRootIndex)
                contextMenu.popup()
            }

            Connections {
                // force not hiding palette cell if it is being dragged to a score
                enabled: paletteCell.paletteDrag
                target: paletteRootModel

                function onElementDraggedToScoreView() {
                    paletteCell.paletteDrag = false;
                }
            }
        } // end ListItemBlank
    } // end DelegateModel

    ContextMenu {
        id: contextMenu

        property var modelIndex: null
        property bool canEdit: true

        StyledContextMenuItem {
            hintIcon: IconCode.DELETE_TANK
            text: qsTrc("palette", "Delete")
            enabled: contextMenu.canEdit

            onTriggered: {
                paletteView.paletteController.remove(contextMenu.modelIndex)
            }
        }

        StyledContextMenuItem {
            text: qsTrc("palette", "Properties…")
            enabled: contextMenu.canEdit

            onTriggered: {
                Qt.callLater(paletteView.paletteController.editCellProperties, contextMenu.modelIndex)
            }
        }
    }
}
