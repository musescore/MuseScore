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
import MuseScore.Utils 3.3

import "utils.js" as Utils

GridView {
    id: paletteView
    clip: true

    interactive: height < contentHeight // TODO: check if it helps on Mac

    keyNavigationEnabled: true
    activeFocusOnTab: true

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
    readonly property bool oneRow: cellDefaultWidth * ncells + moreButton.implicitWidth < width

    implicitHeight: {
        if (!ncells)
            return cellHeight;
        if (!showMoreButton || oneRow)
            return contentHeight;

        const moreButtonCells = Math.ceil(moreButton.implicitWidth / cellWidth);
        const rowCells = Math.floor(width / cellWidth);
        const lastRowCells = ncells % rowCells;
        const freeCells = lastRowCells ? rowCells - lastRowCells : 0;

        if (freeCells >= moreButtonCells)
            return contentHeight;
        return contentHeight + cellHeight;
    }

    readonly property int cellDefaultWidth: cellSize.width
    property bool stretchWidth: !(oneRow && showMoreButton)
    cellWidth: stretchWidth ? Math.floor(Utils.stretched(cellDefaultWidth, width)) : cellDefaultWidth
    cellHeight: cellSize.height

    signal moreButtonClicked()

    MouseArea {
        // Dummy MouseArea to prevent propagation of clicks on empty place to palette's parent
        z: -1000
        anchors.fill: parent
    }

    StyledButton {
        id: moreButton
        visible: showMoreButton

        background: Rectangle {
            color: moreButton.down ? globalStyle.button : mscore.paletteBackground
        }

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: {
            if (paletteView.empty)
                return implicitWidth;

            // align to the left border of some palette cell
            const addition = (parent.width - implicitWidth) % cellWidth - 1; // -1 allows to fit into a cell if palette grid is visible
            if (addition < 0)
                addition += cellWidth;

            return implicitWidth + addition;
        }
        height: cellHeight - (paletteView.oneRow ? 0 : 1)

        text: qsTr("More")
        textColor: down ? globalStyle.buttonText : "black"// palette background has white or light color
        visualFocusTextColor: "darkblue"

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
        cellWidth: parent.cellWidth
        cellHeight: parent.cellHeight

        DropArea {
            id: paletteDropArea
            anchors { fill: parent/*; margins: 10*/ }

//             keys: [ "application/musescore/symbol", "application/musescore/palette/cell" ]

            property var action
            property var proposedAction: Qt.IgnoreAction
            property bool internal: false

            function onDrag(drag) {
                if (drag.proposedAction != proposedAction) {
                    onEntered(drag);
                    return;
                }

                drag.accept(action); // confirm we accept the action we determined inside onEntered

                var idx = paletteView.indexAt(drag.x, drag.y);
                if (idx == -1)
                    idx = paletteView.paletteModel.rowCount(paletteView.paletteRootIndex) - (internal ? 1 : 0);

                if (placeholder.active && placeholder.index == idx)
                    return;
                placeholder.makePlaceholder(idx, { decoration: "#eeeeee", toolTip: "placeholder", accessibleText: "", cellActive: false, mimeData: {} });
            }

            onEntered: {
                onDragOverPaletteFinished();

                // first check if controller allows dropping this item here
                const mimeData = Utils.dropEventMimeData(drag);
                internal = (drag.source.parentModelIndex == paletteView.paletteRootIndex);
                action = paletteView.paletteController.dropAction(mimeData, drag.proposedAction, paletteView.paletteRootIndex, internal);
                proposedAction = drag.proposedAction;

                if (action != Qt.MoveAction)
                    internal = false;

                const accept = (action & drag.supportedActions) && (internal || !externalDropBlocked);

                if (accept)
                    drag.accept(action);
                else
                    drag.accepted = false;

                // If event is accepted, process the drag in a usual way
                if (drag.accepted) {
                    drag.source.internalDrag = internal;
                    drag.source.dragCopy = action == Qt.CopyAction;
                    paletteView.state = "drag";
                    onDrag(drag);
                    }
            }

            onPositionChanged: onDrag(drag)

            function onDragOverPaletteFinished() {
                if (placeholder.active) {
                    placeholder.removePlaceholder();
                    paletteView.state = "default";
                }
                if (drag.source && drag.source.parentModelIndex == paletteView.paletteRootIndex)
                    drag.source.internalDrag = false;
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

                if (typeof data.srcParentModelIndex !== "undefined")
                    drag.source.dropData = data;
                else
                    data.paletteView.insertCell(data.destIndex, data.mimeData, data.action);

                drop.accept(action);
            }
        }
    }

    Text {
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left; leftMargin: 8
            right: moreButton.left
        }
        visible: parent.empty
        font: globalStyle.font
        text: paletteController && paletteController.canDropElements
            ? qsTr("Drag and drop any element here\n(Use %1+Shift to add custom element from the score)").arg(Qt.platform.os === "osx" ? "Cmd" : "Ctrl")
            : qsTr("No elements")
        verticalAlignment: Text.AlignVCenter
        color: "grey"
        wrapMode: Text.WordWrap
        elide: Text.ElideRight
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
            property bool dragCopy: false

            property bool selected: (paletteView.selectionModel && paletteView.selectionModel.hasSelection) ? paletteView.isSelected(modelIndex) : false // hasSelection is to trigger property bindings if selection changes, see https://doc.qt.io/qt-5/qml-qtqml-models-itemselectionmodel.html#hasSelection-prop

            highlighted: activeFocus

            width: paletteView.cellWidth
            height: paletteView.cellHeight

            activeFocusOnTab: true

            contentItem: QmlIconView {
                id: icon
                visible: !parent.dragged || parent.dragCopy
                anchors.fill: parent
                icon: model.decoration
                selected: paletteCell.selected
                active: !!model.cellActive
            }

            readonly property var toolTip: model.toolTip

            onHoveredChanged: {
                if (hovered) {
                    mscore.tooltip.item = paletteCell;
                    mscore.tooltip.text = paletteCell.toolTip ? paletteCell.toolTip : "";
                } else if (mscore.tooltip.item == paletteCell)
                    mscore.tooltip.item = null;
            }

            text: model.accessibleText
            // TODO: these may be needed for support of other screenreaders
            //Accessible.name: model.accessibleText
            //Accessible.description: model.accessibleText

            onClicked: {
                forceActiveFocus();

                paletteView.currentIndex = index;

                const selection = paletteView.selectionModel;

                if (selection) {
                    const modifiers = mscore.keyboardModifiers();
                    const rootIndex = paletteView.paletteRootIndex;

                    if (selection.currentIndex.parent != rootIndex)
                        selection.clearSelection();

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

            onDoubleClicked: {
                const index = paletteCell.modelIndex;
                paletteView.selectionModel.setCurrentIndex(index, ItemSelectionModel.Current);
                paletteView.paletteController.applyPaletteElement(index, mscore.keyboardModifiers());
            }

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
                    const index = paletteCell.modelIndex;
                    paletteView.selectionModel.setCurrentIndex(index, ItemSelectionModel.Current);
                    paletteView.paletteController.applyPaletteElement(index, mscore.keyboardModifiers());
                    event.accepted = true;
                }
            }

            MouseArea {
                id: rightClickArea
                anchors.fill: parent
                acceptedButtons: Qt.RightButton

                onClicked: {
                    contextMenu.modelIndex = paletteCell.modelIndex;
                    contextMenu.canEdit = paletteView.paletteController.canEdit(paletteView.paletteRootIndex);
                    contextMenu.popup();
                }
            }

            Drag.active: paletteCellDragArea.drag.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.CopyAction | (model.editable ? Qt.MoveAction : 0)
            Drag.mimeData: Drag.active ? mimeData : {}

            onInternalDragChanged: DelegateModel.inItems = !internalDrag;

            property var dropData: null

            Drag.onDragStarted: {
                paletteView.state = "drag";
                DelegateModel.inPersistedItems = true;
            }

            Drag.onDragFinished: {
                paletteView.state = "default";
                internalDrag = false;
                DelegateModel.inPersistedItems = false;

                if (dropData) {
                    var data = dropData;
                    if (data.action == Qt.MoveAction && data.srcParentModelIndex == data.paletteView.paletteRootIndex)
                        data.paletteView.moveCell(data.srcRowIndex, data.destIndex);
                    else
                        data.paletteView.insertCell(data.destIndex, data.mimeData, data.action);

                    dropData = null;
                }
            }
//                             Drag.hotSpot: Qt.point(64, 0) // TODO
        }
    }

    Menu {
        id: contextMenu
        property var modelIndex: null
        property bool canEdit: true

        MenuItem {
            enabled: contextMenu.canEdit
            text: qsTr("Delete")
            onTriggered: paletteView.paletteController.remove(contextMenu.modelIndex)
        }
        MenuItem {
            enabled: contextMenu.canEdit
            text: qsTr("Properties…")
            onTriggered: paletteView.paletteController.editCellProperties(contextMenu.modelIndex)
        }
    }
}
