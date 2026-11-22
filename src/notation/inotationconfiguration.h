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

#pragma once

#include <QColor>
#include <optional>

#include "modularity/imoduleinterface.h"
#include "async/channel.h"
#include "types/retval.h"
#include "io/path.h"
#include "global/globaltypes.h"

#include "types/noteinputtypes.h"
#include "types/zoomtype.h"
#include "types/automationlane.h"

namespace mu::notation {
class INotationConfiguration : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(INotationConfiguration)

public:
    virtual ~INotationConfiguration() = default;

    virtual QColor notationColor() const = 0;
    virtual void setNotationColor(const QColor& color) = 0;
    virtual muse::async::Notification notationColorChanged() const = 0;
    virtual void resetNotationColor() = 0;

    virtual QColor backgroundColor() const = 0;
    virtual void setBackgroundColor(const QColor& color) = 0;

    virtual muse::io::path_t backgroundWallpaperPath() const = 0;
    virtual const QPixmap& backgroundWallpaper() const = 0;
    virtual void setBackgroundWallpaperPath(const muse::io::path_t& path) = 0;

    virtual bool backgroundUseColor() const = 0;
    virtual void setBackgroundUseColor(bool value) = 0;

    virtual void resetBackground() = 0;

    virtual muse::async::Notification backgroundChanged() const = 0;

    virtual QColor foregroundColor() const = 0;
    virtual void setForegroundColor(const QColor& color) = 0;

    virtual muse::io::path_t foregroundWallpaperPath() const = 0;
    virtual const QPixmap& foregroundWallpaper() const = 0;
    virtual void setForegroundWallpaperPath(const muse::io::path_t& path) = 0;

    virtual bool foregroundUseColor() const = 0;
    virtual void setForegroundUseColor(bool value) = 0;

    virtual void resetForeground() = 0;

    virtual muse::async::Notification foregroundChanged() const = 0;

    virtual muse::io::path_t wallpapersDefaultDirPath() const = 0;

    virtual bool shouldInvertScore() const = 0;  // Whether score should be inverted now, based on theme.

    virtual bool scoreInversionEnabled() const = 0;
    virtual void setScoreInversionEnabled(bool value) = 0;
    virtual muse::async::Notification scoreInversionChanged() const = 0;

    virtual bool isOnlyInvertInDarkTheme() const = 0;
    virtual void setOnlyInvertInDarkTheme(bool value) = 0;
    virtual muse::async::Notification isOnlyInvertInDarkThemeChanged() const = 0;

    virtual QColor borderColor() const = 0;
    virtual int borderWidth() const = 0;

    virtual QColor playbackCursorColor() const = 0;
    virtual QColor loopMarkerColor() const = 0;
    virtual int cursorOpacity() const = 0;

    virtual bool thinNoteInputCursor() const = 0;

    virtual bool enableJianpu() const = 0;

    virtual QColor selectionColor(engraving::voice_idx_t voiceIndex = 0) const = 0;
    virtual QColor highlightSelectionColor(engraving::voice_idx_t voiceIndex = 0) const = 0;

    virtual QColor dropRectColor() const = 0;

    virtual muse::draw::Color noteInputPreviewColor() const = 0;

    virtual bool useNoteInputCursorInInputByDuration() const = 0;
    virtual void setUseNoteInputCursorInInputByDuration(bool use) = 0;
    virtual muse::async::Notification useNoteInputCursorInInputByDurationChanged() const = 0;

    virtual int selectionProximity() const = 0;
    virtual void setSelectionProximity(int proximity) = 0;
    virtual muse::async::Channel<int> selectionProximityChanged() const = 0;

    virtual ZoomType defaultZoomType() const = 0;
    virtual void setDefaultZoomType(ZoomType zoomType) = 0;

    virtual int defaultZoom() const = 0;
    virtual void setDefaultZoom(int zoomPercentage) = 0;
    virtual muse::async::Notification defaultZoomChanged() const = 0;

    virtual QList<int> possibleZoomPercentageList() const = 0;

    virtual AutomationLane currentAutomationLane() const = 0;
    virtual void setCurrentAutomationLane(AutomationLane lane) = 0;
    virtual muse::async::Notification currentAutomationLaneChanged() const = 0;

    virtual int mouseZoomPrecision() const = 0;
    virtual void setMouseZoomPrecision(int precision) = 0;
    virtual muse::async::Notification mouseZoomPrecisionChanged() const = 0;

    virtual std::string fontFamily() const = 0;
    virtual int fontSize() const = 0;

