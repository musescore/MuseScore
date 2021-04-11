import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

StyledPopupView {
    id: root

    arrowVisible: false
    positionDisplacementX: 0
    positionDisplacementY: opensUpward ? -view.implicitHeight : parent.height
    padding: 0
    margins: 0
    animationEnabled: false

    height: view.implicitHeight
    width: itemWidth

    property alias model: view.model
    property int itemWidth: 300
    property bool reserveSpaceForInvisibleItems: true
    property bool unifyCheckMarksAndIcons: true

    signal handleAction(string actionCode, int actionIndex)

    property QtObject privateProperties: QtObject {
        property var items: []
    }

    ListView {
        id: view

        implicitHeight: contentHeight
        implicitWidth: root.itemWidth

        spacing: 2
        interactive: false

        delegate: Loader {
            id: loader

            sourceComponent: Boolean(modelData.title) ? menuItemComp : separatorComp

            onLoaded: {
                loader.item.modelData = modelData
                loader.item.width = root.itemWidth
            }

            Component {
                id: menuItemComp

                StyledMenuItem {
                    id: item

                    reserveSpaceForInvisibleItems: root.reserveSpaceForInvisibleItems
                    unifyCheckMarksAndIcons: root.unifyCheckMarksAndIcons

                    onSubMenuShowed: {
                        root.closePolicy = PopupView.NoAutoClose
                    }

                    onSubMenuClosed: {
                        root.closePolicy = PopupView.CloseOnPressOutsideParent
                    }

                    onHandleAction: {
                        // NOTE: reset view state
                        view.update()

                        root.handleAction(actionCode, actionIndex)
                    }
                }
            }

            Component {
                id: separatorComp

                Rectangle {
                    height: 1
                    color: ui.theme.strokeColor

                    property var modelData
                }
            }
        }
    }
}
