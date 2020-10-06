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

    readonly property real ncolumns: Math.floor(width / cellWidth);
    readonly property real lastColumnCellWidth : cellWidth + (width % cellWidth) // width of last cell in a row: might be stretched to avoid a gap at row end

    signal moreButtonClicked()

    MouseArea {
        // Dummy MouseArea to prevent propagation of clicks on empty place to palette's parent
        z: -1000
        anchors.fill: parent
    }

    StyledButton {
        id: moreButton
        visible: showMoreButton
        activeFocusOnTab: this === paletteTree.currentTreeItem

        highlighted: visualFocus || hovered

        background: Rectangle {
            color: mscore.paletteBackground
            Rectangle {
                anchors.fill: parent
                color: globalStyle.voice1Color
                opacity: moreButton.down ? 0.4 : (moreButton.highlighted ? 0.2 : 0.0)
            }
            border.color: moreButton.activeFocus ? "lightblue" : "transparent" // show current item
            border.width: 2
        }

        onActiveFocusChanged: {
            if (activeFocus) {
                paletteTree.currentTreeItem = this;

                if (mscore.keyboardModifiers() === Qt.NoModifier)
                    paletteView.selectionModel.clearSelection();
            }
        }

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: {
            if (paletteView.empty)
                return implicitWidth;

            // align to the left border of some palette cell
            var addition = (parent.width - implicitWidth) % cellWidth - 1; // -1 allows to fit into a cell if palette grid is visible
            if (addition < 0)
                addition += cellWidth;

            return implicitWidth + addition;
        }
        height: cellHeight - (paletteView.oneRow ? 0 : 1)

        text: qsTr("More")
        textColor: down ? globalStyle.buttonText : "black"// palette background has white or light color
        visualFocusTextColor: "darkblue"

        onClicked: paletteView.moreButtonClicked()

        Keys.onShortcutOverride: {
            // Intercept all keys that we want to use with Keys.onPressed
            // in case they are assigned as shortcuts in Preferences.
            event.accepted = true; // intercept everything
            switch (event.key) {
                case Qt.Key_Up:
                case Qt.Key_Down:
                    return;
            }
            event.accepted = false; // allow key to function as shortcut (don't intercept)
        }

        Keys.onPressed: {
            // NOTE: All keys must be intercepted with Keys.onShortcutOverride.
            switch (event.key) {
                case Qt.Key_Up:
                    focusPreviousItem();
                    break;
                case Qt.Key_Down:
                    paletteTree.focusNextItem(false);
                    break;
                default:
                    return; // don't accept event
            }
            event.accepted = true;
        }

        function focusPreviousItem() {
            if (paletteView.count == 0) {
                paletteTree.currentItem.forceActiveFocus();
            } else {
                paletteView.currentIndex = paletteView.count - 1
                paletteView.currentItem.forceActiveFocus();
            }
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

                if (drag.source.dragged) {
                    drag.source.internalDrag = internal;
                    drag.source.dragCopy = action == Qt.CopyAction;
                    paletteView.state = "drag";
                    drag.source.paletteDrag = true;
                } else if (typeof drag.source.paletteDrag !== "undefined") // if this is a palette and not, e.g., scoreview
                    return;

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
                if (drag.accepted)
                    onDrag(drag);
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
        Utils.removeSelectedItems(paletteController, selectionModel, paletteRootIndex);
    }

    function focusNextItem(flags) {
        if (flags === undefined)
            flags = ItemSelectionModel.ClearAndSelect;

        if (currentIndex == count - 1) {
            if (moreButton.visible)
                moreButton.forceActiveFocus();
            else
                paletteTree.focusNextItem(false);
        } else {
            currentIndex++; // next grid item
        }
    }

    function focusPreviousItem(flags) {
        if (flags === undefined)
            flags = ItemSelectionModel.ClearAndSelect;

        if (currentIndex == 0)
            paletteTree.currentItem.forceActiveFocus();
        else
            currentIndex--; // previous grid item
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
            if (found || !sameChr)
                return;
        }
        // start new search on next item
        paletteTree.typeAheadStr = chr;
        const nextIndex = (currentIndex === count - 1) ? 0 : currentIndex + 1;
        focusNextMatchingItem(chr, nextIndex);
    }

    function updateSelection(itemPressed) {
        if (itemPressed === undefined)
            itemPressed = false; // reason function was called

        const modifiers = mscore.keyboardModifiers();
        const shiftHeld = modifiers & Qt.ShiftModifier;
        const ctrlHeld = modifiers & Qt.ControlModifier;
        const herePreviously = selectionModel.currentIndex.parent === paletteRootIndex;

        if (!ctrlHeld || !herePreviously)
            selectionModel.clearSelection();

        if (shiftHeld && herePreviously)
            selectionModel.selectRange(currentItem.modelIndex);
        else if (ctrlHeld)
            selectionModel.setCurrentIndex(currentItem.modelIndex, itemPressed ? ItemSelectionModel.Toggle : ItemSelectionModel.NoUpdate);
        else
            selectionModel.setCurrentIndex(currentItem.modelIndex, ItemSelectionModel.Select);
    }

    Keys.onShortcutOverride: {
        // Intercept all keys that we want to use with Keys.onPressed
        // in case they are assigned as shortcuts in Preferences.
        event.accepted = true; // intercept everything
        switch (event.key) {
            case Qt.Key_Up:
            case Qt.Key_Down:
            case Qt.Key_Left:
            case Qt.Key_Right:
            case Qt.Key_Backspace:
            case Qt.Key_Delete:
                return;
        }
        event.accepted = false; // allow key to function as shortcut (don't intercept)
    }

    Keys.onPressed: {
        // NOTE: All keys must be intercepted with Keys.onShortcutOverride.
        switch (event.key) {
            case Qt.Key_Up:
                focusPreviousItem();
                break;
            case Qt.Key_Down:
                focusNextItem();
                break;
            case Qt.Key_Left:
                paletteTree.currentItem.forceActiveFocus();
                break;
            case Qt.Key_Right:
                if (moreButton.visible)
                    moreButton.forceActiveFocus();
                break;
            case Qt.Key_Backspace:
            case Qt.Key_Delete:
                removeSelectedCells();
                break;
            default:
                return; // don't accept event
        }
        event.accepted = true;
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

            onActiveFocusChanged: {
                if (activeFocus) {
                    paletteTree.currentTreeItem = this;
                    paletteView.updateSelection(false);
                }
            }

            opacity: enabled ? 1.0 : 0.3

            readonly property bool dragged: Drag.active && !dragDropReorderTimer.running
            property bool paletteDrag: false
            property bool internalDrag: false
            property bool dragCopy: false

            property bool selected: (paletteView.selectionModel && paletteView.selectionModel.hasSelection) ? paletteView.isSelected(modelIndex) : false // hasSelection is to trigger property bindings if selection changes, see https://doc.qt.io/qt-5/qml-qtqml-models-itemselectionmodel.html#hasSelection-prop

            highlighted: visualFocus || hovered || !!model.cellActive

            width: paletteView.cellWidth
            height: paletteView.cellHeight

            activeFocusOnTab: this === paletteTree.currentTreeItem

            contentItem: QmlIconView {
                id: icon
                visible: !parent.paletteDrag || parent.dragCopy
                anchors.fill: parent
                icon: model.decoration
                selected: false // TODO: remove properties?
                active: false // TODO: remove properties?
            }

            background: Rectangle {
                color: "transparent"
                border.color: paletteCell.activeFocus ? "lightblue" : "transparent" // show current item
                border.width: 2
                width: ((paletteCell.rowIndex + 1) % paletteView.ncolumns) ? paletteView.cellWidth : paletteView.lastColumnCellWidth

                Rectangle {
                    id: cellBackground
                    anchors.fill: parent
                    color: globalStyle.voice1Color
                    opacity: 0.0
                }
            }

            onStateChanged: {
                console.debug("STATE CHANGED " + state)
            }

            states: [
                // Note: if "when" is true for multiple states then
                // the first state listed here takes precendence.

                State {
                    name: "PRESSED"
                    when: leftClickArea.pressed

                    PropertyChanges { target: cellBackground; opacity: 0.75 }
                },

                State {
                    name: "SELECTED"
                    when: selected

                    PropertyChanges { target: cellBackground; opacity: 0.5 }
                },

                State {
                    name: "HOVERED"
                    when: highlighted

                    PropertyChanges { target: cellBackground; opacity: 0.2 }
                }
            ]

            readonly property var toolTip: model.toolTip

            onHoveredChanged: {
                if (hovered) {
                    mscore.tooltip.item = paletteCell;
                    mscore.tooltip.text = paletteCell.toolTip ? paletteCell.toolTip : "";
                } else if (mscore.tooltip.item == paletteCell)
                    mscore.tooltip.item = null;
            }

            text: model.accessibleText; // Accessible.name is ignored for some reason
            Accessible.selectable: true;
            Accessible.selected: selected;

            Keys.onShortcutOverride: {
                // Intercept all keys that we want to use with Keys.onPressed
                // in case they are assigned as shortcuts in Preferences.
                event.accepted = true; // intercept everything
                switch (event.key) {
                    case Qt.Key_Space:
                    case Qt.Key_Enter:
                    case Qt.Key_Return:
                    case Qt.Key_Menu:
                    case Qt.Key_Asterisk:
                        return;
                }
                if (event.key === Qt.Key_F10 && event.modifiers & Qt.ShiftModifier)
                    return;
                if (event.text.match(/[^\x00-\x20\x7F]+$/) !== null)
                    return;
                event.accepted = false; // allow key to function as shortcut (don't intercept)
            }

            Keys.onPressed: {
                // NOTE: All keys must be intercepted with Keys.onShortcutOverride.
                const shiftHeld = event.modifiers & Qt.ShiftModifier;
                const ctrlHeld = event.modifiers & Qt.ControlModifier;
                switch (event.key) {
                    case Qt.Key_Space:
                        if (paletteTree.typeAheadStr.length)
                            paletteView.typeAheadFind(' ');
                        else
                            paletteView.updateSelection(true);
                        break;
                    case Qt.Key_Enter:
                    case Qt.Key_Return:
                        paletteView.selectionModel.setCurrentIndex(modelIndex, ItemSelectionModel.ClearAndSelect);
                        paletteView.paletteController.applyPaletteElement(modelIndex, mscore.keyboardModifiers());
                        break;
                    case Qt.Key_F10:
                        if (!shiftHeld)
                            return;
                        // fallthrough
                    case Qt.Key_Menu:
                        showCellMenu();
                        break;
                    case Qt.Key_Asterisk:
                        if (paletteTree.typeAheadStr.length)
                            paletteView.typeAheadFind('*');
                        else if (!paletteTree.expandCollapseAll(null))
                            paletteTree.currentItem.forceActiveFocus();
                        break;
                    default:
                        if (event.text.match(/[^\x00-\x20\x7F]+$/) !== null) {
                            // Pressed non-control character(s) (e.g. "D") so go
                            // to matching item (e.g. "D Major" in keysig palette)
                            paletteView.typeAheadFind(event.text);
                        }
                        else {
                            return; // don't accept event
                        }
                }
                event.accepted = true;
            }

            MouseArea {
                id: leftClickArea
                anchors.fill: parent
                drag.target: this

                onPressed: {
                    paletteView.currentIndex = paletteCell.rowIndex;
                    paletteCell.forceActiveFocus();
                    paletteView.updateSelection(true);
                    paletteCell.beginDrag();
                }

                onClicked: {
                    if (paletteView.paletteController.applyPaletteElement(paletteCell.modelIndex, mscore.keyboardModifiers()))
                        paletteView.selectionModel.setCurrentIndex(paletteCell.modelIndex, ItemSelectionModel.Current);
                }

                onDoubleClicked: {
                    const index = paletteCell.modelIndex;
                    paletteView.selectionModel.setCurrentIndex(index, ItemSelectionModel.Current);
                    paletteView.paletteController.applyPaletteElement(index, mouse.modifiers);
                }
            }

            MouseArea {
                id: rightClickArea
                anchors.fill: parent
                acceptedButtons: Qt.RightButton

                onClicked: showCellMenu(true)
            }

            Drag.active: leftClickArea.drag.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.CopyAction | (model.editable ? Qt.MoveAction : 0)
            Drag.mimeData: Drag.active ? mimeData : {}

            onInternalDragChanged: {
                if (internalDrag && dragDropReorderTimer.running)
                    return;
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
                    if (data.action == Qt.MoveAction && data.srcParentModelIndex == data.paletteView.paletteRootIndex)
                        data.paletteView.moveCell(data.srcRowIndex, data.destIndex);
                    else
                        data.paletteView.insertCell(data.destIndex, data.mimeData, data.action);

                    dropData = null;
                }
            }
//                             Drag.hotSpot: Qt.point(64, 0) // TODO

            function beginDrag() {
                icon.grabToImage(function(result) {
                        Drag.imageSource = result.url
                        dragDropReorderTimer.restart();
                    })
            }

            function showCellMenu(useCursorPos) {
                if (useCursorPos === undefined)
                    useCursorPos = false;
                contextMenu.modelIndex = modelIndex;
                contextMenu.canEdit = paletteView.paletteController.canEdit(paletteView.paletteRootIndex);
                if (useCursorPos)
                    contextMenu.popup();
                else {
                    contextMenu.x = x + width;
                    contextMenu.y = y;
                    contextMenu.open();
                }
            }

            Connections {
                // force not hiding palette cell if it is being dragged to a score
                enabled: paletteCell.paletteDrag
                target: mscore
                function onElementDraggedToScoreView() { paletteCell.paletteDrag = false }
            }
        } // end ItemDelegate
    } // end DelegateModel

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
