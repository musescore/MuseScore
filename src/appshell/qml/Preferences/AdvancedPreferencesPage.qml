/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
