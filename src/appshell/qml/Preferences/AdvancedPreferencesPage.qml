import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

PreferencesPage {
    id: root

    Component.onCompleted: {
        preferencesModel.load()
    }

    AdvancedPreferencesModel {
        id: preferencesModel
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 20

        Item {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.preferredHeight: 30

            FlatButton {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: qsTrc("appshell", "Reset to default")

                onClicked: {
                    preferencesModel.resetToDefault()
                }
            }

            SearchField {
                id: searchField
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: 160

                hint: qsTrc("appshell", "Search advanced")
            }
        }

        ValueList {
            Layout.fillWidth: true
            Layout.fillHeight: true

            keyRoleName: "keyRole"
            keyTitle: qsTrc("appshell", "Preference")
            valueRoleName: "valueRole"
            valueTitle: qsTrc("appshell", "Value")
            valueTypeRole: "typeRole"

            model: SortFilterProxyModel {
                sourceModel: preferencesModel

                filters: [
                    FilterValue {
                        roleName: "keyRole"
                        roleValue: searchField.searchText
                        compareType: CompareType.Contains
                    }
                ]
            }
        }
    }
}
