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
    Accessible.name: qsTr("Palettes Tree, contains %n palette(s)", "", count)

    activeFocusOnTab: true // allow focus even when empty

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
    property int expandDuration: enableAnimations ? 150 : 0 // duration of expand / collapse animations

    function insertCustomPalette(idx) {
        if (paletteTree.paletteController.insertNewItem(paletteTreeDelegateModel.rootIndex, idx))
            positionViewAtIndex(idx, ListView.Contain);
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

    Keys.onShortcutOverride: {
        // Intercept all keys that we want to use with Keys.onPressed
        // in case they are assigned as shortcuts in Preferences.
        event.accepted = true; // intercept everything
        switch (event.key) {
            case Qt.Key_Down:
            case Qt.Key_Up:
            case Qt.Key_Home:
            case Qt.Key_End:
            case Qt.Key_PageUp:
            case Qt.Key_PageDown:
            case Qt.Key_Backspace:
            case Qt.Key_Delete:
                return;
        }
        event.accepted = false; // allow key to function as shortcut (don't intercept)
    }

    Keys.onPressed: {
        // NOTE: All keys must be intercepted with Keys.onShortcutOverride.
        switch (event.key) {
            case Qt.Key_Down:
                focusNextItem();
                break;
            case Qt.Key_Up:
                focusPreviousItem();
                break;
            case Qt.Key_Home:
                focusFirstItem();
                break;
            case Qt.Key_End:
                focusLastItem();
                break;
            case Qt.Key_PageUp:
                focusPreviousPageItem();
                break;
            case Qt.Key_PageDown:
                focusNextPageItem();
                break;
            case Qt.Key_Backspace:
            case Qt.Key_Delete:
                expandedPopupIndex = null;
                removeSelectedItems();
                break;
            default:
                return; // don't accept event
        }
        event.accepted = true;
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

    function focusNextItem(includeChildren) {
        if (includeChildren === undefined) // https://stackoverflow.com/a/44128406
            includeChildren = true;

        if (includeChildren && currentItem.expanded) {
            currentItem.focusFirstItem();
            return;
        }

        if (currentIndex == count - 1)
            return; // no next item

        incrementCurrentIndex();
        currentItem.forceActiveFocus();
        positionViewAtIndex(currentIndex, ListView.Contain);
    }

    function focusPreviousItem(includeChildren) {
        if (includeChildren === undefined) // https://stackoverflow.com/a/44128406
            includeChildren = true;

        if (currentIndex == 0)
            return; // no previous item

        decrementCurrentIndex();

        if (includeChildren && currentItem.expanded)
            currentItem.focusLastItem();
        else
            currentItem.forceActiveFocus();

        positionViewAtIndex(currentIndex, ListView.Contain);
    }

    function focusNextPageItem() {
        if (currentIndex < count - 1) {
            currentIndex++; // move by at least one item
            // try to keep going, but new item must stay entirely in view
            var distance = currentItem.height;
            while (currentIndex < count - 1) {
                currentIndex++; // try another
                distance += currentItem.height;
                if (distance > height) {
                    currentIndex--; // too far, go back one
                    break;
                }
            }
        }
        currentItem.forceActiveFocus();
        positionViewAtIndex(currentIndex, ListView.Contain);
    }

    function focusPreviousPageItem() {
        if (currentIndex > 0) {
            currentIndex--; // move by at least one item
            // try to keep going, but new item must stay entirely in view
            var distance = currentItem.height;
            while (currentIndex > 0) {
                currentIndex--; // try another
                distance += currentItem.height;
                if (distance > height) {
                    currentIndex++; // too far, go back one
                    break;
                }
            }
        }
        currentItem.forceActiveFocus();
        positionViewAtIndex(currentIndex, ListView.Contain);
    }

    function focusFirstItem() {
        currentIndex = 0;
        currentItem.forceActiveFocus();
        positionViewAtIndex(currentIndex, ListView.Contain);
    }

    function focusLastItem() {
        currentIndex = count - 1;
        if (currentItem.expanded)
            currentItem.focusLastItem();
        else
            currentItem.forceActiveFocus();
        positionViewAtIndex(currentIndex, ListView.Contain);
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
            if (found || !sameChr)
                return;
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
        if (typeAheadStr.length)
            typeAheadTimer.restart();
    }

    function expandCollapseAll(expand) {
        console.assert([true, false, null].indexOf(expand) !== -1, "Invalid value for expand: " + expand);
        // expand = true  - expand all
        //          false - collapse all
        //          null  - decide based on current state
        if (expand === null) {
            // if any are collapsed then expand all, otherwise collapse all
            const startIndex = paletteModel.index(0, 0);
            const collapsedIndexList = paletteModel.match(startIndex, PaletteTreeModel.PaletteExpandedRole, false);
            expand = !!collapsedIndexList.length;
        }
        for (var idx = 0; idx < count; idx++) {
            const paletteIndex = paletteModel.index(idx, 0);
            paletteModel.setData(paletteIndex, expand, PaletteTreeModel.PaletteExpandedRole);
        }
        currentItem.bringIntoViewAfterExpanding();
        return expand; // bool, did we expand?
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

            Component.onDestruction: {
                if (paletteTree.paletteWorkspace.needsItemDestructionAccessibilityWorkaround())
                    Utils.setInvisibleRecursive(this);
            }

            onActiveFocusChanged: {
                if (activeFocus)
                    paletteTree.currentTreeItem = this;
            }

            Component.onCompleted: {
                const w = paletteHeader.implicitWidth + leftPadding + rightPadding;
                paletteTree.implicitWidth = Math.max(paletteTree.implicitWidth, w);
            }

            function focusFirstItem() {
                mainPalette.focusFirstItem();
            }

            function focusLastItem() {
                mainPalette.focusLastItem();
            }

            property bool expanded: filter.length || model.expanded
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
                if (ListView.isCurrentItem && !filter.length)
                    bringIntoViewAfterExpanding();
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

            Keys.onShortcutOverride: {
                // Intercept all keys that we want to use with Keys.onPressed
                // in case they are assigned as shortcuts in Preferences.
                event.accepted = true; // intercept everything
                switch (event.key) {
                    case Qt.Key_Right:
                    case Qt.Key_Plus:
                    case Qt.Key_Left:
                    case Qt.Key_Minus:
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
                switch (event.key) {
                    case Qt.Key_Right:
                    case Qt.Key_Plus:
                        if (!expanded)
                            toggleExpand();
                        else if (event.key === Qt.Key_Right)
                            focusFirstItem();
                        break;
                    case Qt.Key_Left:
                    case Qt.Key_Minus:
                        if (expanded)
                            toggleExpand();
                        break;
                    case Qt.Key_Space:
                        if (paletteTree.typeAheadStr.length) {
                            paletteTree.typeAheadFind(' ');
                            break;
                        }
                        // fallthrough
                    case Qt.Key_Enter:
                    case Qt.Key_Return:
                        toggleExpand();
                        break;
                    case Qt.Key_F10:
                        if (!(event.modifiers & Qt.ShiftModifier))
                            return;
                        // fallthrough
                    case Qt.Key_Menu:
                        paletteHeader.showPaletteMenu();
                        break;
                    case Qt.Key_Asterisk:
                        if (paletteTree.typeAheadStr.length)
                            paletteTree.typeAheadFind('*');
                        else
                            paletteTree.expandCollapseAll(null);
                        break;
                    default:
                        if (event.text.match(/[^\x00-\x20\x7F]+$/) !== null) {
                            // Pressed non-control character(s) (e.g. "L")
                            // so go to matching palette (e.g. "Lines")
                            paletteTree.typeAheadFind(event.text);
                        }
                        else {
                            return; // don't accept event
                        }
                }
                event.accepted = true;
            }

            text: filter.length ? qsTr("%1, contains %n matching element(s)", "palette", mainPalette.count).arg(model.accessibleText)
                                : model.expanded ? qsTr("%1 expanded", "tree item not collapsed").arg(model.accessibleText)
                                                 : model.accessibleText
            Accessible.role: Accessible.TreeItem

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
                    opacity: enabled ? 1 : 0.3
                    expanded: control.expanded
                    hovered: control.hovered
                    text: model.display
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

                    paletteName: model.display
                    paletteIsCustom: model.custom
                    paletteEditingEnabled: model.editable

                    Component.onDestruction: {
                        if (paletteTree.paletteWorkspace.needsItemDestructionAccessibilityWorkaround())
                            Utils.setInvisibleRecursive(this);
                    }

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
        function onHasFocusChanged() {
            if (!palettesWidget.hasFocus) {
                paletteSelectionModel.clearSelection();
                expandedPopupIndex = null;
            }
        }
    }
}
