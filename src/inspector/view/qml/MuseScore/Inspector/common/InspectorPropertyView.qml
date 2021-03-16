import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls 2.2

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Column {
    id: root

    property QtObject propertyItem: null

    property alias titleText: titleLabel.text
    property bool isStyled: propertyItem ? propertyItem.isStyled : false
    property bool isModified: propertyItem ? propertyItem.isModified : false

    width: parent.width

    spacing: 8

    Item {
        height: contentRow.implicitHeight
        width: parent.width

        RowLayout {
            id: contentRow

            width: parent.width

            spacing: 4

            StyledTextLabel {
                id: titleLabel

                Layout.alignment: Qt.AlignLeft
                Layout.fillWidth: true

                horizontalAlignment: Text.AlignLeft
            }

            FlatButton {
                id: menuButton

                icon: IconCode.MENU_THREE_DOTS
                normalStateColor: "transparent"

                onClicked: {
                    menu.open()
                }
            }
        }

        ContextMenu {
            id: menu

            width: parent.width

            y: menuButton.y + menuButton.height

            StyledContextMenuItem {
                id: resetToDefaultItem

                text: root.isStyled ? qsTrc("inspector", "Reset to style default") : qsTrc("inspector", "Reset to default")
                checkable: false
                enabled: root.isModified

                onTriggered: {
                    if (propertyItem) {
                        propertyItem.resetToDefault()
                    }
                }
            }

            StyledContextMenuItem {
                id: applyToStyleItem

                text: qsTrc("inspector", "Set as style")
                checkable: true
                checked: !root.isModified
                enabled: root.isModified

                onTriggered: {
                    if (propertyItem) {
                        propertyItem.applyToStyle()
                    }
                }
            }

            function updateMenuModel() {
                menu.clear()

                menu.addItem(resetToDefaultItem)

                if (root.isStyled) {
                    menu.addItem(applyToStyleItem)
                }
            }
        }
    }

    onPropertyItemChanged: {
        if (propertyItem) {
            menu.updateMenuModel()
        }
    }

    onIsStyledChanged: {
        if (propertyItem) {
            menu.updateMenuModel()
        }
    }
}
