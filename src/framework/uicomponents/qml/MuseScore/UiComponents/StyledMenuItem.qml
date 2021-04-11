import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

ListItemBlank {
    id: root

    hoveredStateColor: ui.theme.accentColor
    pressedStateColor: ui.theme.accentColor
    enabled: Boolean(modelData) && Boolean(modelData.enabled)

    isSelected: Boolean(privateProperties.showedSubMenu) || (privateProperties.hasIcon && privateProperties.isSelectable && privateProperties.isSelected)

    property var modelData

    enum IconAndCheckMarkMode {
        None,
        ShowOne,
        ShowBoth
    }

    property int iconAndCheckMarkMode: StyledMenuItem.ShowOne
    property bool reserveSpaceForShortcutOrSubmenuIndicator: privateProperties.hasShortcut || privateProperties.hasSubMenu

    signal subMenuShowed()
    signal subMenuClosed()

    signal handleAction(string actionCode, int actionIndex)

    QtObject {
        id: privateProperties

        property bool hasShortcut: Boolean(modelData) && Boolean(modelData.shortcut)

        property bool hasSubMenu: Boolean(modelData) && Boolean(modelData.subitems) && modelData.subitems.length > 0
        property var showedSubMenu: undefined

        property bool isCheckable: Boolean(modelData) && Boolean(modelData.checkable)
        property bool isChecked: isCheckable && Boolean(modelData.checked)

        property bool isSelectable: Boolean(modelData) && Boolean(modelData.selectable)
        property bool isSelected: isSelectable && Boolean(modelData.selected)

        property bool hasIcon: Boolean(modelData) && Boolean(modelData.icon)

        function showSubMenu() {
            if (privateProperties.showedSubMenu) {
                return
            }

            var menuComponent = Qt.createComponent("StyledMenu.qml");
            var menu = menuComponent.createObject(root)
            menu.positionDisplacementX = root.width
            menu.positionDisplacementY = 0

            menu.model = modelData.subitems

            menu.handleAction.connect(function(actionCode, actionIndex) {
                Qt.callLater(root.handleAction, actionCode, actionIndex)
                menu.close()
            })

            menu.closed.connect(function() {
                privateProperties.showedSubMenu = undefined
                menu.destroy()
                subMenuClosed()
            })

            subMenuShowed()

            privateProperties.showedSubMenu = menu
            menu.toggleOpened()
        }
    }

    RowLayout {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: root.iconAndCheckMarkMode === StyledMenuItem.None ? 24 : 12
        anchors.right: parent.right
        anchors.rightMargin: 18

        spacing: 12

        StyledIconLabel {
            Layout.alignment: Qt.AlignLeft
            width: 16
            iconCode: {
                if (root.iconAndCheckMarkMode !== StyledMenuItem.ShowBoth && privateProperties.hasIcon) {
                    return privateProperties.hasIcon ? modelData.icon : IconCode.NONE
                } else if (privateProperties.isCheckable) {
                    return privateProperties.isChecked ? IconCode.TICK_RIGHT_ANGLE : IconCode.NONE
                } else  if (privateProperties.isSelectable) {
                    return privateProperties.isSelected ? IconCode.TICK_RIGHT_ANGLE : IconCode.NONE
                }

                return IconCode.NONE
            }
            visible: !isEmpty || root.iconAndCheckMarkMode !== StyledMenuItem.None
        }

        StyledIconLabel {
            Layout.alignment: Qt.AlignLeft
            width: 16
            iconCode: privateProperties.hasIcon ? modelData.icon : IconCode.NONE
            visible: root.iconAndCheckMarkMode === StyledMenuItem.ShowBoth
        }

        StyledTextLabel {
            Layout.fillWidth: true
            text: Boolean(modelData) && Boolean(modelData.title) ? modelData.title : ""
            horizontalAlignment: Text.AlignLeft
        }

        StyledTextLabel {
            id: shortcutLabel
            Layout.alignment: Qt.AlignRight
            text: privateProperties.hasShortcut ? modelData.shortcut : ""
            horizontalAlignment: Text.AlignRight
            visible: !isEmpty || (root.reserveSpaceForShortcutOrSubmenuIndicator)
        }

        StyledIconLabel {
            id: submenuIndicator
            Layout.alignment: Qt.AlignRight
            width: 16
            iconCode: privateProperties.hasSubMenu ? IconCode.SMALL_ARROW_RIGHT : IconCode.NONE
            visible: !isEmpty || (root.reserveSpaceForShortcutOrSubmenuIndicator && !shortcutLabel.visible)
        }
    }

    onHovered: {
        if (!privateProperties.hasSubMenu) {
            return
        }

        if (isHovered) {
            privateProperties.showSubMenu()
        } else {
            var mouseOnShowedSubMenu = mapToItem(privateProperties.showedSubMenu, mouseX, mouseY)
            var eps = 4
            var subMenuWidth = privateProperties.showedSubMenu.x + privateProperties.showedSubMenu.width
            var subMenuHeight = privateProperties.showedSubMenu.y + privateProperties.showedSubMenu.height
            var isHoveredOnShowedSubMenu = (0 < mouseOnShowedSubMenu.x + eps &&
                                            mouseOnShowedSubMenu.x - eps < subMenuWidth) &&
                                           (0 < mouseOnShowedSubMenu.y + eps &&
                                            mouseOnShowedSubMenu.y - eps < subMenuHeight)

            if (isHoveredOnShowedSubMenu) {
                return
            }

            privateProperties.showedSubMenu.close()
        }
    }

    onClicked: {
        if (privateProperties.hasSubMenu) {
            privateProperties.showSubMenu()
            return
        }

        root.handleAction(modelData.code, privateProperties.isSelectable ? index : -1)
    }
}
