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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

import "internal/Properties"

StyledDialogView {
    id: root

    title: qsTrc("project/properties", "Project properties")

    contentWidth: 680
    contentHeight: 500
    margins: 16

    property int propertyNameWidth: 110
    property NavigationPanel navigationPanel: NavigationPanel {
        name: "ProjectPropertiesPanel"
        section: root.navigationSection
        direction: NavigationPanel.Horizontal
        order: 1
        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }
    }

    ProjectPropertiesModel {
        id: projectPropertiesModel
    }

    Component.onCompleted: {
        projectPropertiesModel.load()
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 8

        ProjectPropertiesView {
            id: propertiesListView

            model: projectPropertiesModel
            propertyNameWidth: root.propertyNameWidth

            navigationPanel: root.navigationPanel
            navigationColumnStart: propertiesFileInfoPanel.navigationColumnEnd + 1
        }

        Connections {
            target: projectPropertiesModel

            function onPropertyAdded(index) {
                propertiesListView.positionViewAtIndex(index, ListView.Contain)
                propertiesListView.currentIndex = index
            }
        }

        SeparatorLine {}

        ProjectPropertiesFileInfoPanel {
            id: propertiesFileInfoPanel

            propertyNameWidth: root.propertyNameWidth
            Layout.topMargin: 4
            Layout.bottomMargin: 8

            navigationPanel: root.navigationPanel
        }

        ButtonBox {
            Layout.fillWidth: true

            buttons: [ ButtonBoxModel.Ok, ButtonBoxModel.Cancel ]

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            FlatButton {
                text: qsTrc("project", "New property")
                buttonRole: ButtonBoxModel.CustomRole
                buttonId: ButtonBoxModel.CustomButton + 1
                isLeftSide: true

                onClicked: {
                    projectPropertiesModel.newProperty()
                }
            }

            onStandardButtonClicked: function(buttonId) {
                if (buttonId === ButtonBoxModel.Ok) {
                    projectPropertiesModel.saveProperties()
                    root.hide()
                } else if (buttonId === ButtonBoxModel.Cancel) {
                    root.hide()
                }
            }
        }
    }
}
