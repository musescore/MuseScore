/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

import MuseScore.Playback 1.0

import "internal/SoundFlag"

StyledPopupView {
    id: root

    property NavigationSection notationViewNavigationSection: null
    property int navigationOrderStart: 0
    property int navigationOrderEnd: museSoundsParams.navigationPanelOrderEnd

    contentWidth: content.childrenRect.width
    contentHeight: content.childrenRect.height
    onContentHeightChanged: {
        root.updatePosition()
    }

    function updatePosition() {
        var iconRect = soundFlagModel.iconRect

        var popupHeight = root.contentHeight + root.margins * 2 + root.padding * 2
        var popupHalfWidth = root.contentWidth / 2 + root.margins + root.padding

        root.x = iconRect.x + iconRect.width / 2 - popupHalfWidth
        root.y = iconRect.y - popupHeight

        root.setOpensUpward(true)
    }

    Column {
        id: content

        width: 300

        spacing: 12

        SoundFlagSettingsModel {
            id: soundFlagModel
        }

        Component.onCompleted: {
            soundFlagModel.init()
        }

        RowLayout {
            width: parent.width

            spacing: 6

            StyledIconLabel {
                iconCode: IconCode.AUDIO
            }

            StyledTextLabel {
                id: titleLabel

                Layout.fillWidth: true

                text: soundFlagModel.title
                font: ui.theme.bodyBoldFont
                horizontalAlignment: Text.AlignLeft
            }

            MenuButton {
                Layout.preferredWidth: 20
                Layout.preferredHeight: width

                menuModel: soundFlagModel.contextMenuModel

                navigation.panel: NavigationPanel {
                    id: menuNavPanel
                    name: "SoundFlagMenu"
                    direction: NavigationPanel.Vertical
                    section: root.notationViewNavigationSection
                    order: museSoundsParams.navigationPanelOrderEnd + 1
                    accessible.name: qsTrc("notation", "Sound flag menu")
                }

                onHandleMenuItem: function(itemId) {
                    soundFlagModel.handleContextMenuItem(itemId)
                }
            }
        }

        MuseSoundsParams {
            id: museSoundsParams

            width: parent.width

            model: soundFlagModel

            visible: soundFlagModel.inited

            navigationPanelSection: root.notationViewNavigationSection
            navigationPanelOrderStart: root.navigationOrderStart
        }
    }
}
