import QtQuick 2.9

import MuseScore.UiComponents 1.0
import MuseScore.Plugins 1.0

Item {
    id: root

    property alias model: filterModel.sourceModel
    property alias count: view.count

    property string title: ""

    property string search: ""
    property string selectedCategory: ""
    property bool installed: false

    signal pluginClicked(var plugin)

    height: view.height

    function resetSelectedPlugin() {
        view.currentIndex = -1
    }

    SortFilterProxyModel {
        id: filterModel

        filters: [
            FilterValue {
                roleName: "name"
                roleValue: root.search
                compareType: CompareType.Contains
            },
            FilterValue {
                roleName: "installed"
                roleValue: root.installed
                compareType: CompareType.Equal
            },
            FilterValue {
                roleName: "category"
                roleValue: root.selectedCategory
                compareType: CompareType.Contains
            }
        ]
    }

    GridView {
        id: view

        anchors.left: parent.left
        anchors.right: parent.right

        readonly property int sideMargin: 28

        anchors.leftMargin: -sideMargin
        anchors.rightMargin: -sideMargin

        height: contentHeight

        header: Item {
            height: titleLabel.height
            anchors.left: parent.left
            anchors.right: parent.right

            StyledTextLabel {
                id: titleLabel

                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.left: parent.left
                anchors.leftMargin: view.sideMargin

                text: root.title
                font: ui.theme.tabBoldFont
            }
        }

        model: filterModel

        clip: true

        cellHeight: 254
        cellWidth: sideMargin + 296 + sideMargin

        boundsBehavior: Flickable.StopAtBounds

        delegate: Item {
            id: item

            height: view.cellHeight
            width: view.cellWidth

            PluginItem {
                anchors.centerIn: parent

                name: model.name
                thumbnailUrl: model.thumbnailUrl
                selected: view.currentIndex == index

                height: 206
                width: 296

                onClicked: {
                    forceActiveFocus()

                    view.positionViewAtIndex(index, GridView.Visible)
                    view.currentIndex = index
                    root.pluginClicked(model)
                }
            }
        }
    }
}
