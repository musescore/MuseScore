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
import QtQuick

import Muse.UiComponents
import MuseScore.Preferences

import "internal"

PreferencesPage {
    id: root

    Component.onCompleted: {
        importPreferencesModel.load()
    }

    ImportPreferencesModel {
        id: importPreferencesModel
    }

    Column {
        width: parent.width
        spacing: root.sectionsSpacing

        ImportStyleSection {
            id: importStyleSection
            styleFileImportPath: importPreferencesModel.styleFileImportPath
            fileChooseTitle: importPreferencesModel.styleChooseTitle()
            filePathFilter: importPreferencesModel.stylePathFilter()
            fileDirectory: importPreferencesModel.fileDirectory(styleFileImportPath)

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 1

            onStyleFileImportPathChangeRequested: function(path) {
                importPreferencesModel.styleFileImportPath = path
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }
        }

        SeparatorLine { }

        CharsetsSection {
            charsets: importPreferencesModel.charsets()
            currentOvertureCharset: importPreferencesModel.currentOvertureCharset

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 2

            onOvertureCharsetChangeRequested: function(charset) {
                importPreferencesModel.currentOvertureCharset = charset
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }
        }

        SeparatorLine { }

        MusicXmlSection {
            importLayout: importPreferencesModel.importLayout
            importBreaks: importPreferencesModel.importBreaks
            needUseDefaultFont: importPreferencesModel.needUseDefaultFont
            inferTextType: importPreferencesModel.inferTextType

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 3

            onImportLayoutChangeRequested: function(importLayout) {
                importPreferencesModel.importLayout = importLayout
            }

            onImportBreaksChangeRequested: function(importBreaks) {
                importPreferencesModel.importBreaks = importBreaks
            }

            onUseDefaultFontChangeRequested: function(use) {
                importPreferencesModel.needUseDefaultFont = use
            }

            onInferTextTypeChangeRequested: function (inferTextType) {
                importPreferencesModel.inferTextType = inferTextType
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }
        }

        SeparatorLine { }

        MidiSection {
            shortestNotes: importPreferencesModel.shortestNotes()
            currentShortestNote: importPreferencesModel.currentShortestNote

            roundTempo: importPreferencesModel.roundTempo

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 4

            onCurrentShortestNoteChangeRequested: function(note) {
                importPreferencesModel.currentShortestNote = note
            }

            onRoundTempoChangeRequested: function(round) {
                importPreferencesModel.roundTempo = round
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }
        }

        SeparatorLine { }

        MeiSection {
            meiImportLayout: importPreferencesModel.meiImportLayout

            onMeiImportLayoutChangeRequested: function(meiImportLayout) {
                importPreferencesModel.meiImportLayout = meiImportLayout
            }
        }

        SeparatorLine { }

        MnxSection {
            requireExactSchemaValidation: importPreferencesModel.mnxRequireExactSchemaValidation

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 5

            onRequireExactSchemaValidationChangeRequested: function(value) {
                importPreferencesModel.mnxRequireExactSchemaValidation = value
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }
        }

        SeparatorLine { }

        EncoreSection {
            importPageLayout: importPreferencesModel.encoreImportPageLayout
            importPageBreaks: importPreferencesModel.encoreImportPageBreaks
            importSystemLocks: importPreferencesModel.encoreImportSystemLocks
            importStaffSize: importPreferencesModel.encoreImportStaffSize
            importTempoTextSemantic: importPreferencesModel.encoreImportTempoTextSemantic
            importUnsupportedArticulationsAsText: importPreferencesModel.encoreImportUnsupportedArticulationsAsText

            instrumentSearchModeModel: importPreferencesModel.encoreInstrumentSearchModeModel()
            currentInstrumentSearchMode: importPreferencesModel.encoreInstrumentSearchMode

            tablatureImportModeModel: importPreferencesModel.encoreTablatureImportModeModel()
            currentTablatureImportMode: importPreferencesModel.encoreTablatureImportMode

            underfillStrategyModel: importPreferencesModel.encoreUnderfillStrategyModel()
            currentUnderfillStrategy: importPreferencesModel.encoreUnderfillStrategy
            overfillStrategyModel: importPreferencesModel.encoreOverfillStrategyModel()
            currentOverfillStrategy: importPreferencesModel.encoreOverfillStrategy

            firstMeasureIsPickup: importPreferencesModel.encoreFirstMeasureIsPickup
            mergeVoices: importPreferencesModel.encoreMergeVoices

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 6

            onImportPageLayoutChangeRequested: function(value) {
                importPreferencesModel.encoreImportPageLayout = value
            }

            onImportPageBreaksChangeRequested: function(value) {
                importPreferencesModel.encoreImportPageBreaks = value
            }

            onImportSystemLocksChangeRequested: function(value) {
                importPreferencesModel.encoreImportSystemLocks = value
            }

            onImportStaffSizeChangeRequested: function(value) {
                importPreferencesModel.encoreImportStaffSize = value
            }

            onImportTempoTextSemanticChangeRequested: function(value) {
                importPreferencesModel.encoreImportTempoTextSemantic = value
            }

            onImportUnsupportedArticulationsAsTextChangeRequested: function(value) {
                importPreferencesModel.encoreImportUnsupportedArticulationsAsText = value
            }

            onInstrumentSearchModeChangeRequested: function(value) {
                importPreferencesModel.encoreInstrumentSearchMode = value
            }

            onTablatureImportModeChangeRequested: function(value) {
                importPreferencesModel.encoreTablatureImportMode = value
            }

            onUnderfillStrategyChangeRequested: function(value) {
                importPreferencesModel.encoreUnderfillStrategy = value
            }

            onOverfillStrategyChangeRequested: function(value) {
                importPreferencesModel.encoreOverfillStrategy = value
            }

            onFirstMeasureIsPickupChangeRequested: function(value) {
                importPreferencesModel.encoreFirstMeasureIsPickup = value
            }

            onMergeVoicesChangeRequested: function(value) {
                importPreferencesModel.encoreMergeVoices = value
            }

            onResetToDefaultRequested: {
                importPreferencesModel.encoreImportPageLayout = true
                importPreferencesModel.encoreImportPageBreaks = true
                importPreferencesModel.encoreImportSystemLocks = true
                importPreferencesModel.encoreImportStaffSize = true
                importPreferencesModel.encoreImportTempoTextSemantic = true
                importPreferencesModel.encoreImportUnsupportedArticulationsAsText = false
                importPreferencesModel.encoreInstrumentSearchMode = 0   // NameAndMidi
                importPreferencesModel.encoreUnderfillStrategy = 2      // IrregularMeasure
                importPreferencesModel.encoreOverfillStrategy = 2       // IrregularMeasure
                importPreferencesModel.encoreFirstMeasureIsPickup = true
                importPreferencesModel.encoreMergeVoices = true
                importPreferencesModel.encoreTablatureImportMode = 1   // Linked
            }

            onFocusChanged: {
                if (activeFocus) {
                    if (!suppressEnsureVisible) {
                        root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                    }
                    suppressEnsureVisible = false
                }
            }
        }
    }

    function reset() {
        importStyleSection.reset()
    }
}
