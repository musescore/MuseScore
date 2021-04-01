import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

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

    signal doubleClicked(var index, var item)

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

        ValueListHeaderItem {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.leftMargin: privateProperties.sideMargin

            headerTitle: keyTitle
            spacing: privateProperties.spacing
            isSorterEnabled: keySorter.enabled
            sortOrder: keySorter.sortOrder

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

        ScrollBar.vertical: StyledScrollBar {
            parent: view.parent

            Layout.alignment: Qt.AlignTop | Qt.AlignBottom | Qt.AlignRight
            Layout.rightMargin: 16

            visible: view.contentHeight > view.height
            z: 1
        }

        delegate: ValueListItem {
            item: model

            property var modelIndex: sortFilterProxyModel.index(item.index, 0)

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

            onClicked: {
                selectionModel.select(modelIndex)
            }

            onDoubleClicked: {
                root.doubleClicked(sortFilterProxyModel.mapToSource(modelIndex), item)
            }
        }
    }
}
