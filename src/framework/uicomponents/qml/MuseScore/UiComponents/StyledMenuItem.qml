import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

ListItemBlank {
    id: root

    defaultColor: ui.theme.accentColor
    enabled: Boolean(modelData) && Boolean(modelData.enabled)

    isSelected: privateProperties.showedSubMenu != undefined || (privateProperties.hasIcon && privateProperties.isSelectable && privateProperties.isSelected)

    property var modelData
    property bool reserveSpaceForInvisibleItems: true

    signal subMenuShowed()
    signal subMenuClosed()

    signal handleAction(string actionCode, int actionIndex)

    QtObject {
        id: privateProperties

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

            menu.handleAction.connect(function(actionCode, actionIndex){
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
        anchors.leftMargin: 12
        anchors.right: parent.right
        anchors.rightMargin: 12

        spacing: 12

        StyledIconLabel {
            Layout.alignment: Qt.AlignLeft
            width: 16
            iconCode: {
                if (privateProperties.isCheckable) {
                    return privateProperties.isChecked ? IconCode.TICK_RIGHT_ANGLE : IconCode.NONE
                } else if (privateProperties.hasIcon) {
                    return privateProperties.hasIcon ? modelData.icon : IconCode.NONE
                } else if (privateProperties.isSelectable) {
                    return privateProperties.isSelected ? IconCode.TICK_RIGHT_ANGLE : IconCode.NONE
                }

                return IconCode.NONE
            }

            visible: !isEmpty || reserveSpaceForInvisibleItems
        }

        StyledTextLabel {
            Layout.fillWidth: true
            text: Boolean(modelData) && Boolean(modelData.title) ? modelData.title : ""
            horizontalAlignment: Text.AlignLeft

            visible: Boolean(text) || reserveSpaceForInvisibleItems
        }

        StyledTextLabel {
            Layout.alignment: Qt.AlignRight
            text: Boolean(modelData) && Boolean(modelData.shortcut) ? modelData.shortcut : ""
            horizontalAlignment: Text.AlignRight

            visible: Boolean(text) || reserveSpaceForInvisibleItems
        }

        StyledIconLabel {
            Layout.alignment: Qt.AlignRight
            width: 16
            iconCode: privateProperties.hasSubMenu ? IconCode.SMALL_ARROW_RIGHT : IconCode.NONE

            visible: !isEmpty || reserveSpaceForInvisibleItems
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
