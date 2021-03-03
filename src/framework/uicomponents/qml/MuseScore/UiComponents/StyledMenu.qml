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

    onAboutToShow: {
        view.model = items
    }

    onAboutToHide: {
        items = []
        view.model = []
    }

    function clear() {
        items = []
    }

    function addMenuItem(itemAction) {
        items.push(itemAction)
    }

    signal handleAction(string actionCode)

    property var items: []

    width: content.width
    height: content.height

    Column {
        id: content

        Repeater {
            id: view

            Loader {
                id: loader
                sourceComponent: Boolean(modelData.code) ? menuItemComp : separatorComp
                onLoaded: {
                    loader.item.modelData = modelData
                }

                Component {
                    id: menuItemComp

                    ListItemBlank {
                        id: item

                        property var modelData
                        property bool hasSubMenu: Boolean(modelData) && modelData.subitems.length > 0
                        property var showedSubMenu: undefined

                        defaultColor: ui.theme.accentColor
                        isSelected: showedSubMenu != undefined

                        width: 300

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

                            menu.handleAction.connect(function(actionCode){
                                root.handleAction(actionCode)
                                if (Boolean(root)) {
                                    root.close()
                                }
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
                                iconCode: Boolean(modelData) && Boolean(modelData.icon) ? modelData.icon : IconCode.NONE
                            }

                            StyledTextLabel {
                                Layout.fillWidth: true
                                text: Boolean(modelData) ? modelData.title : ""
                                horizontalAlignment: Text.AlignLeft
                            }

                            StyledTextLabel {
                                Layout.alignment: Qt.AlignRight
                                text: Boolean(modelData) ? modelData.shortcut : ""
                                horizontalAlignment: Text.AlignRight
                            }

                            StyledIconLabel {
                                Layout.alignment: Qt.AlignRight
                                width: 16
                                iconCode: hasSubMenu ? IconCode.SMALL_ARROW_RIGHT : IconCode.NONE
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

                            root.handleAction(modelData.code)
                            if (Boolean(root)) {
                                root.close()
                            }
                        }
                    }
                }

                Component {
                    id: separatorComp

                    SeparatorLine {
                        width: 300
                        property var modelData
                    }
                }
            }
        }
    }
}