    virtual muse::io::path_t userStylesPath() const = 0;
    virtual void setUserStylesPath(const muse::io::path_t& path) = 0;
    virtual muse::async::Channel<muse::io::path_t> userStylesPathChanged() const = 0;

    virtual muse::io::path_t defaultStyleFilePath() const = 0;
    virtual void setDefaultStyleFilePath(const muse::io::path_t& path) = 0;
    virtual muse::async::Channel<muse::io::path_t> defaultStyleFilePathChanged() const = 0;

    virtual muse::io::path_t partStyleFilePath() const = 0;
    virtual void setPartStyleFilePath(const muse::io::path_t& path) = 0;
    virtual muse::async::Channel<muse::io::path_t> partStyleFilePathChanged() const = 0;

    virtual NoteInputMethod defaultNoteInputMethod() const = 0;
    virtual void setDefaultNoteInputMethod(NoteInputMethod method) = 0;
    virtual muse::async::Notification defaultNoteInputMethodChanged() const = 0;

    virtual bool addAccidentalDotsArticulationsToNextNoteEntered() const = 0;
    virtual void setAddAccidentalDotsArticulationsToNextNoteEntered(bool value) = 0;
    virtual muse::async::Notification addAccidentalDotsArticulationsToNextNoteEnteredChanged() const = 0;

    // what the tuplet commands (duplet to nonuplet) use the selected duration as (see TupletOptions::selectedDurationAs)
    virtual engraving::TupletSelectedDuration tupletCommandsSelectedDurationAs() const = 0;
    virtual void setTupletCommandsSelectedDurationAs(engraving::TupletSelectedDuration selectedDurationAs) = 0;
    virtual muse::async::Notification tupletCommandsSelectedDurationAsChanged() const = 0;

    virtual muse::io::path_t userMusicFontsPath() const = 0;
    virtual void setUserMusicFontsPath(const muse::io::path_t& path) = 0;
    virtual muse::async::Channel<muse::io::path_t> userMusicFontsPathChanged() const = 0;

    virtual bool isMidiInputEnabled() const = 0;
    virtual void setIsMidiInputEnabled(bool enabled) = 0;
    virtual muse::async::Notification isMidiInputEnabledChanged() const = 0;

    virtual bool startNoteInputAtSelectedNoteRestWhenPressingMidiKey() const = 0;
    virtual void setStartNoteInputAtSelectedNoteRestWhenPressingMidiKey(bool value) = 0;
    virtual muse::async::Notification startNoteInputAtSelectedNoteRestWhenPressingMidiKeyChanged() const = 0;

    virtual bool isAutomaticallyPanEnabled() const = 0;
    virtual void setIsAutomaticallyPanEnabled(bool enabled) = 0;
    virtual muse::async::Notification isAutomaticallyPanEnabledChanged() const = 0;

    virtual bool isPlayRepeatsEnabled() const = 0;
    virtual void setIsPlayRepeatsEnabled(bool enabled) = 0;
    virtual muse::async::Notification isPlayRepeatsChanged() const = 0;

    virtual bool isPlayChordSymbolsEnabled() const = 0;
    virtual void setIsPlayChordSymbolsEnabled(bool enabled) = 0;
    virtual muse::async::Notification isPlayChordSymbolsChanged() const = 0;

    virtual bool isPlayPreviewNotesInInputByDuration() const = 0;
    virtual void setIsPlayPreviewNotesInInputByDuration(bool play) = 0;
    virtual muse::async::Notification isPlayPreviewNotesInInputByDurationChanged() const = 0;

    virtual bool playPreviewNotesWithScoreDynamics() const = 0;
    virtual void setPlayPreviewNotesWithScoreDynamics(bool use) = 0;
    virtual muse::async::Notification playPreviewNotesWithScoreDynamicsChanged() const = 0;

    virtual bool isMetronomeEnabled() const = 0;
    virtual void setIsMetronomeEnabled(bool enabled) = 0;
    virtual muse::async::Notification isMetronomeEnabledChanged() const = 0;

    virtual bool isCountInEnabled() const = 0;
    virtual void setIsCountInEnabled(bool enabled) = 0;
    virtual muse::async::Notification isCountInEnabledChanged() const = 0;

    virtual muse::ValCh<muse::Orientation> canvasOrientation() const = 0;
    virtual void setCanvasOrientation(muse::Orientation orientation) = 0;

    virtual bool colorNotesOutsideOfUsablePitchRange() const = 0;
    virtual void setColorNotesOutsideOfUsablePitchRange(bool value) = 0;
    virtual muse::async::Channel<bool> colorNotesOutsideOfUsablePitchRangeChanged() const = 0;

