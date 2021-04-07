import QtQuick 2.12
import QtQuick.Controls 2.12

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Rectangle {
    id: root

    property alias orientation: gridView.orientation

    property alias keynav: keynavSub

    QtObject {
        id: privatesProperties

        property bool isHorizontal: orientation === Qt.Horizontal
    }

    KeyNavigationSubSection {
        id: keynavSub
        name: "NotationToolBar"
    }

    NoteInputBarModel {
        id: noteInputModel
    }

    Component.onCompleted: {
        noteInputModel.load()
    }

    GridViewSectional {
        id: gridView
        anchors.fill: parent

        sectionRole: "sectionRole"

        cellWidth: 36
        cellHeight: cellWidth

        model: noteInputModel

        sectionDelegate: SeparatorLine {
            orientation: gridView.orientation === Qt.Vertical ? Qt.Horizontal : Qt.Vertical
            visible: itemIndex !== 0
        }

        itemDelegate: FlatButton {
            property var item: Boolean(itemModel) ? itemModel : null
            property var hasSubitems: Boolean(item) && item.subitemsRole.length !== 0

            normalStateColor: Boolean(item) && item.checkedRole ? ui.theme.accentColor : "transparent"

            icon: Boolean(item) ? item.iconRole : IconCode.NONE
            hint: Boolean(item) ? item.hintRole : ""

            iconFont: ui.theme.toolbarIconsFont

            keynav.subsection: keynavSub
            keynav.order: Boolean(item) ? item.orderRole : 0

            pressAndHoldInterval: 200

            width: gridView.cellWidth
            height: gridView.cellWidth

            onClicked: {
                if (menuLoader.isMenuOpened() || (hasSubitems && !item.showSubitemsByPressAndHoldRole)) {
                    menuLoader.toggleOpened(item.subitemsRole)
                    return
                }

                Qt.callLater(noteInputModel.handleAction, item.codeRole)
            }

            onPressAndHold: {
                if (menuLoader.isMenuOpened() || !hasSubitems) {
                    return
                }

                menuLoader.toggleOpened(item.subitemsRole)
            }

            Loader {
                id: menuLoader
                anchors.fill: parent

                property var menu: menuLoader.item

                function isMenuOpened() {
                    return menuLoader.menu && menuLoader.menu.isOpened
                }

                function toggleOpened(items) {
                    if (!menuLoader.sourceComponent) {
                        menuLoader.sourceComponent = itemMenuComp
                    }

                    if (menuLoader.menu.isOpened) {
                        menuLoader.menu.close()
                        return
                    }

                    menuLoader.menu.model = items
                    menuLoader.menu.open()
                }
            }
        }
    }

    Component {
        id: itemMenuComp

        StyledMenu {
            id: itemMenu

            onHandleAction: {
                Qt.callLater(noteInputModel.handleAction, actionCode, actionIndex)
                itemMenu.close()
            }
        }
    }

    FlatButton {
        id: customizeButton

        anchors.margins: 8

        icon: IconCode.CONFIGURE
        normalStateColor: "transparent"
        keynav.subsection: keynavSub
        keynav.order: 100

        onClicked: {
            api.launcher.open("musescore://notation/noteinputbar/customise")
        }
    }

    states: [
        State {
            when: privatesProperties.isHorizontal
            PropertyChanges {
                target: gridView
                sectionWidth: 1
                sectionHeight: root.height
                rows: 1
                columns: gridView.noLimit
            }

            AnchorChanges {
                target: customizeButton
                anchors.right: root.right
                anchors.verticalCenter: root.verticalCenter
            }
        },
        State {
            when: !privatesProperties.isHorizontal
            PropertyChanges {
                target: gridView
                sectionWidth: root.width
                sectionHeight: 1
                rows: gridView.noLimit
                columns: 2
            }

            AnchorChanges {
                target: customizeButton
                anchors.bottom: root.bottom
                anchors.right: root.right
            }
        }
    ]
}
