/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
import QtQuick.Layouts 1.3

import Muse.UiComponents 1.0
import Muse.Ui 1.0
import MuseScore.Project 1.0

Item {

    id: root

    property alias selectedTemplatePath: model.currentTemplatePath
    property bool hasSelectedTemplate: selectedTemplatePath !== ""

    property NavigationSection navigationSection: null

    property alias navigation: templatesView.navigationPanel

    signal done

    TemplatesModel {
        id: model

        onCurrentTemplateChanged: {
            templatePreview.load(model.currentTemplatePath)
        }
    }

    Component.onCompleted: {
        model.load()
    }

    RowLayout {
        id: row

        anchors.fill: parent

        spacing: 12

        TitleListView {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width / 4

            navigationPanel.section: root.navigationSection
            navigationPanel.name: "Category"
            navigationPanel.order: 2

            listTitle: qsTrc("project/newscore", "Category")
            model: model.categoriesTitles
            currentIndex: model.currentCategoryIndex
            searchEnabled: false

            onTitleClicked: function(index) {
                model.currentCategoryIndex = index

                if (templatesView.searching) {
                    model.saveCurrentCategory()
                    templatesView.clearSearch()
                }
            }
        }

        SeparatorLine { orientation: Qt.Vertical }

        TitleListView {
            id: templatesView

            Layout.fillHeight: true
            Layout.preferredWidth: parent.width / 4

            navigationPanel.section: root.navigationSection
            navigationPanel.name: "Template"
            navigationPanel.order: 3

            listTitle: qsTrc("project/newscore", "Template")
            model: model.templatesTitles
            currentIndex: model.currentTemplateIndex

            onTitleClicked: function(index) {
                model.currentTemplateIndex = index
            }

            onDoubleClicked: function(index) {
                model.currentTemplateIndex = index
                root.done()
            }

            onSearchTextChanged: {
                model.setSearchText(searchText)
            }
        }

        SeparatorLine { orientation: Qt.Vertical; Layout.rightMargin: 30 + row.spacing }

        SeparatorLine { orientation: Qt.Vertical }

        TemplatePreview {
            id: templatePreview

            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        SeparatorLine { orientation: Qt.Vertical }

        Column {
            Layout.preferredWidth: 30
            Layout.alignment: Qt.AlignVCenter

            spacing: 12

            NavigationPanel {
                id: navZoomPanel
                name: "ZoomPanel"
                direction: NavigationPanel.Vertical
                enabled: root.enabled && root.visible
                section: root.navigationSection
                order: 4
            }

            FlatButton {
                icon: IconCode.ZOOM_IN

                navigation.name: "ZOOM_IN"
                navigation.panel: navZoomPanel
                navigation.row: 1
                navigation.accessible.name: qsTrc("project/newscore", "Zoom in")

                onClicked: {
                    templatePreview.zoomIn()
                }
            }

            FlatButton {
                icon: IconCode.ZOOM_OUT

                navigation.name: "ZOOM_OUT"
                navigation.panel: navZoomPanel
                navigation.row: 2
                navigation.accessible.name: qsTrc("project/newscore", "Zoom out")

                onClicked: {
                    templatePreview.zoomOut()
                }
            }
        }
    }
}