    virtual bool warnGuitarBends() const = 0;
    virtual void setWarnGuitarBends(bool value) = 0;
    virtual muse::async::Channel<bool> warnGuitarBendsChanged() const = 0;

    virtual int delayBetweenNotesInRealTimeModeMilliseconds() const = 0;
    virtual void setDelayBetweenNotesInRealTimeModeMilliseconds(int delayMs) = 0;
    virtual muse::async::Channel<int> delayBetweenNotesInRealTimeModeMillisecondsChanged() const = 0;

    //! Added to the estimated audio output latency when recording from a MIDI keyboard with the metronome
    virtual int midiRecordingLatencyCompensationMilliseconds() const = 0;
    virtual void setMidiRecordingLatencyCompensationMilliseconds(int latencyMs) = 0;
    virtual muse::async::Channel<int> midiRecordingLatencyCompensationMillisecondsChanged() const = 0;

    //! MIDI recording with metronome: shortest duration written (ticks)
    virtual int midiRecordingMinQuantizationTicks() const = 0;
    virtual void setMidiRecordingMinQuantizationTicks(int value) = 0;

    //! MIDI recording with metronome: detect tuplets
    virtual bool midiRecordingSearchTuplets() const = 0;
    virtual void setMidiRecordingSearchTuplets(bool value) = 0;

    //! MIDI recording with metronome: lengthen notes up to the next one when the gap is shorter than the minimum quantization
    virtual bool midiRecordingLegato() const = 0;
    virtual void setMidiRecordingLegato(bool value) = 0;

    //! MIDI recording with metronome: measures of count-in (0, 1 or 2)
    virtual int midiRecordingCountInMeasures() const = 0;
    virtual void setMidiRecordingCountInMeasures(int value) = 0;

    //! MIDI recording with metronome: record on two staves, split by midiRecordingSplitPitch
    virtual bool midiRecordingTwoStaves() const = 0;
    virtual void setMidiRecordingTwoStaves(bool value) = 0;

    //! MIDI recording with metronome: the lowest pitch of the upper staff (default: middle C)
    virtual int midiRecordingSplitPitch() const = 0;
    virtual void setMidiRecordingSplitPitch(int value) = 0;

    //! MIDI recording with metronome: sent when one of the settings above changes
    virtual muse::async::Notification midiRecordingSettingsChanged() const = 0;

    virtual bool useMidiVelocityAndDurationDuringNoteInput() const = 0;
    virtual void setUseMidiVelocityAndDurationDuringNoteInput(bool use) = 0;
    virtual muse::async::Channel<bool> useMidiVelocityAndDurationDuringNoteInputChanged() const = 0;

    virtual int notePlayDurationMilliseconds() const = 0;
    virtual void setNotePlayDurationMilliseconds(int durationMs) = 0;
    virtual muse::async::Channel<int> notePlayDurationMillisecondsChanged() const = 0;

    virtual void setTemplateModeEnabled(std::optional<bool> enabled) = 0;
    virtual void setTestModeEnabled(std::optional<bool> enabled) = 0;

    virtual muse::io::path_t instrumentsXmlPath() const = 0;
    virtual muse::io::path_t scoreOrdersXmlPath() const = 0;

    virtual muse::io::path_t userInstrumentsFolder() const = 0;
    virtual muse::io::paths_t userInstrumentsAndScoreOrdersPaths() const = 0;
    virtual void setUserInstrumentsFolder(const muse::io::path_t& path) = 0;
    virtual muse::async::Channel<muse::io::path_t> userInstrumentsFolderChanged() const = 0;

    virtual muse::io::path_t stringTuningsPresetsPath() const = 0;

    virtual bool isSnappedToGrid(muse::Orientation gridOrientation) const = 0;
    virtual void setIsSnappedToGrid(muse::Orientation gridOrientation, bool isSnapped) = 0;

    virtual int gridSizeSpatium(muse::Orientation gridOrientation) const = 0;
    virtual void setGridSize(muse::Orientation gridOrientation, int sizeSpatium) = 0;

    virtual bool needToShowMScoreError(const std::string& errorKey) const = 0;
    virtual void setNeedToShowMScoreError(const std::string& errorKey, bool show) = 0;

    virtual muse::ValCh<bool> midiUseWrittenPitch() const = 0;
    virtual void setMidiUseWrittenPitch(bool useWrittenPitch) = 0;

    virtual muse::io::path_t styleFileImportPath() const = 0;
    virtual void setStyleFileImportPath(const muse::io::path_t& path) = 0;
    virtual muse::async::Channel<std::string> styleFileImportPathChanged() const = 0;
};
}
