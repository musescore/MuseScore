/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.9

import MuseScore.UiComponents 1.0
import MuseScore.Extensions 1.0

Item {
    id: root

    height: view.height

    property string title: ""

    property alias model: filterModel.sourceModel

    property alias filters: filterModel.filters

    property string selectedExtensionCode: ""

    property int count: view.count

    signal clicked(int index, var extension)

    SortFilterProxyModel {
        id: filterModel
    }

    GridView {
        id: view

        readonly property int sideMargin: 24

        anchors.left: parent.left
        anchors.leftMargin: -sideMargin
        anchors.right: parent.right
        anchors.rightMargin: -sideMargin

        height: contentHeight

        model: filterModel

        clip: true

        cellHeight: 272
        cellWidth: 650

        boundsBehavior: Flickable.StopAtBounds

        header: Item {
            height: titleLabel.height
            anchors.left: parent.left
            anchors.right: parent.right

            StyledTextLabel {
                id: titleLabel

                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.left: parent.left
                anchors.leftMargin: view.sideMargin

                text: root.title
                font: ui.theme.tabBoldFont
            }
        }

        delegate: Item {
            id: item

            height: view.cellHeight
            width: view.cellWidth

            ExtensionItem {
                anchors.centerIn: parent

                height: 224
                width: 602

                code: model.code
                name: model.name
                description: model.description
                status: model.status

                selected: selectedExtensionCode === model.code

                onClicked: {
                    forceActiveFocus()

                    view.positionViewAtIndex(index, GridView.Visible)
                    root.clicked(index, model)
                }
            }
        }
    }
}
