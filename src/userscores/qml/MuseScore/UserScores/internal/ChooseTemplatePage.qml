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
import QtQuick 2.7
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.UserScores 1.0

Item {
    property alias selectedTemplatePath: model.currentTemplatePath
    property bool hasSelectedTemplate: selectedTemplatePath !== ""

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

            listTitle: qsTrc("userscores", "Category")
            model: model.categoriesTitles
            searchEnabled: false

            onTitleClicked: {
                model.setCurrentCategory(index)
            }
        }

        SeparatorLine { orientation: Qt.Vertical }

        TitleListView {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width / 4

            listTitle: qsTrc("userscores", "Template")
            model: model.templatesTitles

            onTitleClicked: {
                model.setCurrentTemplate(index)
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

            FlatButton {
                icon: IconCode.ZOOM_IN

                onClicked: {
                    templatePreview.zoomIn()
                }
            }

            FlatButton {
                icon: IconCode.ZOOM_OUT

                onClicked: {
                    templatePreview.zoomOut()
                }
            }
        }
    }
}
