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
pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.PropertiesPanel

import "common"
import "general"
import "measures"
import "systemlayout"
import "emptystaves"
import "notation"
import "text"
import "score"
import "parts"

ExpandableBlank {
    id: root

    required property PropertiesPanelAbstractModel sectionModel // Comes from propertiesPanelListModel
    property var anchorItem: null

    signal ensureContentVisibleRequested(int invisibleContentHeight)

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
        case PropertiesPanelAbstractModel.SECTION_GENERAL: return generalSection
        case PropertiesPanelAbstractModel.SECTION_MEASURES: return measuresSection
        case PropertiesPanelAbstractModel.SECTION_SYSTEM_PAGE_LAYOUT: return systemPageLayoutSection
        case PropertiesPanelAbstractModel.SECTION_EMPTY_STAVES: return emptyStavesSection
        case PropertiesPanelAbstractModel.SECTION_TEXT: return textSection
        case PropertiesPanelAbstractModel.SECTION_NOTATION:
            if ((sectionModel as PropertiesPanelAbstractProxyModel).isMultiModel) {
                return notationMultiElementsSection
            } else {
                return notationSingleElementSection
            }
        case PropertiesPanelAbstractModel.SECTION_SCORE_DISPLAY: return scoreSection
        case PropertiesPanelAbstractModel.SECTION_SCORE_APPEARANCE: return scoreAppearanceSection
        case PropertiesPanelAbstractModel.SECTION_PARTS: return partsSection
        }

        return undefined
    }

    Component {
        id: generalSection

        GeneralSection {
            model: root.sectionModel as GeneralSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
            anchorItem: root.anchorItem

            onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                root.ensureContentVisibleRequested(-invisibleContentHeight)
            }
        }
    }

    Component {
        id: measuresSection

        MeasuresSection {
            model: root.sectionModel as MeasuresSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
            anchorItem: root.anchorItem

            onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                root.ensureContentVisibleRequested(-invisibleContentHeight)
            }
        }
    }

    Component {
        id: systemPageLayoutSection

        SystemPageLayoutSection {
            model: root.sectionModel as SystemPageLayoutSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
            anchorItem: root.anchorItem

            onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                root.ensureContentVisibleRequested(-invisibleContentHeight)
            }
        }
    }

    Component {
        id: emptyStavesSection

        EmptyStavesSection {
            model: root.sectionModel as EmptyStavesVisibilitySettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
            anchorItem: root.anchorItem

            onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                root.ensureContentVisibleRequested(-invisibleContentHeight)
            }
        }
    }

    Component {
        id: textSection

        TextPropertiesSection {
            model: root.sectionModel as TextSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
            anchorItem: root.anchorItem

            onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                root.ensureContentVisibleRequested(-invisibleContentHeight)
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
        }
    }

    Component {
        id: notationSingleElementSection

        NotationSingleElementView {
            model: root.sectionModel as NotationSettingsProxyModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
        }
    }

    Component {
        id: scoreSection

        ScoreDisplaySection {
            model: root.sectionModel as ScoreDisplaySettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
        }
    }

    Component {
        id: scoreAppearanceSection

        ScoreAppearanceSection {
            model: root.sectionModel as ScoreAppearanceSettingsModel
            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigation.row + 1
            anchorItem: root.anchorItem

            onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                root.ensureContentVisibleRequested(-invisibleContentHeight)
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
        }
    }
}
