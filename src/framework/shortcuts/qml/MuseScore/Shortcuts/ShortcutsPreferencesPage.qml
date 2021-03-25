import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Shortcuts 1.0

Item {
    id: root

    ShortcutsModel {
        id: shortcutsModel
    }

    Component.onCompleted: {
        shortcutsModel.load()
    }

    QtObject {
        id: privateProperties

        readonly property int buttonWidth: 105
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 20

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height

            FlatButton {
                Layout.preferredWidth: privateProperties.buttonWidth

                text: qsTrc("shortcuts", "Define...")
            }

            FlatButton {
                Layout.preferredWidth: privateProperties.buttonWidth

                text: qsTrc("shortcuts", "Clear")
            }

            Item { Layout.fillWidth: true }

            SearchField {
                id: searchField

                Layout.preferredWidth: 160

                hint: qsTrc("shortcuts", "Search shortcut")
            }
        }

        ValueList {
            id: view

            Layout.fillWidth: true
            Layout.fillHeight: true

            keyRoleName: "title"
            keyTitle: qsTrc("shortcuts", "ACTION")
            valueRoleName: "sequence"
            valueTitle: qsTrc("shortcuts", "SHORTCUT")
            readOnly: true

            model: SortFilterProxyModel {
                sourceModel: shortcutsModel

                filters: [
                    FilterValue {
                        roleName: "searchKey"
                        roleValue: searchField.searchText
                        compareType: CompareType.Contains
                    }
                ]
            }

            onClicked: {
                shortcutsModel.editShortcut(index)
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height

            FlatButton {
                Layout.preferredWidth: privateProperties.buttonWidth

                text: qsTrc("shortcuts", "Load...")

                onClicked: {
                    shortcutsModel.selectShortcutsFile()
                }
            }

            FlatButton {
                Layout.preferredWidth: privateProperties.buttonWidth

                text: qsTrc("shortcuts", "Save")
            }

            Item { Layout.fillWidth: true }

            FlatButton {
                Layout.preferredWidth: privateProperties.buttonWidth

                text: qsTrc("shortcuts", "Print")
            }

            FlatButton {
                Layout.preferredWidth: privateProperties.buttonWidth

                text: qsTrc("shortcuts", "Reset to default")
            }
        }
    }
}
