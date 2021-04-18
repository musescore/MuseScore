import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

StyledPopupView {
    id: root

    property alias model: view.model
    property int itemWidth: 300

    signal handleAction(string actionCode, int actionIndex)

    arrowVisible: false
    positionDisplacementX: 0
    positionDisplacementY: opensUpward ? -view.implicitHeight : parent.height
    padding: 0
    margins: 0
    animationEnabled: false

    height: view.implicitHeight
    width: itemWidth

    keynav.name: "StyledMenu"
    keynav.direction: KeyNavigationSubSection.Vertical

    onModelChanged: {
        prv.hasItemsWithIconAndCheckable = false
        prv.hasItemsWithIconOrCheckable = false
        prv.hasItemsWithSubmenu = false
        prv.hasItemsWithShortcut = false

        for (let i = 0; i < model.length; i++) {
            let modelData = model[i]
            let hasIcon = (Boolean(modelData.icon) && modelData.icon !== IconCode.NONE)

            if (modelData.checkable && hasIcon) {
                prv.hasItemsWithIconAndCheckable = true
                prv.hasItemsWithIconOrCheckable = true
            } else if (modelData.checkable || hasIcon) {
                prv.hasItemsWithIconOrCheckable = true
            }

            if (Boolean(modelData.subitems) && modelData.subitems.length > 0) {
                prv.hasItemsWithSubmenu = true
            }

            if (Boolean(modelData.shortcut)) {
                prv.hasItemsWithShortcut = true
            }
        }
    }

    property QtObject prv_prop: QtObject {
        id: prv
        property bool hasItemsWithIconAndCheckable: false
        property bool hasItemsWithIconOrCheckable: false
        property bool hasItemsWithSubmenu: false
        property bool hasItemsWithShortcut: false
    }

    function focusOnFirstItem() {
        var loader = view.itemAtIndex(0)
        if (loader && loader.item) {
            loader.item.keynav.forceActive()
        }
    }

    function focusOnSelected() {
        for (var i = 0; i < view.count; ++i) {
            var loader = view.itemAtIndex(i)
            if (loader && loader.item && loader.item.isSelected) {
                loader.item.keynav.forceActive()
                return true
            }
        }
        return false
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

                    keynav.subsection: root.keynav
                    keynav.column: 0
                    keynav.row: model.index

                    iconAndCheckMarkMode: {
                        if (prv.hasItemsWithIconAndCheckable) {
                            return StyledMenuItem.ShowBoth
                        } else if (prv.hasItemsWithIconOrCheckable) {
                            return StyledMenuItem.ShowOne
                        }
                        return StyledMenuItem.None
                    }

                    reserveSpaceForShortcutOrSubmenuIndicator:
                        prv.hasItemsWithSubmenu || prv.hasItemsWithShortcut

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
