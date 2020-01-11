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

ListView {
    id: paletteTree
    Accessible.role: Accessible.Tree // makes NVDA say "TreeView"

    keyNavigationEnabled: true
    activeFocusOnTab: true

    property PaletteWorkspace paletteWorkspace
    property var paletteModel: paletteWorkspace ? paletteWorkspace.mainPaletteModel : null
    property PaletteController paletteController: paletteWorkspace ? paletteWorkspace.mainPaletteController : null

    // Scroll palettes list when dragging a palette close to the list's border
    property bool itemDragged: false
    preferredHighlightBegin: Math.min(48, Math.floor(0.1 * height))
    preferredHighlightEnd: Math.ceil(height - preferredHighlightBegin)
    highlightRangeMode: itemDragged ? ListView.ApplyRange : ListView.NoHighlightRange

    property Item currentTreeItem: currentItem // most recently focused item at any level of the tree

    property string filter: ""
    onFilterChanged: {
        if (filter.length) {
            paletteSelectionModel.clear();
            expandedPopupIndex = null;
            }
        if (paletteModel)
            paletteModel.setFilterFixedString(filter);
    }

    property bool enableAnimations: true

    function insertCustomPalette(idx) {
        if (paletteTree.paletteController.insertNewItem(paletteTreeDelegateModel.rootIndex, idx))
            positionViewAtIndex(idx, ListView.Contain);
    }

    ItemSelectionModel {
        id: paletteSelectionModel
        model: paletteTree.paletteModel
    }

    function applyCurrentElement() {
        var index = paletteSelectionModel.currentIndex;
        if (!index.valid && filter.length) {
            // in search and no cell selected: apply the first found element
            const parentIndex = paletteTreeDelegateModel.modelIndex(0);
            index = paletteModel.index(0, 0, parentIndex);
            }

        paletteController.applyPaletteElement(index, Qt.NoModifier);
    }

    property var expandedPopupIndex: null // TODO: or use selection model? That would allow to preserve popups on removing palettes

    onExpandedPopupIndexChanged: {
        if (footerItem)
            footerItem.height = 0;
    }

    onCurrentIndexChanged: {
        if (paletteSelectionModel.hasSelection && paletteSelectionModel.currentIndex.row != currentIndex)
            paletteSelectionModel.clearSelection();
    }

    function ensureYVisible(y) {
        if (y > footerItem.y)
            footerItem.height = y - footerItem.y

        if (y > contentY + height)
            contentY = y - height;
        else if (y < contentY)
            contentY = y;
    }

    function removeSelectedItems(parentIndex) {
        Utils.removeSelectedItems(paletteController, paletteSelectionModel, parentIndex);
    }

    Keys.onDeletePressed: {
        expandedPopupIndex = null;
        removeSelectedItems();
    }
    Keys.onPressed: {
        if (event.key == Qt.Key_Backspace) {
            expandedPopupIndex = null;
            removeSelectedItems();
            event.accepted = true;
        } else if (event.key == Qt.Key_Home) {
            positionViewAtBeginning();
            event.accepted = true;
        } else if (event.key == Qt.Key_End) {
            positionViewAtEnd();
            event.accepted = true;
        } else if (event.key == Qt.Key_PageUp) {
            var idx = indexAt(contentX, contentY);
            if (idx < 0)
                idx = 0;
            if (idx > 0 && itemAt(contentX, contentY).height > height)
                contentY -= height;
            else
                positionViewAtIndex(idx, ListView.End);
            event.accepted = true;
        } else if (event.key == Qt.Key_PageDown) {
            var idx = indexAt(contentX, contentY + height);
            if (idx < 0)
                idx = count - 1;
            if (idx < count - 1 && itemAt(contentX, contentY + height).height > height)
                contentY += height;
            else
                positionViewAtIndex(idx, ListView.Beginning);
            event.accepted = true;
        }
    }

    displaced: Transition {
        enabled: paletteTree.enableAnimations
        NumberAnimation { property: "y"; duration: 150 }
    }

    ScrollBar.vertical: ScrollBar {
        id: scrollbar

        readonly property color baseColor: (globalStyle.base.hslLightness > 0.5) ? "#28282a" : "#d7d7d5"
        readonly property color pressedColor: "#bdbebf"

        Component.onCompleted: contentItem.color = Qt.binding(function() { return scrollbar.pressed ? baseColor : "#bdbebf"; })
    }

    boundsBehavior: Flickable.StopAtBounds
    maximumFlickVelocity: 1500

    PlaceholderManager {
        id: placeholder
        delegateModel: paletteTreeDelegateModel
    }
    function placeholderData() {
        return { display: "", gridSize: Qt.size(1, 1), drawGrid: false, custom: false, editable: false, expanded: false };
    }

    function focusFirstItem() {
        currentIndex = 0;
        if (filter.length) // on searching jump directly to the found cells
            currentItem.focusFirstItem();
        else
            currentItem.forceActiveFocus();
    }

    function focusLastItem() {
        currentIndex = count - 1;
        if (filter.length) // on searching jump directly to the found cells
            currentItem.focusLastItem();
        else
            currentItem.forceActiveFocus();
    }

    function getTintedColor(baseColor, tintColor, opacity) {
        var tintColorWithOpacity = Qt.rgba(tintColor.r, tintColor.g, tintColor.b, opacity);
        return Qt.tint(baseColor, tintColorWithOpacity);
    }
    readonly property color selectionColor: getTintedColor(globalStyle.window, globalStyle.base, 0.85)
    readonly property color highlightColor: getTintedColor(globalStyle.window, globalStyle.base, 0.6)

    model: DelegateModel {
        id: paletteTreeDelegateModel
        model: paletteTree.paletteModel

        delegate: ItemDelegate {
            id: control
            topPadding: 0
            bottomPadding: expanded ? 4 : 0
            property int rowIndex: index
            property var modelIndex: paletteTree.model.modelIndex(index, 0)

            onActiveFocusChanged: {
                if (activeFocus)
                    paletteTree.currentTreeItem = this;
            }

            Component.onCompleted: {
                const w = paletteHeader.implicitWidth + leftPadding + rightPadding;
                paletteTree.implicitWidth = Math.max(paletteTree.implicitWidth, w);
            }

            function focusFirstItem() {
                mainPalette.currentIndex = 0;
                mainPalette.currentItem.forceActiveFocus();
            }

            function focusLastItem() {
                mainPalette.currentIndex = mainPalette.count - 1;
                mainPalette.currentItem.forceActiveFocus();
            }

            property bool expanded: filter.length || model.expanded
            function toggleExpand() {
                model.expanded = !expanded
            }

            property bool selected: paletteSelectionModel.hasSelection ? paletteSelectionModel.isSelected(modelIndex) : false
            onClicked: {
                forceActiveFocus();
                const cmd = selected ? ItemSelectionModel.Toggle : ItemSelectionModel.ClearAndSelect;
                paletteSelectionModel.setCurrentIndex(modelIndex, cmd);
                paletteTree.currentIndex = index;
            }
            onDoubleClicked: {
                forceActiveFocus();
                paletteSelectionModel.setCurrentIndex(modelIndex, ItemSelectionModel.Deselect);
                toggleExpand();
            }

            background: Rectangle {
                visible: !control.Drag.active
                z: -1
                color: control.selected ? paletteTree.selectionColor : (control.highlighted ? paletteTree.highlightColor : (control.down ? globalStyle.button : "transparent"))
            }

            highlighted: (activeFocus && !selected) || DelegateModel.isUnresolved

            property bool popupExpanded: paletteTree.expandedPopupIndex == modelIndex
            function togglePopup() {
                const expand = !popupExpanded;
                paletteTree.expandedPopupIndex = expand ? modelIndex : null;
            }

            property size cellSize: model.gridSize
            property bool drawGrid: model.drawGrid

            activeFocusOnTab: this === paletteTree.currentTreeItem

            function hidePalette() {
                paletteTree.expandedPopupIndex = null;
                paletteTree.paletteController.remove(modelIndex);
            }

            Keys.onPressed: {
                switch (event.key) {
                    case Qt.Key_Right:
                        if (expanded)
                            mainPalette.focus = true;
                        else
                            toggleExpand();
                        break;
                    case Qt.Key_Left:
                        if (expanded && !mainPalette.focus)
                            toggleExpand();
                        focus = true;
                        break;
                    case Qt.Key_Space:
                    case Qt.Key_Enter:
                    case Qt.Key_Return:
                        toggleExpand();
                        break;
                    default:
                        return; // don't accept event
                }
                event.accepted = true;
            }

            text: model.display

            width: parent.width

            Drag.active: paletteHeaderDragArea.drag.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.MoveAction
            Drag.proposedAction: Qt.MoveAction
            Drag.mimeData: { "application/musescore/palettetree": "" } // for keys filtering. TODO: make more reasonable MIME data?

            Drag.onDragStarted: {
                if (popupExpanded)
                    togglePopup();
                paletteTree.itemDragged = true;
                DelegateModel.inPersistedItems = true;
                DelegateModel.inItems = false;
                placeholder.makePlaceholder(control.rowIndex, paletteTree.placeholderData());
            }

            Drag.onDragFinished: {
                paletteTree.itemDragged = false;
                if (dropAction != Qt.IgnoreAction)
                    paletteTree.currentIndex = -1;
                const destIndex = placeholder.active ? placeholder.index : control.rowIndex;
                placeholder.removePlaceholder();
                const controller = paletteTree.paletteController;
                const root = paletteTreeDelegateModel.rootIndex;
                DelegateModel.inItems = true;
                DelegateModel.inPersistedItems = false;

                if (dropAction == Qt.MoveAction) {
                    controller.move(
                        root, rowIndex,
                        root, destIndex);
                }
            }

            DropArea {
                anchors { fill: parent/*; margins: 10*/ }
                keys: [ "application/musescore/palettetree" ]
                onEntered: {
                    const idx = control.DelegateModel.itemsIndex;
                    if (!control.DelegateModel.isUnresolved) {
                        placeholder.makePlaceholder(idx, paletteTree.placeholderData());
                        paletteTree.currentIndex = idx;
                        }
                }
                onDropped: {
                    if (drop.proposedAction == Qt.MoveAction)
                        drop.acceptProposedAction();
                }
            }

            contentItem: Column {
                visible: !control.DelegateModel.isUnresolved
                states: [
                    State {
                        name: "collapsed"
                        PropertyChanges { target: mainPaletteContainer; visible: false; restoreEntryValues: false }
                    },
                    State {
                        name: "expanded"
                        PropertyChanges { target: mainPaletteContainer; visible: true; restoreEntryValues: false }
                    },
                    State {
                        name: "dragged"
                        PropertyChanges { target: paletteHeader; text: ""; unresolved: true }
                        PropertyChanges { target: mainPaletteContainer; visible: false }
                    }
                ]

                transitions: [
                    Transition {
                        from: "collapsed"; to: "expanded"
                        enabled: paletteTree.enableAnimations
                        NumberAnimation { target: mainPaletteContainer; property: "height"; from: 0; to: mainPaletteContainer.implicitHeight; easing.type: Easing.OutCubic; duration: 150 }
                    },
                    Transition {
                        from: "expanded"; to: "collapsed"
                        enabled: paletteTree.enableAnimations
                        SequentialAnimation {
                            PropertyAction { target: mainPaletteContainer; property: "visible"; value: true } // temporarily set palette visible to animate it being hidden
                            NumberAnimation { target: mainPaletteContainer; property: "height"; from: mainPaletteContainer.implicitHeight; to: 0; easing.type: Easing.OutCubic; duration: 150 }
                            PropertyAction { target: mainPaletteContainer; property: "visible"; value: false } // make palette invisible again
                            PropertyAction { target: mainPaletteContainer; property: "height"; value: mainPaletteContainer.implicitHeight } // restore the height binding
                        }
                    }
                ]

                state: control.Drag.active ? "dragged" : (control.expanded ? "expanded" : "collapsed")

                TreePaletteHeader {
                    id: paletteHeader
                    width: parent.width
                    opacity: enabled ? 1 : 0.3
                    expanded: control.expanded
                    hovered: control.hovered
                    text: control.text
                    hidePaletteElementVisible: {
                        return !control.selected && control.expanded
                            && paletteSelectionModel.hasSelection && paletteSelectionModel.columnIntersectsSelection(0, control.modelIndex)
                            && paletteTree.paletteModel.parent(paletteSelectionModel.currentIndex) == control.modelIndex; // HACK to work around a (possible?) bug in columnIntersectsSelection
                    }
                    custom: model.custom

                    unresolved: control.DelegateModel.isUnresolved

                    onToggleExpandRequested: {
                        paletteTree.currentIndex = control.rowIndex;
                        control.toggleExpand();
                    }

                    editingEnabled: model.editable
                    onEnableEditingToggled: model.editable = val

                    onHideSelectedElementsRequested: paletteTree.removeSelectedItems(control.modelIndex);

                    onInsertNewPaletteRequested: paletteTree.insertCustomPalette(control.rowIndex);
                    onHidePaletteRequested: control.hidePalette();

                    paletteWorkspace: paletteTree.paletteWorkspace
                    modelIndex: control.modelIndex

                    onEditPalettePropertiesRequested: {
                        const modelIndex = control.modelIndex;
                        paletteTree.paletteController.editPaletteProperties(modelIndex);
                    }

                    MouseArea {
                        id: paletteHeaderDragArea
                        anchors.fill: parent
                        drag.target: this // TODO or control or paletteHeader?

                        drag.axis: Drag.YAxis

                        onPressed: control.grabToImage(function(result) {
                            control.Drag.imageSource = result.url
                        })

                        onClicked: control.onClicked(mouse)
                        onDoubleClicked: control.onDoubleClicked(mouse)
                    }
                }

                Rectangle {
                    id: mainPaletteContainer
                    readonly property int padding: 1
                    implicitHeight: mainPalette.implicitHeight + 2 * padding
                    implicitWidth: parent.width
                    height: implicitHeight
                    border { width: 1; color: enabled ? "black" : "#33000000" }

                    Palette {
                        id: mainPalette
                        anchors { fill: parent; margins: parent.padding }

                        cellSize: control.cellSize
                        drawGrid: control.drawGrid

                        paletteModel: control.DelegateModel.isUnresolved ? null : paletteTree.paletteModel
                        paletteRootIndex: control.modelIndex
                        paletteController: paletteTree.paletteController
                        selectionModel: paletteSelectionModel

                        showMoreButton: !filter.length
                        onMoreButtonClicked: control.togglePopup();
                        onVisibleChanged: {
                            if (!visible && control.popupExpanded)
                                control.togglePopup();
                        }

                        enableAnimations: paletteTree.enableAnimations
                        externalDropBlocked: paletteTree.expandedPopupIndex && !control.popupExpanded // FIXME: find another way to prevent drops go under a popup
                    }
                }

                MoreElementsPopup {
                    id: palettePopup
                    visible: control.popupExpanded
                    maxHeight: Math.min(0.75 * paletteTree.height, 500)

                    y: mainPaletteContainer.y + mainPaletteContainer.height + Utils.style.popupMargin
                    width: parent.width

                    modal: false
                    focus: true
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside | Popup.CloseOnPressOutsideParent

                    // TODO: change settings to "hidden" model?
                    cellSize: control.cellSize
                    drawGrid: control.drawGrid

                    paletteName: control.text
                    paletteIsCustom: model.custom
                    paletteEditingEnabled: model.editable

                    onVisibleChanged: {
                        // build pool model on first popup appearance
                        if (visible && !poolPalette) {
                            poolPalette = paletteTree.paletteWorkspace.poolPaletteModel(control.modelIndex);
                            poolPaletteRootIndex = paletteTree.paletteWorkspace.poolPaletteIndex(control.modelIndex, poolPalette);
                            poolPaletteController = paletteTree.paletteWorkspace.poolPaletteController(poolPalette, control.modelIndex);

                            customPalette = paletteTree.paletteWorkspace.customElementsPaletteModel
                            customPaletteRootIndex = paletteTree.paletteWorkspace.customElementsPaletteIndex(control.modelIndex) // TODO: make a property binding? (but that works incorrectly)
                            customPaletteController = paletteTree.paletteWorkspace.customElementsPaletteController
                        }
                        // if closing by other reasons than pressing "More" button again (e.g. via Esc key), synchronize "expanded" status
                        if (control.popupExpanded != visible)
                            control.togglePopup();
                    }

                    property bool needScrollToBottom: false

                    onAboutToShow: {
                        needScrollToBottom = true;
                        if (implicitHeight)
                            scrollToPopupBottom();
                    }
                    onOpened: {
                        scrollToPopupBottom();
                        needScrollToBottom = false;
                        enablePaletteAnimations = true;
                    }
                    onClosed: enablePaletteAnimations = false

                    function scrollToPopupBottom() {
                        const popupBottom = implicitHeight + y + control.y + 14; // 14 for DropShadow in StyledPopup: depends on blur radius and vertical offset
                        paletteTree.ensureYVisible(popupBottom);
                    }

                    onImplicitHeightChanged: {
                        if (visible && (needScrollToBottom || atYEnd))
                            scrollToPopupBottom();
                    }

                    onAddElementsRequested: {
                        const parentIndex = control.modelIndex;
                        var idx = paletteTree.paletteModel.rowCount(parentIndex);
                        for (var i = 0; i < mimeDataList.length; i++) {
                            const mimeData = mimeDataList[i];
                            if (paletteTree.paletteController.insert(parentIndex, idx, mimeData, Qt.MoveAction))
                                idx++;
                        }
                    }
                }
            }
        }
    }

    // placeholder footer item to reserve a space for "More" popup to expand
    footer: Item { height: 0 }

    Connections {
        target: palettesWidget
        onHasFocusChanged: {
            if (!palettesWidget.hasFocus) {
                paletteSelectionModel.clearSelection();
                expandedPopupIndex = null;
            }
        }
    }
}
