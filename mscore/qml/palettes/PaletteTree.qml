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
    implicitHeight: contentHeight

//         interactive: false
    keyNavigationEnabled: true
    activeFocusOnTab: true

    property PaletteWorkspace paletteWorkspace
    property var paletteModel: paletteWorkspace ? paletteWorkspace.mainPaletteModel : null
    property PaletteController paletteController: paletteWorkspace ? paletteWorkspace.mainPaletteController : null

    property string filter: ""
    onFilterChanged: {
        if (paletteModel)
            paletteModel.setFilterFixedString(filter);
    }

    ItemSelectionModel {
        id: paletteSelectionModel
        model: paletteTree.paletteModel
    }

    ItemSelectionModel {
        id: paletteExpandedModel
        model: paletteTree.paletteModel
    }

    property var expandedPopupIndex: null // TODO: or use selection model? That would allow to preserve popups on removing palettes

    onExpandedPopupIndexChanged: {
        if (footerItem)
            footerItem.height = 0;
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
        Utils.removeSelectedItems(paletteModel, paletteController, paletteSelectionModel, parentIndex);
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
        }
    }

    ScrollBar.vertical: ScrollBar {}

    PlaceholderManager {
        id: placeholder
        delegateModel: paletteTreeDelegateModel
    }

    model: DelegateModel {
        id: paletteTreeDelegateModel
        model: paletteTree.paletteModel

        delegate: ItemDelegate {
            id: control
            topPadding: 0
            bottomPadding: 0
            property int rowIndex: index
            property var modelIndex: paletteTree.model.modelIndex(index, 0)

            property bool expanded: filter.length || (paletteExpandedModel.hasSelection ? paletteExpandedModel.isSelected(modelIndex) : false)
            function toggleExpand() {
                paletteExpandedModel.select(modelIndex, ItemSelectionModel.Toggle);
            }

            property bool selected: paletteSelectionModel.hasSelection ? paletteSelectionModel.isSelected(modelIndex) : false
            onClicked: {
                forceActiveFocus();
                const cmd = selected ? ItemSelectionModel.Toggle : ItemSelectionModel.ClearAndSelect;
                paletteSelectionModel.setCurrentIndex(modelIndex, cmd);
                paletteTree.currentIndex = index;
            }
            Rectangle {
                z: -1
                anchors.fill: parent
                visible: control.selected
                color: "lightsteelblue";
                radius: 5
            }

            highlighted: activeFocus && !selected

            property bool popupExpanded: paletteTree.expandedPopupIndex == modelIndex
            function togglePopup() {
                const expand = !popupExpanded;
                paletteTree.expandedPopupIndex = expand ? modelIndex : null;
                if (expand)
                    palettePopup.needScrollToBottom = true;
            }

            property size cellSize: model.gridSize
            property bool drawGrid: model.drawGrid

            activeFocusOnTab: true

            function hidePalette() {
                paletteTree.expandedPopupIndex = null;
                paletteTree.paletteController.remove(modelIndex);
            }

            Keys.onRightPressed: {
                if (expanded)
                    mainPalette.focus = true;
                else
                    toggleExpand();
            }
            Keys.onLeftPressed: {
                if (expanded && !mainPalette.focus)
                    toggleExpand();
                focus = true;
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
                DelegateModel.inPersistedItems = true;
                DelegateModel.inItems = false;
            }

            Drag.onDragFinished: {
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
                    if (rowIndex != -1)
                        placeholder.makePlaceholder(rowIndex, { display: "", gridSize: Qt.size(1, 1), drawGrid: false });
                }
                onDropped: {
                    if (drop.proposedAction == Qt.MoveAction)
                        drop.acceptProposedAction();
                }
            }

            contentItem: Column {
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
                        PropertyChanges { target: paletteHeader; text: "" }
                        PropertyChanges { target: mainPaletteContainer; visible: false }
                    }
                ]

                transitions: [
                    Transition {
                        from: "collapsed"; to: "expanded"
                        NumberAnimation { target: mainPaletteContainer; property: "height"; from: 0; to: mainPaletteContainer.implicitHeight; easing.type: Easing.OutCubic; duration: 150 }
                    },
                    Transition {
                        from: "expanded"; to: "collapsed"
                        SequentialAnimation {
                            PropertyAction { target: mainPaletteContainer; property: "visible"; value: true } // temporarily set palette visible to animate it being hidden
                            NumberAnimation { target: mainPaletteContainer; property: "height"; from: mainPaletteContainer.implicitHeight; to: 0; easing.type: Easing.OutCubic; duration: 150 }
                            PropertyAction { target: mainPaletteContainer; property: "visible"; value: false } // make palette invisible again
                        }
                    }
                ]

                state: control.Drag.active ? "dragged" : (control.expanded ? "expanded" : "collapsed")

                TreePaletteHeader {
                    id: paletteHeader
                    width: parent.width
                    expanded: control.expanded
                    text: control.text
                    hidePaletteElementVisible: {
                        return !control.selected && control.expanded
                            && paletteSelectionModel.hasSelection && paletteSelectionModel.columnIntersectsSelection(0, control.modelIndex)
                            && paletteTree.paletteModel.parent(paletteSelectionModel.currentIndex) == control.modelIndex; // HACK to work around a (possible?) bug in columnIntersectsSelection
                    }

                    onToggleExpandRequested: {
                        paletteTree.currentIndex = control.rowIndex;
                        control.toggleExpand();
                    }

                    onHideSelectedElementsRequested: paletteTree.removeSelectedItems(control.modelIndex);

                    onHidePaletteRequested: control.hidePalette();

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
                    property int padding: 1
                    implicitHeight: mainPalette.implicitHeight + 2 * padding
                    implicitWidth: mainPalette.implicitWidth + 2 * padding
                    height: implicitHeight
                    width: implicitWidth
                    border { width: 1; color: "black" }

                    Palette {
                        id: mainPalette
                        anchors { fill: parent; margins: parent.padding }
                        maxWidth: control.availableWidth - 2 * parent.padding

                        cellSize: control.cellSize
                        drawGrid: control.drawGrid

                        paletteModel: control.text.length ? paletteTree.paletteModel : null // TODO: other way to detect a placeholder?
                        paletteRootIndex: control.modelIndex
                        paletteController: paletteTree.paletteController
                        selectionModel: paletteSelectionModel

                        showMoreButton: true
                        onMoreButtonClicked: control.togglePopup();
                        onVisibleChanged: {
                            if (!visible && control.popupExpanded)
                                control.togglePopup();
                        }

                        externalMoveBlocked: paletteTree.expandedPopupIndex && !control.popupExpanded // FIXME: find another way to prevent drops go under a popup
                    }
                }

                MoreElementsPopup {
                    id: palettePopup
                    visible: control.popupExpanded

                    y: mainPaletteContainer.y + mainPaletteContainer.height
                    width: parent.width

                    modal: false
                    focus: true
                    clip: true
                    closePolicy: Popup.CloseOnEscape// | Popup.CloseOnPressOutside

                    // TODO: change settings to "hidden" model?
                    cellSize: control.cellSize
                    drawGrid: control.drawGrid

                    paletteName: control.text
                    paletteIsCustom: model.custom

                    onVisibleChanged: {
                        // build pool model on first popup appearance
                        if (visible && !poolPalette) {
                            poolPalette = paletteTree.paletteWorkspace.poolPaletteModel(control.modelIndex);
                            poolPaletteRootIndex = paletteTree.paletteWorkspace.poolPaletteIndex(control.modelIndex, poolPalette);
                            poolPaletteController = paletteTree.paletteWorkspace.poolPaletteController(poolPalette, control.modelIndex);

                            customPalette = paletteTree.paletteWorkspace.customElementsPaletteModel
                            customPaletteRootIndex = paletteTree.paletteWorkspace.customElementsPaletteIndex(poolPaletteRootIndex) // TODO: make a property binding? (but that works incorrectly)
                            customPaletteController = paletteTree.paletteWorkspace.customElementsPaletteController
                        }
                        // if closing by other reasons than pressing "More" button again (e.g. via Esc key), synchronize "expanded" status
                        if (control.popupExpanded != visible)
                            control.togglePopup();
                    }

                    property bool needScrollToBottom: false

                    function scrollToPopupBottom() {
                        needScrollToBottom = false;
                        const popupBottom = implicitHeight + y + control.y;
                        paletteTree.ensureYVisible(popupBottom);
                    }

                    onNeedScrollToBottomChanged: {
                        if (needScrollToBottom && implicitHeight)
                            scrollToPopupBottom();
                    }

                    onImplicitHeightChanged: {
                        if (needScrollToBottom)
                            scrollToPopupBottom();
                    }

                    onAddElementsRequested: {
                        const parentIndex = control.modelIndex;
                        var idx = paletteTree.paletteModel.rowCount(parentIndex);
                        for (var i = 0; i < mimeDataList.length; i++) {
                            const mimeData = mimeDataList[i];
                            if (paletteTree.paletteController.insert(parentIndex, idx, mimeData))
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
            if (!palettesWidget.hasFocus)
                paletteSelectionModel.clear();
        }
    }
}
