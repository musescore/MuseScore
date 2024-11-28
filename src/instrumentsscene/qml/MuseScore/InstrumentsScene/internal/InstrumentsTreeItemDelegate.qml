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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.InstrumentsScene 1.0

FocusableControl {
    id: root

    required property var originalParent

    property var item: null
    property TreeView treeView: undefined
    property var index: styleData.index
    property string filterKey

    readonly property int type: item ? item.type : InstrumentsTreeItemType.UNDEFINED
    readonly property bool isSelected: item && item.isSelected
    readonly property bool isDragAvailable: item && item.isSelectable
    readonly property bool isExpandable: item && item.isExpandable
    readonly property bool isEditable: item && item.isEditable

    property int sideMargin: 0

    property var popupAnchorItem: null

    signal clicked(var mouse)
    signal doubleClicked(var mouse)
    signal removeSelectionRequested()

    signal popupOpened(var popupX, var popupY, var popupHeight)
    signal popupClosed()

    signal visibilityChanged(bool visible)

    signal dragStarted()
    signal dropped()

    QtObject {
        id: prv

        property bool dragged: mouseArea.drag.active && mouseArea.pressed

        onDraggedChanged: {
            if (dragged) {
                root.dragStarted()
                if (styleData.isExpanded) {
                    root.treeView.collapse(styleData.index)
                }
            } else {
                root.dropped()
            }
        }
    }

    anchors.verticalCenter: parent ? parent.verticalCenter : undefined
    anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined

    height: parent ? parent.height : implicitHeight
    width: parent ? parent.width : implicitWidth

    implicitHeight: 38
    implicitWidth: 248

    Drag.keys: [ root.filterKey ]
    Drag.active: prv.dragged && isDragAvailable
    Drag.source: root
    Drag.hotSpot.x: width / 2
    Drag.hotSpot.y: height / 2

    navigation.name: "InstrumentsTreeItemDelegate"
    navigation.column: 0

    navigation.accessible.role: MUAccessible.ListItem
    navigation.accessible.name: titleLabel.text

    onNavigationTriggered: { root.clicked(null) }

    mouseArea.preventStealing: true
    mouseArea.propagateComposedEvents: true

    mouseArea.hoverEnabled: root.visible

    mouseArea.onClicked: function(mouse) { root.clicked(mouse) }
    mouseArea.onDoubleClicked: function(mouse) { root.doubleClicked(mouse) }

    mouseArea.drag.target: root
    mouseArea.drag.axis: Drag.YAxis

    Keys.onShortcutOverride: function(event) {
        switch (event.key) {
        case Qt.Key_Backspace:
        case Qt.Key_Delete:
            event.accepted = true
            root.removeSelectionRequested()
            break
        default:
            break
        }
    }

    StyledDropShadow {
        id: shadow

        anchors.fill: parent
        source: background
        visible: false
    }

    Loader {
        id: popupLoader

        readonly property StyledPopupView openedPopup: popupLoader.item as StyledPopupView
        readonly property bool isPopupOpened: Boolean(openedPopup) && openedPopup.isOpened

        function openPopup(comp: Component, btn: Item, item) {
            popupLoader.sourceComponent = comp
            if (!openedPopup) {
                return
            }

            openedPopup.parent = btn
            openedPopup.needActiveFirstItem = btn.navigation.highlight

            openedPopup.load(item)

            openedPopup.opened.connect(function() {
                root.popupOpened(openedPopup.x, openedPopup.y, openedPopup.height)
            })

            openedPopup.closed.connect(function() {
                root.popupClosed()
                sourceComponent = null
            })

            openedPopup.open()
        }

        function closeOpenedPopup() {
            if (isPopupOpened) {
                openedPopup.close()
            }
        }
    }

    Component {
        id: instrumentSettingsComp

        InstrumentSettingsPopup {
            anchorItem: popupAnchorItem

            onReplaceInstrumentRequested: {
                // The popup would close when the dialog to select the new
                // instrument is shown; when it closes, it is unloaded, i.e.
                // deleted, which means that it is deleted while a signal
                // handler inside it is being executed. This causes a crash.
                // To prevent that, let the popup close itself, and perform the
                // actual operation "later", i.e. not (directly or indirectly)
                // inside the signal handler in the popup.
                Qt.callLater(model.itemRole.replaceInstrument)
            }

            onResetAllFormattingRequested: {
                // Same as above
                Qt.callLater(model.itemRole.resetAllFormatting)
            }
        }
    }

    Component {
        id: staffSettingsComp

        StaffSettingsPopup {
            anchorItem: popupAnchorItem
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: root.sideMargin
        anchors.rightMargin: root.sideMargin

        spacing: 2

        VisibilityBox {
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: width

            objectName: "VisibleBtnInstrument"
            navigation.panel: root.navigation.panel
            navigation.row: root.navigation.row
            navigation.column: 1

            isVisible: model && model.itemRole.isVisible

            onVisibleToggled: {
                if (!model) {
                    return
                }

                if (root.isSelected) {
                    root.visibilityChanged(!isVisible)
                } else {
                    model.itemRole.isVisible = !isVisible
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.leftMargin: 12 * styleData.depth
            height: childrenRect.height

            FlatButton {
                id: expandButton
                anchors.left: parent.left

                visible: root.isExpandable

                objectName: "ExpandBtnInstrument"
                enabled: expandButton.visible
                navigation.panel: root.navigation.panel
                navigation.row: root.navigation.row
                navigation.column: 2
                navigation.accessible.name: styleData.isExpanded
                                            //: Collapse a tree item
                                            ? qsTrc("global", "Collapse")
                                            //: Expand a tree item
                                            : qsTrc("global", "Expand")

                transparent: true
                icon: styleData.isExpanded ? IconCode.SMALL_ARROW_DOWN : IconCode.SMALL_ARROW_RIGHT

                onClicked: {
                    if (!styleData.isExpanded) {
                        root.treeView.expand(styleData.index)
                    } else {
                        root.treeView.collapse(styleData.index)
                    }
                }
            }

            StyledTextLabel {
                id: titleLabel

                anchors.left: expandButton.right
                anchors.leftMargin: 4
                anchors.right: parent.right
                anchors.rightMargin: 8
                anchors.verticalCenter: expandButton.verticalCenter

                text: model ? model.itemRole.title : ""
                horizontalAlignment: Text.AlignLeft
                opacity: model && model.itemRole.isVisible ? 1 : 0.75

                font: {
                    if (Boolean(model) && root.type === InstrumentsTreeItemType.PART && model.itemRole.isVisible) {
                        return ui.theme.bodyBoldFont
                    }

                    return ui.theme.bodyFont
                }
            }
        }

        FlatButton {
            id: settingsButton

            Layout.alignment: Qt.AlignRight
            Layout.preferredWidth: width

            visible: root.isEditable

            objectName: "SettingsBtnInstrument"
            enabled: root.visible
            navigation.panel: root.navigation.panel
            navigation.row: root.navigation.row
            navigation.column: 3
            navigation.accessible.name: qsTrc("instruments", "Settings")

            icon: IconCode.SETTINGS_COG

            onClicked: {
                if (popupLoader.isPopupOpened) {
                    popupLoader.closeOpenedPopup()
                    return
                }

                let comp = null
                let item = {}

                if (root.type === InstrumentsTreeItemType.PART) {
                    comp = instrumentSettingsComp

                    item["partId"] = model.itemRole.id
                    item["instrumentId"] = model.itemRole.instrumentId()
                } else if (root.type === InstrumentsTreeItemType.STAFF) {
                    comp = staffSettingsComp

                    item["id"] = model.itemRole.id
                }

                popupLoader.openPopup(comp, this, item)
            }

            Behavior on opacity {
                NumberAnimation { duration: 150 }
            }
        }
    }

    onVisibleChanged: {
        if (visible) {
            opacity = 1.0
        } else {
            opacity = 0.0
        }
    }

    Behavior on opacity {
        enabled: styleData.depth !== 0
        NumberAnimation { duration: 150 }
    }

    focusBorder.drawOutsideParent: false

    background.states: [
        State {
            name: "HOVERED"
            when: root.mouseArea.containsMouse && !root.mouseArea.pressed && !root.isSelected && !prv.dragged

            PropertyChanges {
                target: root.background
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "PRESSED"
            when: root.mouseArea.pressed && !root.isSelected && !prv.dragged

            PropertyChanges {
                target: root.background
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHit
            }
        },

        State {
            name: "SELECTED"
            when: root.isSelected && !root.mouseArea.containsMouse && !root.mouseArea.pressed

            PropertyChanges {
                target: root.background
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityNormal
            }
        },

        State {
            name: "SELECTED_HOVERED"
            when: root.isSelected && root.mouseArea.containsMouse && !root.mouseArea.pressed

            PropertyChanges {
                target: root.background
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityHover
            }
        },

        State {
            name: "SELECTED_PRESSED"
            when: root.isSelected && root.mouseArea.pressed

            PropertyChanges {
                target: root.background
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityHit
            }
        },

        State {
            name: "PART_EXPANDED"
            when: styleData.isExpanded && !root.isSelected &&
                  root.type === InstrumentsTreeItemType.PART

            PropertyChanges {
                target: root.background
                color: ui.theme.textFieldColor
                opacity: 1
            }
        },

        State {
            name: "PARENT_EXPANDED"
            when: root.visible && !root.isSelected &&
                  (root.type === InstrumentsTreeItemType.INSTRUMENT ||
                   root.type === InstrumentsTreeItemType.STAFF)

            PropertyChanges {
                target: root.background
                color: ui.theme.textFieldColor
                opacity: 1
            }
        }
    ]

    states: [
        State {
            when: prv.dragged
            name: "DRAGGED"

            ParentChange {
                target: root
                parent: root.treeView.contentItem
            }

            PropertyChanges {
                target: shadow
                visible: true
            }

            PropertyChanges {
                target: root
                height: implicitHeight
                width: treeView.contentItem.width
            }

            AnchorChanges {
                target: root
                anchors {
                    verticalCenter: undefined
                    horizontalCenter: undefined
                }
            }
        },

        //! NOTE: Workaround for a bug in Qt 6.2.4 - see PR #24106 comment
        // https://bugreports.qt.io/browse/QTBUG-99436
        State {
            when: !prv.dragged
            name: "DROPPED"

            ParentChange {
                target: root
                parent: root.originalParent
            }
        }

    ]
}
