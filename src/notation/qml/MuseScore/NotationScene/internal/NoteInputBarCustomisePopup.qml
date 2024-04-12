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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

StyledPopupView {
    id: root

    contentWidth: 280
    contentHeight: 600

    NoteInputBarCustomiseModel {
        id: customiseModel
    }

    onOpened: {
        customiseModel.load()
        view.focusOnFirst()
    }

    onClosed: {
        view.clearFocus()
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        StyledTextLabel {
            id: titleLabel
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            Layout.topMargin: 8

            text: qsTrc("notation", "Customize toolbar")
            horizontalAlignment: Text.AlignLeft
            font: ui.theme.largeBodyBoldFont
        }

        CustomiseControlPanel {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: 20

            isAddSeparatorAvailable: customiseModel.isAddSeparatorAvailable
            isRemovingAvailable: customiseModel.isRemovingAvailable
            isMovingUpAvailable: customiseModel.isMovingUpAvailable
            isMovingDownAvailable: customiseModel.isMovingDownAvailable

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 1

            onAddSeparatorLineRequested: {
                customiseModel.addSeparatorLine()
            }

            onRemoveSelectionRequested: {
                customiseModel.removeSelection()
            }

            onMoveSelectionUpRequested: {
                customiseModel.moveSelectionUp()
                Qt.callLater(view.positionViewAtSelectedItems)
            }

            onMoveSelectionDownRequested: {
                customiseModel.moveSelectionDown()
                Qt.callLater(view.positionViewAtSelectedItems)
            }
        }

        CustomiseView {
            id: view
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.topMargin: 12

            model: customiseModel

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2
            navigationPanel.accessible.name: titleLabel.text

            onSelectRowRequested: function(index) {
                customiseModel.selectRow(index)
            }

            onClearSelectionRequested: {
                customiseModel.clearSelection()
            }

            onRemoveSelectionRequested: {
                customiseModel.removeSelection()
            }
        }
    }
}
