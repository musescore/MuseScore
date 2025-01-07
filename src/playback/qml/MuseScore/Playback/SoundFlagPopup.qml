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

import MuseScore.Playback 1.0

import "internal/SoundFlag"

StyledPopupView {
    id: root

    property alias notationViewNavigationSection: navPanel.section
    property alias navigationOrderStart: navPanel.order
    readonly property alias navigationOrderEnd: museSoundsParams.navigationPanelOrderEnd

    property QtObject model: soundFlagModel

    contentWidth: content.width
    contentHeight: content.childrenRect.height
    onContentHeightChanged: {
        root.updatePosition()
    }

    showArrow: false

    openPolicies: PopupView.Default | PopupView.OpenOnContentReady
    isContentReady: soundFlagModel.inited

    signal elementRectChanged(var elementRect)

    function updatePosition() {
        var popupHeight = root.contentHeight + root.margins * 2 + root.padding * 2
        root.y = -popupHeight
        root.x = (root.parent.width / 2) - (root.width / 2) + root.margins

        root.setOpensUpward(true)
    }

    Column {
        id: content

        width: 294

        spacing: 12

        SoundFlagSettingsModel {
            id: soundFlagModel

            onIconRectChanged: function(rect) {
                root.elementRectChanged(rect)
            }
        }

        Component.onCompleted: {
            soundFlagModel.init()
        }

        RowLayout {
            width: parent.width

            spacing: 6

            NavigationPanel {
                id: navPanel
                name: "SoundFlagSettings"
                direction: NavigationPanel.Vertical
                section: root.notationViewNavigationSection
                order: root.navigationOrderStart
                accessible.name: qsTrc("playback", "Sound flag settings")
            }

            StyledIconLabel {
                Layout.preferredWidth: 24
                Layout.preferredHeight: width

                iconCode: IconCode.AUDIO
            }

            StyledTextLabel {
                id: titleLabel

                Layout.fillWidth: true

                text: soundFlagModel.title
                font: ui.theme.largeBodyBoldFont
                horizontalAlignment: Text.AlignLeft

                NavigationControl {
                    id: navCtrl
                    name: "SoundFlagTitle"
                    enabled: titleLabel.enabled && titleLabel.visible
                    panel: navPanel

                    order: 1

                    accessible.role: MUAccessible.StaticText
                    accessible.visualItem: titleLabel
                    accessible.name: titleLabel.text
                }

                NavigationFocusBorder {
                    navigationCtrl: navCtrl
                }
            }

            MenuButton {
                Layout.preferredWidth: 24
                Layout.preferredHeight: width

                menuModel: soundFlagModel.contextMenuModel

                enabled: !museSoundsParams.noOptions

                navigation.panel: navPanel
                navigation.order: 2
                navigation.accessible.name: qsTrc("playback", "Sound flag menu")

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
            navigationPanelOrderStart: navPanel.order + 1
        }
    }
}
