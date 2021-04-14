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

    signal handleAction(string actionCode, int actionIndex)

    onModelChanged: {
        privateProperties.hasItemsWithIconAndCheckable = false
        privateProperties.hasItemsWithIconOrCheckable = false
        privateProperties.hasItemsWithSubmenu = false
        privateProperties.hasItemsWithShortcut = false

        for (let i = 0; i < model.length; i++) {
            let modelData = model[i]
            let hasIcon = (Boolean(modelData.icon) && modelData.icon !== IconCode.NONE)

            if (modelData.checkable && hasIcon) {
                privateProperties.hasItemsWithIconAndCheckable = true
                privateProperties.hasItemsWithIconOrCheckable = true
            } else if (modelData.checkable || hasIcon) {
                privateProperties.hasItemsWithIconOrCheckable = true
            }

            if (Boolean(modelData.subitems) && modelData.subitems.length > 0) {
                privateProperties.hasItemsWithSubmenu = true
            }

            if (Boolean(modelData.shortcut)) {
                privateProperties.hasItemsWithShortcut = true
            }
        }
    }

    property QtObject privateProperties: QtObject {
        property bool hasItemsWithIconAndCheckable: false
        property bool hasItemsWithIconOrCheckable: false
        property bool hasItemsWithSubmenu: false
        property bool hasItemsWithShortcut: false
    }

    Item {
        implicitHeight: view.contentHeight
        implicitWidth: root.itemWidth

        ListView {
            id: view

            anchors.fill: parent

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

                        iconAndCheckMarkMode: {
                            if (privateProperties.hasItemsWithIconAndCheckable) {
                                return StyledMenuItem.ShowBoth
                            } else if (privateProperties.hasItemsWithIconOrCheckable) {
                                return StyledMenuItem.ShowOne
                            }
                            return StyledMenuItem.None
                        }

                        reserveSpaceForShortcutOrSubmenuIndicator:
                            privateProperties.hasItemsWithSubmenu || privateProperties.hasItemsWithShortcut

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

        Canvas {
            visible: item.showSubitemsByPressAndHoldRole

            width: 4
            height: 4

            anchors.margins: 2
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            onPaint: {
                const ctx = getContext("2d");
                ctx.fillStyle = ui.theme.fontPrimaryColor;
                ctx.moveTo(width, 0);
                ctx.lineTo(width, height);
                ctx.lineTo(0, height);
                ctx.closePath();
                ctx.fill();
            }
        }
    }
}
