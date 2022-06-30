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
import MuseScore.NotationScene 1.0

import "internal"

StyledDialogView {
    id: root

    title: qsTrc("notation", "Parts")

    contentWidth: 664
    contentHeight: 558

    modal: true

    PartListModel {
        id: partsModel
    }

    QtObject {
        id: privateProperties

        readonly property int sideMargin: 36
        readonly property int buttonsMargin: 24
    }

    Component.onCompleted: {
        partsModel.load()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        PartsTopPanel {
            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height
            Layout.topMargin: privateProperties.sideMargin

            sideMargin: privateProperties.sideMargin

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 1

            onCreateNewPartRequested: {
                partsModel.createNewPart()
            }
        }

        PartsView {
            id: view
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 38
            Layout.bottomMargin: 24

            model: partsModel

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2
        }

        PartsBottomPanel {
            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height
            Layout.leftMargin: privateProperties.buttonsMargin
            Layout.rightMargin: privateProperties.buttonsMargin
            Layout.bottomMargin: privateProperties.buttonsMargin
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom

            canOpenSelectedParts: partsModel.hasSelection

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 3

            onCloseRequested: {
                root.hide()
            }

            onOpenSelectedPartsRequested: {
                partsModel.openSelectedParts()
                root.hide()
            }

            onOpenAllPartsRequested: {
                partsModel.openAllParts()
                root.hide()
            }
        }
    }
}
