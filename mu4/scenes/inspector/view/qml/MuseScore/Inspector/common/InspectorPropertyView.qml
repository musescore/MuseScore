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

            ContextMenu {
                id: menu

                Layout.alignment: Qt.AlignRight
                Layout.fillWidth: true
                Layout.maximumWidth: 24
                Layout.preferredHeight: 24
                Layout.preferredWidth: 24

                Action {
                    id: resetToDefaultAction

                    checkable: false
                    enabled: root.isModified

                    onTriggered: {
                        menu.forceClose()

                        if (propertyItem) {
                            propertyItem.resetToDefault()
                        }
                    }
                }

                Action {
                    id: applyToStyleAction

                    checkable: true
                    checked: !root.isModified
                    enabled: root.isModified

                    onTriggered: {
                        menu.forceClose()

                        if (propertyItem) {
                            propertyItem.applyToStyle()
                        }
                    }
                }

                function updateMenuModel() {
                    menu.clearMenuItems()

                    if (root.isStyled) {
                        menu.addMenuItem(qsTr("Reset to style default"), resetToDefaultAction)
                        menu.addMenuItem(qsTr("Set as style"), applyToStyleAction)
                    } else {
                        menu.addMenuItem( qsTr("Reset to default"), resetToDefaultAction)
                    }
                }
            }
        }

        MouseArea {
            anchors.fill: contentRow
            hoverEnabled: true
            propagateComposedEvents: true

            onContainsMouseChanged: {
                menu.hovered = containsMouse
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
