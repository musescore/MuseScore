/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Item {
    id: root

    property var model: null

    property NavigationSection navigationPanelSection: null
    property int navigationPanelOrderStart: 0
    property int navigationPanelOrderEnd: playingTechniquesGridView.navigationPanel.order

    property bool noOptions: !modifySoundView.hasPresets && !playingTechniquesGridView.hasPlayingTechniques

    height: !noOptions ? content.childrenRect.height : noOptionsLabel.implicitHeight

    signal closeRequested()

    Column {
        id: content

        width: parent.width

        spacing: 12

        ParamsGridView {
            id: modifySoundView

            property bool hasPresets: Boolean(model) && model.length !== 0

            width: parent.width

            title: qsTrc("playback", "Modify sound")
            model: root.model.availablePresets
            selectionModel: root.model.selectedPresetCodes

            navigationPanel.section: root.navigationPanelSection
            navigationPanel.order: root.navigationPanelOrderStart

            visible: hasPresets

            needAddPaddingForScrollbar: modifySoundView.isTruncated || playingTechniquesGridView.isTruncated

            onToggleParamRequested: {
                root.model.togglePreset(paramCode)
            }

            onCloseRequested: {
                root.closeRequested()
            }
        }

        ParamsGridView {
            id: playingTechniquesGridView

            property bool hasPlayingTechniques: Boolean(model) && model.length !== 0

            width: parent.width

            title: qsTrc("playback", "Playing techniques")
            model: root.model.availablePlayingTechniques
            selectionModel: [ root.model.selectedPlayingTechniqueCode ]

            navigationPanel.section: root.navigationPanelSection
            navigationPanel.order: root.navigationPanelOrderStart + 1

            visible: hasPlayingTechniques

            needAddPaddingForScrollbar: modifySoundView.isTruncated || playingTechniquesGridView.isTruncated

            onToggleParamRequested: {
                root.model.togglePlayingTechnique(paramCode)
            }

            onCloseRequested: {
                root.closeRequested()
            }
        }
    }

    StyledTextLabel {
        id: noOptionsLabel

        text: qsTrc("playback", "Sound flag options are not available for this sound.")
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.Wrap

        visible: root.noOptions
    }
}
