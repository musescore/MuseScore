import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

StyledPopupView {
    id: root

    arrowVisible: false
    positionDisplacementX: 0
    padding: 0
    margins: 0

    property bool reserveSpaceForInvisibleItems: true

    property int itemWidth: 300

    width: content.width
    height: content.height

    onAboutToShow: {
        view.model = privateProperties.items
    }

    onAboutToHide: {
        privateProperties.items = []
        view.model = []
    }

    function clear() {
        privateProperties.items = []
    }

    function addMenuItem(itemAction) {
        privateProperties.items.push(itemAction)
    }

    signal handleAction(string actionCode, int actionIndex)

    property QtObject privateProperties: QtObject {
        property var items: []
    }

    Column {
        id: content

        spacing: 2
        Repeater {
            id: view

            Loader {
                id: loader
                sourceComponent: Boolean(modelData.title) ? menuItemComp : separatorComp
                onLoaded: {
                    loader.item.modelData = modelData
                    loader.item.width = root.itemWidth
                }

                Component {
                    id: menuItemComp

                    ListItemBlank {
                        id: item

                        property var modelData
                        property bool hasSubMenu: Boolean(modelData) && Boolean(modelData.subitems) && modelData.subitems.length > 0
                        property var showedSubMenu: undefined

                        property bool isCheckable: Boolean(modelData) && Boolean(modelData.checkable)
                        property bool isChecked: isCheckable && Boolean(modelData.checked)

                        property bool isSelectable: Boolean(modelData) && Boolean(modelData.selectable)
                        isSelected: showedSubMenu != undefined || (isSelectable && Boolean(modelData.selected))

                        defaultColor: ui.theme.accentColor
                        enabled: Boolean(modelData) && Boolean(modelData.enabled)

                        function showSubMenu() {
                            if (showedSubMenu) {
                                return
                            }

                            var menuComponent = Qt.createComponent("StyledMenu.qml");
                            var menu = menuComponent.createObject(item)
                            menu.positionDisplacementX = item.width
                            menu.positionDisplacementY = 0

                            for (var i in modelData.subitems) {
                                menu.addMenuItem(modelData.subitems[i])
                            }

                            menu.handleAction.connect(function(actionCode, actionIndex){
                                Qt.callLater(root.handleAction, actionCode, actionIndex)
                                menu.close()
                            })

                            menu.closed.connect(function() {
                                showedSubMenu = undefined
                                menu.destroy()
                                root.closePolicy = PopupView.CloseOnPressOutsideParent
                            })

                            root.closePolicy = PopupView.NoAutoClose

                            showedSubMenu = menu
                            menu.toggleOpened()
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
                                    if (isCheckable) {
                                        return isChecked ? IconCode.TICK_RIGHT_ANGLE : IconCode.NONE
                                    } else {
                                        return Boolean(modelData) && Boolean(modelData.icon) ? modelData.icon : IconCode.NONE
                                    }
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
                                iconCode: hasSubMenu ? IconCode.SMALL_ARROW_RIGHT : IconCode.NONE

                                visible: !isEmpty || reserveSpaceForInvisibleItems
                            }
                        }

                        onHovered: {
                            if (!hasSubMenu) {
                                return
                            }

                            if (isHovered) {
                                showSubMenu()
                            } else {
                                var mouseOnShowedSubMenu = mapToItem(showedSubMenu, mouseX, mouseY)
                                var eps = 4
                                var isHoveredOnShowedSubMenu = (0 < mouseOnShowedSubMenu.x + eps && mouseOnShowedSubMenu.x - eps < showedSubMenu.x + showedSubMenu.width) &&
                                                               (0 < mouseOnShowedSubMenu.y + eps && mouseOnShowedSubMenu.y - eps < showedSubMenu.y + showedSubMenu.height)

                                if (isHoveredOnShowedSubMenu) {
                                    return
                                }

                                showedSubMenu.close()
                            }
                        }

                        onClicked: {
                            if (hasSubMenu) {
                                showSubMenu()
                                return
                            }

                            root.handleAction(modelData.code, isSelectable ? index : -1)
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
}
