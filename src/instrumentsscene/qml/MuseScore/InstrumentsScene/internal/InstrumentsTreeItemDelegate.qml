/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.InstrumentsScene 1.0

FocusableControl {
    id: root

    property var treeView: undefined
    property var index: styleData.index
    property string filterKey
    property int type: InstrumentsTreeItemType.UNDEFINED
    property bool isSelected: false
    property bool isDragAvailable: false
    property alias isExpandable: expandButton.visible
    property alias isEditable: settingsButton.visible

    property int sideMargin: 0

    property var popupAnchorItem: null

    signal clicked(var mouse)
    signal doubleClicked(var mouse)
    signal removeSelectionRequested()

    signal popupOpened(var popupX, var popupY, var popupHeight)
    signal popupClosed()

    QtObject {
        id: prv

        property bool dragged: mouseArea.drag.active && mouseArea.pressed

        onDraggedChanged: {
            if (dragged && styleData.isExpanded) {
                root.treeView.collapse(styleData.index)
            }
        }

        property var openedPopup: null
        property bool isPopupOpened: Boolean(openedPopup) && openedPopup.isOpened

        function openPopup(popup, item) {
            if (Boolean(popup)) {
                openedPopup = popup
                popup.load(item)
                root.popupOpened(popup.x, popup.y, popup.height)
                popup.open()
            }
        }

        function closeOpenedPopup() {
            if (isPopupOpened) {
                openedPopup.close()
                resetOpenedPopup()
            }
        }

        function resetOpenedPopup() {
            root.popupClosed()
            openedPopup = null
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

        function createPopup(comp, btn) {
            popupLoader.sourceComponent = comp
            popupLoader.item.parent = btn
            popupLoader.item.needActiveFirstItem = btn.navigation.highlight
            return popupLoader.item
        }
    }

    Component {
        id: instrumentSettingsComp

        InstrumentSettingsPopup {
            navigationParentControl: settingsButton.navigation
            anchorItem: popupAnchorItem

            onClosed: {
                prv.resetOpenedPopup()
                popupLoader.sourceComponent = null
            }
        }
    }

    Component {
        id: staffSettingsComp

        StaffSettingsPopup {
            navigationParentControl: settingsButton.navigation
            anchorItem: popupAnchorItem

            onClosed: {
                prv.resetOpenedPopup()
                popupLoader.sourceComponent = null
            }
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

                model.itemRole.isVisible = !isVisible
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.leftMargin: 12 * styleData.depth
            height: childrenRect.height

            FlatButton {
                id: expandButton
                anchors.left: parent.left

                objectName: "ExpandBtnInstrument"
                enabled: expandButton.visible
                navigation.panel: root.navigation.panel
                navigation.row: root.navigation.row
                navigation.column: 2
                navigation.accessible.name: qsTrc("instruments", "Expand")

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

            objectName: "SettingsBtnInstrument"
            enabled: root.visible
            navigation.panel: root.navigation.panel
            navigation.row: root.navigation.row
            navigation.column: 3
            navigation.accessible.name: qsTrc("instruments", "Settings")

            icon: IconCode.SETTINGS_COG

            onClicked: {
                if (prv.isPopupOpened) {
                    prv.closeOpenedPopup()
                    return
                }

                var popup = null
                var item = {}

                if (root.type === InstrumentsTreeItemType.PART) {

                    popup = popupLoader.createPopup(instrumentSettingsComp, this)

                    item["partId"] = model.itemRole.id
                    item["instrumentId"] = model.itemRole.instrumentId()

                } else if (root.type === InstrumentsTreeItemType.STAFF) {

                    popup = popupLoader.createPopup(staffSettingsComp, this)

                    item["id"] = model.itemRole.id
                }

                prv.openPopup(popup, item)
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
        }
    ]
}
