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
pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.Inspector

import "common"
import "general"
import "measures"
import "emptystaves"
import "notation"
import "text"
import "score"
import "parts"

ExpandableBlank {
    id: root

    required property AbstractInspectorModel sectionModel // Comes from inspectorListModel
    property var anchorItem: null

    signal ensureContentVisibleRequested(int invisibleContentHeight)
    signal popupOpened(var openedPopup, var visualControl)

    property NavigationPanel navigationPanel: NavigationPanel {
        name: root.title
        direction: NavigationPanel.Vertical
        accessible.name: root.title
        enabled: root.enabled && root.visible
    }

    navigation.panel: root.navigationPanel
    navigation.row: 0

    title: root.sectionModel?.title ?? ""

    headerAccessory: contentItem?.headerAccessory

    contentItemComponent: {
        if (!root.sectionModel) {
            return undefined
        }

        switch (root.sectionModel.sectionType) {
        case AbstractInspectorModel.SECTION_GENERAL: return generalSection
        case AbstractInspectorModel.SECTION_MEASURES: return measuresSection
        case AbstractInspectorModel.SECTION_EMPTY_STAVES: return emptyStavesSection
        case AbstractInspectorModel.SECTION_VOICE_POSITION: return voiceAndPositionSection
        case AbstractInspectorModel.SECTION_TEXT_LINES:
        case AbstractInspectorModel.SECTION_TEXT: return textSection
        case AbstractInspectorModel.SECTION_NOTATION:
            if ((sectionModel as AbstractInspectorProxyModel).isMultiModel) {
                return notationMultiElementsSection
            } else {
                return notationSingleElementSection
            }
        case AbstractInspectorModel.SECTION_SCORE_DISPLAY: return scoreSection
        case AbstractInspectorModel.SECTION_SCORE_APPEARANCE: return scoreAppearanceSection
        case AbstractInspectorModel.SECTION_PARTS: return partsSection
        }

        return undefined
    }

    Component {
        id: generalSection

        GeneralInspectorView {
            model: root.sectionModel as GeneralSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
            anchorItem: root.anchorItem

            onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                root.ensureContentVisibleRequested(-invisibleContentHeight)
            }

            onPopupOpened: function(openedPopup, control) {
                root.popupOpened(openedPopup, control)
            }
        }
    }

    Component {
        id: measuresSection

        MeasuresInspectorView {
            model: root.sectionModel as MeasuresSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
            anchorItem: root.anchorItem

            onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                root.ensureContentVisibleRequested(-invisibleContentHeight)
            }

            onPopupOpened: function(openedPopup, control) {
                root.popupOpened(openedPopup, control)
            }
        }
    }

    Component {
        id: emptyStavesSection

        EmptyStavesVisibilityInspectorView {
            model: root.sectionModel as EmptyStavesVisibilitySettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
            anchorItem: root.anchorItem

            onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                root.ensureContentVisibleRequested(-invisibleContentHeight)
            }

            onPopupOpened: function(openedPopup, control) {
                root.popupOpened(openedPopup, control)
            }
        }
    }

    Component {
        id: voiceAndPositionSection

        VoiceAndPositionInspectorView {
            model: root.sectionModel as VoiceAndPositionSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
            anchorItem: root.anchorItem

            onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                root.ensureContentVisibleRequested(-invisibleContentHeight)
            }

            onPopupOpened: function(openedPopup, control) {
                root.popupOpened(openedPopup, control)
            }
        }
    }

    Component {
        id: textSection

        TextInspectorView {
            model: root.sectionModel as TextSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
            anchorItem: root.anchorItem

            onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                root.ensureContentVisibleRequested(-invisibleContentHeight)
            }

            onPopupOpened: function(openedPopup, control) {
                root.popupOpened(openedPopup, control)
            }
        }
    }

    Component {
        id: notationMultiElementsSection

        NotationMultiElementView {
            model: root.sectionModel as NotationSettingsProxyModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
            anchorItem: root.anchorItem

            onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                root.ensureContentVisibleRequested(-invisibleContentHeight)
            }

            onPopupOpened: function(openedPopup, control) {
                root.popupOpened(openedPopup, control)
            }
        }
    }

    Component {
        id: notationSingleElementSection

        NotationSingleElementView {
            model: root.sectionModel as NotationSettingsProxyModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1

            onPopupOpened: function(openedPopup, control) {
                root.popupOpened(openedPopup, control)
            }
        }
    }

    Component {
        id: scoreSection

        ScoreDisplayInspectorView {
            model: root.sectionModel as ScoreDisplaySettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1

            onPopupOpened: function(openedPopup, control) {
                root.popupOpened(openedPopup, control)
            }
        }
    }

    Component {
        id: scoreAppearanceSection

        ScoreAppearanceInspectorView {
            model: root.sectionModel as ScoreAppearanceSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
            anchorItem: root.anchorItem

            onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                root.ensureContentVisibleRequested(-invisibleContentHeight)
            }

            onPopupOpened: function(openedPopup, control) {
                root.popupOpened(openedPopup, control)
            }
        }
    }

    Component {
        id: partsSection

        PartsSettings {
            model: root.sectionModel as PartsSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
            anchorItem: root.anchorItem

            onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                root.ensureContentVisibleRequested(-invisibleContentHeight)
            }

            onPopupOpened: function(openedPopup, control) {
                root.popupOpened(openedPopup, control)
            }
        }
    }
}
