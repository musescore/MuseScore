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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

import "internal"

Item {
    id: root

    property alias model: sortFilterProxyModel.sourceModel

    property bool readOnly: false

    property string keyRoleName: "key"
    property string keyTitle: qsTrc("uicomponents", "Key")
    property string valueRoleName: "value"
    property string valueTitle: qsTrc("uicomponents", "Value")
    property string valueTypeRole: "valueType"
    property string valueEnabledRoleName: "enabled"
    property string iconRoleName: "icon"

    property alias hasSelection: selectionModel.hasSelection
    readonly property var selection: sortFilterProxyModel.mapSelectionToSource(selectionModel.selection)

    property NavigationSection navigationSection: null
    property int navigationOrderStart: 0

    signal handleItem(var index, var item)

    QtObject {
        id: privateProperties

        property real valueItemWidth: 126
        property real spacing: 4
        property real sideMargin: 30

        function toggleSorter(sorter) {
            if (!sorter.enabled) {
                setSorterEnabled(sorter, true)
            } else if (sorter.sortOrder === Qt.AscendingOrder) {
                sorter.sortOrder = Qt.DescendingOrder
            } else {
                setSorterEnabled(sorter, false)
            }

            selectionModel.clear()
        }

        function setSorterEnabled(sorter, enable) {
            sorter.enabled = enable
        }
    }

    Rectangle {
        id: background
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor
        border.width: 1
        border.color: ui.theme.strokeColor
    }

    SortFilterProxyModel {
        id: sortFilterProxyModel

        sorters: [
            SorterValue {
                id: keySorter
                roleName: keyRoleName
            },
            SorterValue {
                id: valueSorter
                roleName: valueRoleName
            }
        ]
    }

    ItemMultiSelectionModel {
        id: selectionModel

        model: sortFilterProxyModel
    }

    RowLayout {
        id: header

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 38

        property NavigationPanel headerNavigation: NavigationPanel {
            name: "ValueListHeaderPanel"
            section: root.navigationSection
            direction: NavigationPanel.Horizontal
            order: root.navigationOrderStart
            accessible.name: qsTrc("uicomponents", "Value list header panel")
            enabled: root.visible

            onActiveChanged: {
                if (active) {
                    root.forceActiveFocus()
                }
            }
        }

        ValueListHeaderItem {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.leftMargin: privateProperties.sideMargin

            headerTitle: keyTitle
            spacing: privateProperties.spacing
            isSorterEnabled: keySorter.enabled
            sortOrder: keySorter.sortOrder

            navigation.panel: header.headerNavigation
            navigation.order: 1

            onClicked: {
                privateProperties.toggleSorter(keySorter)
                privateProperties.setSorterEnabled(valueSorter, false)
            }
        }

        ValueListHeaderItem {
            Layout.preferredWidth: privateProperties.valueItemWidth
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: privateProperties.sideMargin

            headerTitle: valueTitle
            spacing: privateProperties.spacing
            isSorterEnabled: valueSorter.enabled
            sortOrder: valueSorter.sortOrder

            navigation.panel: header.headerNavigation
            navigation.order: 2

            onClicked: {
                privateProperties.toggleSorter(valueSorter)
                privateProperties.setSorterEnabled(keySorter, false)
            }
        }
    }

    ListView {
        id: view

        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.leftMargin: background.border.width
        anchors.right: parent.right
        anchors.rightMargin: background.border.width
        anchors.bottom: parent.bottom
        anchors.bottomMargin: background.border.width

        model: sortFilterProxyModel

        clip: true
        boundsBehavior: Flickable.StopAtBounds

        property NavigationPanel navigation: NavigationPanel {
            name: "ValueListPanel"
            section: root.navigationSection
            direction: NavigationPanel.Both
            order: root.navigationOrderStart + 1
            accessible.name: qsTrc("uicomponents", "Value list panel")
            enabled: root.visible

            onActiveChanged: {
                if (active) {
                    root.forceActiveFocus()
                }
            }
        }

        ScrollBar.vertical: StyledScrollBar {
            anchors.right: parent.right
            anchors.rightMargin: 8
        }

        delegate: Item {
            //! NOTE: Added to prevent components clipping when navigating
            width: view.width
            height: listItem.height + ui.theme.navCtrlBorderWidth * 2

            ValueListItem {
                id: listItem

                anchors.centerIn: parent
                width: view.width - ui.theme.navCtrlBorderWidth * 2

                item: model

                property var modelIndex: sortFilterProxyModel.index(model.index, 0)

                keyRoleName: root.keyRoleName
                valueRoleName: root.valueRoleName
                valueTypeRole: root.valueTypeRole
                valueEnabledRoleName: root.valueEnabledRoleName
                iconRoleName: root.iconRoleName

                isSelected: selectionModel.hasSelection && selectionModel.isSelected(modelIndex)
                readOnly: root.readOnly

                spacing: privateProperties.spacing
                sideMargin: privateProperties.sideMargin
                valueItemWidth: privateProperties.valueItemWidth

                navigation.panel: view.navigation
                navigation.enabled: enabled
                navigation.row: model.index
                navigation.column: 0

                navigation.onNavigationEvent: {
                    switch (event.type) {
                    case NavigationEvent.Up:
                        if (model.index === 0) {
                            event.accepted = true
                        }
                        break
                    case NavigationEvent.Down:
                        if (model.index === view.model.rowCount() - 1) {
                            event.accepted = true
                        }
                        break
                    }
                }

                onClicked: {
                    selectionModel.select(modelIndex)
                }

                onDoubleClicked: {
                    root.handleItem(sortFilterProxyModel.mapToSource(modelIndex), item)
                }

                onNavigationTriggered: {
                    root.handleItem(sortFilterProxyModel.mapToSource(modelIndex), item)
                }

                onFocusChanged: {
                    if (activeFocus) {
                        view.positionViewAtIndex(index, ListView.Contain)
                    }
                }
            }
        }
    }
}
