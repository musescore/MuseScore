import QtQuick 2.9

import MuseScore.UiComponents 1.0
import MuseScore.Extensions 1.0

Item {
    id: root

    height: view.height

    property alias model: filterModel.sourceModel

    property alias filters: filterModel.filters

    property int selectedIndex: -1

    property int count: view.count

    signal clicked(int index, var extension)

    FilterProxyModel {
        id: filterModel
    }

    GridView {
        id: view

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        height: contentHeight

        model: filterModel

        clip: true

        cellHeight: 272
        cellWidth: 704

        boundsBehavior: Flickable.StopAtBounds

        delegate: Item {
            id: item

            height: view.cellHeight
            width: view.cellWidth

            ExtensionItem {
                anchors.centerIn: parent

                height: 224
                width: 656

                code: model.code
                name: model.name
                description: model.description
                status: model.status

                selected: selectedIndex === index

                onClicked: {
                    view.positionViewAtIndex(index, GridView.Visible)
                    root.clicked(index, model)
                }
            }
        }
    }
}
