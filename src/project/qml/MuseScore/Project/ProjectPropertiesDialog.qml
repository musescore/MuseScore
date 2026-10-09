/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Project

import "internal/Properties"

StyledDialogView {
    id: root

    title: qsTrc("project/properties", "Project properties")

    contentWidth: 680
    contentHeight: 500
    margins: 16

    readonly property int propertyNameWidth: 160
    readonly property int propertyRowHorizontalSpacing: 8
    readonly property int propertyRowRightMargin: propertiesListView.propertyRowRightMargin

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

    //! The language of the score, see ProjectPropertiesModel::instrumentNamesLanguages()
    property var instrumentNamesLanguages: []
    property string instrumentNamesLanguage: ""

    NavigationPanel {
        id: instrumentNamesLanguageNavPanel

        name: "InstrumentNamesLanguagePanel"
        section: root.navigationSection
        order: 3
    }

    Component.onCompleted: {
        projectPropertiesModel.load()

        instrumentNamesLanguages = projectPropertiesModel.instrumentNamesLanguages()
        instrumentNamesLanguage = projectPropertiesModel.instrumentNamesLanguage()
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 8

        ProjectPropertiesView {
            id: propertiesListView

            propertiesModel: projectPropertiesModel

            Layout.fillHeight: true
            Layout.fillWidth: true

            propertyNameWidth: root.propertyNameWidth
            propertyRowHorizontalSpacing: root.propertyRowHorizontalSpacing

            navigationPanel: root.navigationPanel
            navigationColumnStart: propertiesFileInfoPanel.navigationColumnEnd + 1
        }

        SeparatorLine {}

        RowLayout {
            Layout.fillWidth: true
            Layout.rightMargin: root.propertyRowRightMargin

            spacing: root.propertyRowHorizontalSpacing

            StyledTextLabel {
                Layout.preferredWidth: root.propertyNameWidth

                text: qsTrc("project/properties", "Language")
                horizontalAlignment: Text.AlignLeft
            }

            StyledDropdown {
                id: instrumentNamesLanguageDropdown

                Layout.fillWidth: true

                model: root.instrumentNamesLanguages
                textRole: "name"
                valueRole: "code"

                currentIndex: indexOfValue(root.instrumentNamesLanguage)

                navigation.name: "InstrumentNamesLanguageDropdown"
                navigation.panel: instrumentNamesLanguageNavPanel
                navigation.row: 0
                navigation.accessible.name: qsTrc("project/properties", "Language of the score") + ": " + currentText

                onActivated: function(index, value) {
                    root.instrumentNamesLanguage = value
                    projectPropertiesModel.setInstrumentNamesLanguage(value)
                }
            }
        }

        SeparatorLine {}

        ProjectPropertiesFileInfoPanel {
            id: propertiesFileInfoPanel

            propertiesModel: projectPropertiesModel

            Layout.fillWidth: true
            Layout.topMargin: 4
            Layout.rightMargin: root.propertyRowRightMargin
            Layout.bottomMargin: 8

            propertyNameWidth: root.propertyNameWidth
            propertyRowHorizontalSpacing: root.propertyRowHorizontalSpacing
            propertyRowRightMargin: root.propertyRowRightMargin

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
