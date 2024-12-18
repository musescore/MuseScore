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
import QtQml.Models 2.2

import MuseScore.Palette 1.0
import Muse.Ui 1.0
import Muse.UiComponents 1.0

import "utils.js" as Utils

StyledListView {
    id: paletteTree

    property PaletteProvider paletteProvider
    property var paletteModel: Boolean(paletteProvider) ? paletteProvider.mainPaletteModel : null
    property PaletteController paletteController: paletteProvider ? paletteProvider.mainPaletteController : null

    property alias navigation: keynavTree

    // Scroll palettes list when dragging a palette close to the list's border
    property bool itemDragged: false

    property Item currentTreeItem: currentItem // most recently focused item at any level of the tree

    property string filter: ""
    property bool searchOpened: false

    property bool isResultFound: count !== 0

    property bool enableAnimations: true
    property int expandDuration: enableAnimations ? 150 : 0 // duration of expand / collapse animations

    preferredHighlightBegin: Math.min(48, Math.floor(0.1 * height))
    preferredHighlightEnd: Math.ceil(height - preferredHighlightBegin)
    highlightRangeMode: itemDragged ? ListView.ApplyRange : ListView.NoHighlightRange

    cacheBuffer: 3000

    NavigationPanel {
        id: keynavTree
        name: "PalettesTree"
        enabled: paletteTree.enabled && paletteTree.visible
        direction: NavigationPanel.Both
        onActiveChanged: function(active) {
            if (active) {
                paletteTree.forceActiveFocus()
            }
        }
    }

    onSearchOpenedChanged: {
        if (paletteProvider) {
            paletteProvider.setSearching(searchOpened)
        }
    }

    onFilterChanged: {
        if (Boolean(filter)) {
            paletteSelectionModel.clear()
            expandedPopupIndex = null
        }

        if (paletteProvider) {
            paletteProvider.setFilter(filter)
            paletteTree.positionViewAtBeginning()   // Scroll to the top after a search
        }
    }

    function insertCustomPalette(idx, paletteName) {
        if (paletteTree.paletteController.insertNewItem(paletteTreeDelegateModel.rootIndex, idx, paletteName)) {
            positionViewAtIndex(idx, ListView.Contain)
        }
    }

    ItemSelectionModel {
        id: paletteSelectionModel
        model: paletteTree.paletteModel

        function selectRange(endIndex) {
            const firstRow = currentIndex.row;
            const lastRow = endIndex.row;
            const parentIndex = endIndex.parent;
            const step = firstRow < lastRow ? 1 : -1;
            const endRow = lastRow + step;
            for (var row = firstRow; row !== endRow; row += step) {
                const idx = paletteTree.paletteModel.index(row, 0, parentIndex);
                select(idx, ItemSelectionModel.Select);
            }
        }
    }

    function applyCurrentElement() {
        var index = paletteSelectionModel.currentIndex;
        if (!index.valid && filter.length) {
            // in search and no cell selected: apply the first found element
            const parentIndex = paletteTreeDelegateModel.modelIndex(0);
            index = paletteModel.index(0, 0, parentIndex);
            paletteSelectionModel.select(index, ItemSelectionModel.Select);
        }

        paletteController.applyPaletteElement(index, Qt.NoModifier);
    }

    property var expandedPopupIndex: null // TODO: or use selection model? That would allow to preserve popups on removing palettes

    onExpandedPopupIndexChanged: {
        if (footerItem) {
            footerItem.height = 0
        }
    }

    onCurrentIndexChanged: {
        if (paletteSelectionModel.hasSelection && paletteSelectionModel.currentIndex.row !== currentIndex) {
            paletteSelectionModel.clearSelection();
        }
    }

    function ensureYVisible(y) {
        if (y > footerItem.y) {
            footerItem.height = y - footerItem.y
        }

        if (y > contentY + height) {
            contentY = y - height;
        } else if (y < contentY) {
            contentY = y;
        }
    }

    function removeSelectedItems(parentIndex) {
        Utils.removeSelectedItems(paletteController, paletteSelectionModel, parentIndex);
    }

    displaced: Transition {
        enabled: paletteTree.enableAnimations
        NumberAnimation { property: "y"; duration: 150 }
    }

    PlaceholderManager {
        id: placeholder
        delegateModel: paletteTreeDelegateModel
    }
    function placeholderData() {
        return { display: "", gridSize: Qt.size(1, 1), drawGrid: false, custom: false, editable: false, expanded: false };
    }

    function focusNextMatchingItem(str, startIndex) {
        const modelIndex = paletteModel.index(startIndex, 0);
        const matchedIndexList = paletteModel.match(modelIndex, Qt.ToolTipRole, str);

        if (matchedIndexList.length) {
            currentIndex = matchedIndexList[0].row;
            currentItem.forceActiveFocus();
            positionViewAtIndex(currentIndex, ListView.Contain);
            return true;
        }

        return false;
    }

    function typeAheadFind(chr) {
        if (typeAheadStr.length) {
            // continue search on current item
            const sameChr = chr === typeAheadStr;
            typeAheadStr += chr;
            const found = focusNextMatchingItem(typeAheadStr, currentIndex);

            if (found || !sameChr) {
                return;
            }
        }

        // start new search on next item
        typeAheadStr = chr;
        const nextIndex = (currentIndex === count - 1) ? 0 : currentIndex + 1;
        focusNextMatchingItem(chr, nextIndex);
    }

    property string typeAheadStr: ''

    Timer {
        id: typeAheadTimer
        interval: 1000
        onTriggered: typeAheadStr = ''
    }

    onTypeAheadStrChanged: {
        if (typeAheadStr.length) {
            typeAheadTimer.restart();
        }
    }

    function numberOfExpandedPalettes() {
        const startIndex = paletteModel.index(0, 0);
        const collapsedIndexList = paletteModel.match(startIndex, PaletteTreeModel.PaletteExpandedRole, true);
        return collapsedIndexList.length;
    }

    function numberOfCollapsedPalettes() {
        const startIndex = paletteModel.index(0, 0);
        const collapsedIndexList = paletteModel.match(startIndex, PaletteTreeModel.PaletteExpandedRole, false);
        return collapsedIndexList.length;
    }

    function canExpandAll() {
        return !paletteProvider.isSinglePalette && numberOfCollapsedPalettes() > 0
    }

    function expandCollapseAll(expand) {
        console.assert([true, false, null].indexOf(expand) !== -1, "Invalid value for expand: " + expand);
        // expand = true  - expand all
        //          false - collapse all
        //          null  - decide based on current state: expand if possible, collapse otherwise
        if (expand === null) {
            expand = canExpandAll()
        }

        for (let idx = 0; idx < count; idx++) {
            const paletteIndex = paletteModel.index(idx, 0);
            paletteModel.setData(paletteIndex, expand, PaletteTreeModel.PaletteExpandedRole);
        }

        currentItem.bringIntoViewAfterExpanding();

        return expand; // return true if we did expand
    }

    Connections {
        target: paletteProvider

        function onIsSinglePaletteChanged() {
            if (paletteProvider.isSinglePalette) {
                // Collapse all except first one
                let hasFoundExpandedPalette = false
                for (let idx = 0; idx < count; idx++) {
                    const paletteIndex = paletteModel.index(idx, 0);

                    if (hasFoundExpandedPalette) {
                        paletteModel.setData(paletteIndex, false, PaletteTreeModel.PaletteExpandedRole);
                        continue
                    }

                    if (paletteModel.data(paletteIndex, PaletteTreeModel.PaletteExpandedRole)) {
                        hasFoundExpandedPalette = true
                    }
                }
            }
        }
    }

    Loader {
        id: palettePopup
        active: false

        property var control: null
        property var model: null
        property alias isOpened: palettePopup.active

        sourceComponent: MoreElementsPopup {
            id: moreElementsPopup

            property var control: palettePopup.control
            property var model: palettePopup.model

            maxHeight: Math.min(0.75 * paletteTree.height, 500)

            // TODO: change settings to "hidden" model?
            cellSize: control.cellSize
            drawGrid: control.drawGrid

            paletteName: model.display
            paletteIsCustom: model.custom
            paletteEditingEnabled: model.editable

            onIsOpenedChanged: {
                // build pool model on first popup appearance
                if (visible && !poolPalette) {
                    poolPalette = paletteTree.paletteProvider.poolPaletteModel(control.modelIndex);
                    poolPaletteRootIndex = paletteTree.paletteProvider.poolPaletteIndex(control.modelIndex, poolPalette);
                    poolPaletteController = paletteTree.paletteProvider.poolPaletteController(poolPalette, control.modelIndex);

                    customPalette = paletteTree.paletteProvider.customElementsPaletteModel
                    customPaletteRootIndex = paletteTree.paletteProvider.customElementsPaletteIndex(control.modelIndex) // TODO: make a property binding? (but that works incorrectly)
                    customPaletteController = paletteTree.paletteProvider.customElementsPaletteController
                }
                if (!isOpened) {
                    paletteTree.expandedPopupIndex = null
                }
            }

            property bool needScrollToBottom: false

            onOpened: {
                scrollToPopupBottom();
                needScrollToBottom = false;
                enablePaletteAnimations = true;
            }

            onClosed: {
                enablePaletteAnimations = false;
                palettePopup.active = false;
            }

            function scrollToPopupBottom() {
                //! FIXME Not worked as should
//                        const popupBottom = implicitHeight + y + control.y + 14; // 14 for DropShadow in StyledPopup: depends on blur radius and vertical offset
//                        paletteTree.ensureYVisible(popupBottom);
            }

            onContentHeightChanged: {
                if (visible && (needScrollToBottom || atYEnd))
                    scrollToPopupBottom();
            }

            onAddElementsRequested: function(mimeDataList) {
                const parentIndex = control.modelIndex;
                var idx = paletteTree.paletteModel.rowCount(parentIndex);

                for (var i = 0; i < mimeDataList.length; i++) {
                    const mimeData = mimeDataList[i];

                    if (paletteTree.paletteController.insert(parentIndex, idx, mimeData, Qt.MoveAction)) {
                        idx++;
                    }
                }
            }
        }

        function close() {
            if (palettePopup.active) {
                palettePopup.item.close();
            }
        }

        function toggleOpened(model, control, parent) {
            if (palettePopup.active) {
                palettePopup.close();
                return;
            }

            palettePopup.parent = parent;
            palettePopup.model = model;
            palettePopup.control = control;

            palettePopup.active = true;
            palettePopup.item.setParentItem(parent);

            palettePopup.item.toggleOpened();
        }
    }

    model: DelegateModel {
        id: paletteTreeDelegateModel
        model: paletteTree.paletteModel

        delegate: ItemDelegate {
            id: control
            topPadding: 0
            bottomPadding: expanded ? 4 : 0
            property int rowIndex: index
            property int navigationRow: (index + 1) * 10000 // to make unique
            property var modelIndex: paletteTree.model.modelIndex(index)

            Keys.onShortcutOverride: function(event) {
                switch (event.key) {
                case Qt.Key_Backspace:
                case Qt.Key_Delete:
                    event.accepted = true
                    paletteTree.removeSelectedItems()
                    break
                default:
                    break
                }
            }

            onActiveFocusChanged: {
                if (activeFocus) {
                    paletteTree.currentTreeItem = this;
                }
            }

            Component.onCompleted: {
                const w = paletteHeader.implicitWidth + leftPadding + rightPadding;
                paletteTree.implicitWidth = Math.max(paletteTree.implicitWidth, w);
            }

            Component.onDestruction: {
                if (palettePopup.active && palettePopup.model === model) {
                    palettePopup.close()
                }
            }

            readonly property bool expanded: paletteTree.searchOpened || Boolean(model.expanded)

            function toggleExpand() {
                model.expanded = !expanded
            }

            function bringIntoViewAfterExpanding() {
                expandTimer.restart();
            }

            Timer {
                id: expandTimer
                interval: expandDuration + 50 // allow extra grace period
                onTriggered: paletteTree.positionViewAtIndex(index, ListView.Contain)
            }

            onExpandedChanged: {
                if (ListView.isCurrentItem && !filter.length) {
                    bringIntoViewAfterExpanding();
                }
            }

            property bool selected: paletteSelectionModel.hasSelection ? paletteSelectionModel.isSelected(modelIndex) : false

            onClicked: {
                forceActiveFocus();

                if (paletteProvider.isSingleClickToOpenPalette) {
                    toggleExpand()

                    if (selected && !expanded) {
                        paletteSelectionModel.clearSelection();
                    } else if (!selected) {
                        paletteSelectionModel.setCurrentIndex(modelIndex, ItemSelectionModel.ClearAndSelect);
                    }
                } else {
                    const cmd = selected ? ItemSelectionModel.Toggle : ItemSelectionModel.ClearAndSelect;
                    paletteSelectionModel.setCurrentIndex(modelIndex, cmd);
                }

                paletteTree.currentIndex = index;
            }

            onDoubleClicked: {
                if (paletteProvider.isSingleClickToOpenPalette) {
                    return;
                }

                forceActiveFocus();
                paletteSelectionModel.setCurrentIndex(modelIndex, ItemSelectionModel.Deselect);
                toggleExpand();
            }

            background: ListItemBlank {
                visible: !control.Drag.active
                isSelected: control.selected

                navigation.name: model.display
                navigation.panel: keynavTree
                navigation.row: control.navigationRow
                navigation.column: 0
                navigation.accessible.name: control.text
                enabled: control.visible
                navigation.onActiveChanged: {
                    if (navigation.active) {
                        forceActiveFocus();

                        if (!control.selected) {
                            paletteSelectionModel.setCurrentIndex(modelIndex, ItemSelectionModel.ClearAndSelect);
                        }

                        paletteTree.currentIndex = index;
                        paletteTree.positionViewAtIndex(control.rowIndex, ListView.Contain);
                    }
                }
                navigation.onTriggered: {
                    control.toggleExpand()
                }
            }

            highlighted: (activeFocus && !selected) || DelegateModel.isUnresolved

            property bool popupExpanded: palettePopup.isOpened
            onPopupExpandedChanged: {
                paletteTree.interactive = !popupExpanded
            }

            function togglePopup(btn) {
                const expand = !popupExpanded;
                paletteTree.expandedPopupIndex = expand ? modelIndex : null;
                palettePopup.toggleOpened(model, control, btn)
            }

            property size cellSize: model.gridSize
            property bool drawGrid: model.drawGrid

            function hidePalette() {
                paletteTree.expandedPopupIndex = null;
                paletteTree.paletteController.remove(modelIndex);
            }

            text: filter.length ? qsTrc("palette", "%1, contains %n matching element(s)", "", mainPalette.count).arg(model.accessibleText)
                                : model.expanded ? qsTrc("palette", "%1 expanded", "tree item not collapsed").arg(model.accessibleText)
                                                 : model.accessibleText

            width: ListView.view.width

            Drag.active: paletteProvider.isPaletteDragEnabled && paletteHeaderDragArea.drag.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.MoveAction
            Drag.proposedAction: Qt.MoveAction
            Drag.mimeData: { "application/musescore/palettetree": "" } // for keys filtering. TODO: make more reasonable MIME data?

            Drag.onDragStarted: {
                if (popupExpanded) {
                    togglePopup();
                }

                paletteHeader.closeContextMenu()

                paletteTree.itemDragged = true;
                DelegateModel.inPersistedItems = true;
                DelegateModel.inItems = false;
                placeholder.makePlaceholder(control.rowIndex, paletteTree.placeholderData());
            }

            Drag.onDragFinished: function(dropAction) {
                paletteTree.itemDragged = false;

                if (dropAction !== Qt.IgnoreAction) {
                    paletteTree.currentIndex = -1;
                }

                const destIndex = placeholder.active ? placeholder.index : control.rowIndex;
                placeholder.removePlaceholder();
                const controller = paletteTree.paletteController;
                const root = paletteTreeDelegateModel.rootIndex;
                DelegateModel.inItems = true;
                DelegateModel.inPersistedItems = false;

                if (dropAction === Qt.MoveAction) {
                    controller.move(
                                root, rowIndex,
                                root, destIndex);
                }
            }

            DropArea {
                anchors.fill: parent
                keys: [ "application/musescore/palettetree" ]

                onEntered: {
                    const idx = control.DelegateModel.itemsIndex;

                    if (!control.DelegateModel.isUnresolved) {
                        placeholder.makePlaceholder(idx, paletteTree.placeholderData());
                        paletteTree.currentIndex = idx;
                    }
                }

                onDropped: function(drop) {
                    if (drop.proposedAction === Qt.MoveAction)
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
                        NumberAnimation { target: mainPaletteContainer; property: "height"; from: 0; to: mainPaletteContainer.implicitHeight; easing.type: Easing.OutCubic; duration: paletteTree.expandDuration }
                    },
                    Transition {
                        from: "expanded"; to: "collapsed"
                        enabled: paletteTree.enableAnimations
                        SequentialAnimation {
                            PropertyAction { target: mainPaletteContainer; property: "visible"; value: true } // temporarily set palette visible to animate it being hidden
                            NumberAnimation { target: mainPaletteContainer; property: "height"; from: mainPaletteContainer.implicitHeight; to: 0; easing.type: Easing.OutCubic; duration: paletteTree.expandDuration }
                            PropertyAction { target: mainPaletteContainer; property: "visible"; value: false } // make palette invisible again
                            PropertyAction { target: mainPaletteContainer; property: "height"; value: mainPaletteContainer.implicitHeight } // restore the height binding
                        }
                    }
                ]

                state: control.Drag.active ? "dragged" : (control.expanded ? "expanded" : "collapsed")

                TreePaletteHeader {
                    id: paletteHeader
                    width: parent.width
                    opacity: enabled ? 1 : ui.theme.itemOpacityDisabled
                    expanded: control.expanded
                    hovered: control.hovered
                    text: model.display

                    isInVisibleArea: control.y >= paletteTree.contentY && control.y < (paletteTree.contentY + paletteTree.height)

                    navigationPanel: keynavTree
                    navigationRow: control.navigationRow

                    hidePaletteElementVisible: {
                        return !control.selected && control.expanded
                                && paletteSelectionModel.hasSelection && paletteSelectionModel.columnIntersectsSelection(0, control.modelIndex)
                                && paletteTree.paletteModel.parent(paletteSelectionModel.currentIndex) === control.modelIndex; // HACK to work around a (possible?) bug in columnIntersectsSelection
                    }

                    custom: model.custom

                    unresolved: control.DelegateModel.isUnresolved

                    onToggleExpandRequested: {
                        paletteTree.currentIndex = control.rowIndex;
                        control.toggleExpand();
                    }

                    editingEnabled: model.editable
                    onEnableEditingToggled: function(val) {
                        model.editable = val
                    }

                    onHideSelectedElementsRequested: paletteTree.removeSelectedItems(control.modelIndex);

                    onInsertNewPaletteRequested: paletteTree.insertCustomPalette(control.rowIndex, qsTrc("palette", "Untitled palette"));
                    onHidePaletteRequested: control.hidePalette();

                    paletteProvider: paletteTree.paletteProvider
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

                        onPressed: function(mouse) {
                            control.grabToImage(function(result) {
                                control.Drag.imageSource = result.url
                                control.Drag.hotSpot.x = mouse.x
                                control.Drag.hotSpot.y = mouse.y
                            })
                        }

                        onClicked: function(mouse) { control.clicked() }
                        onDoubleClicked: function(mouse) { control.doubleClicked() }
                    }
                }

                Rectangle {
                    id: mainPaletteContainer
                    readonly property int padding: 1
                    implicitHeight: mainPalette.implicitHeight + 2 * padding
                    implicitWidth: parent.width
                    height: implicitHeight
                    border { width: 1; color: ui.theme.strokeColor }

                    PaletteGridView {
                        id: mainPalette
                        anchors.fill: parent
                        anchors.margins: parent.padding

                        navigationPanel: keynavTree
                        navigationRow: control.navigationRow + 1

                        cellSize: control.cellSize
                        drawGrid: control.drawGrid

                        paletteModel: control.DelegateModel.isUnresolved ? null : paletteTree.paletteModel
                        paletteRootIndex: control.modelIndex
                        paletteController: paletteTree.paletteController
                        selectionModel: paletteSelectionModel

                        isInVisibleArea: {
                            var mainPaletteBottom = control.y + mainPalette.height
                            return mainPaletteBottom >= paletteTree.contentY && mainPaletteBottom < (paletteTree.contentY + paletteTree.height)
                        }

                        showMoreButton: !paletteTree.searchOpened
                        onMoreButtonClicked: function(btn) { control.togglePopup(btn) }

                        onVisibleChanged: {
                            if (!visible && control.popupExpanded) {
                                control.togglePopup();
                            }
                        }

                        enableAnimations: paletteTree.enableAnimations
                        externalDropBlocked: paletteTree.expandedPopupIndex && !control.popupExpanded // FIXME: find another way to prevent drops go under a popup
                    }
                }
            }
        }
    }

    // placeholder footer item to reserve a space for "More" popup to expand
    footer: Item { height: 0 }
}
