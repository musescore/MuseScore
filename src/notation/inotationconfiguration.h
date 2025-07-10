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
#ifndef MU_NOTATION_INOTATIONCONFIGURATION_H
#define MU_NOTATION_INOTATIONCONFIGURATION_H

#include <QColor>
#include <optional>

#include "modularity/imoduleinterface.h"
#include "async/channel.h"
#include "types/retval.h"
#include "io/path.h"
#include "notationtypes.h"
#include "global/globaltypes.h"

namespace mu::notation {
class INotationConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(INotationConfiguration)

public:
    virtual ~INotationConfiguration() = default;

    virtual QColor notationColor() const = 0;

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

    virtual QColor borderColor() const = 0;
    virtual int borderWidth() const = 0;

    virtual QColor playbackCursorColor() const = 0;
    virtual QColor loopMarkerColor() const = 0;
    virtual int cursorOpacity() const = 0;

    virtual bool thinNoteInputCursor() const = 0;

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

    virtual qreal scalingFromZoomPercentage(int zoomPercentage) const = 0;
    virtual int zoomPercentageFromScaling(qreal scaling) const = 0;

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

    virtual muse::io::path_t userMusicFontsPath() const = 0;
    virtual void setUserMusicFontsPath(const muse::io::path_t& path) = 0;
    virtual muse::async::Channel<muse::io::path_t> userMusicFontsPathChanged() const = 0;

    virtual bool isMidiInputEnabled() const = 0;
    virtual void setIsMidiInputEnabled(bool enabled) = 0;
    virtual muse::async::Notification isMidiInputEnabledChanged() const = 0;

    virtual bool startNoteInputAtSelectionWhenPressingMidiKey() const = 0;
    virtual void setStartNoteInputAtSelectionWhenPressingMidiKey(bool value) = 0;
    virtual muse::async::Notification startNoteInputAtSelectionWhenPressingMidiKeyChanged() const = 0;

    virtual bool isAutomaticallyPanEnabled() const = 0;
    virtual void setIsAutomaticallyPanEnabled(bool enabled) = 0;

    virtual bool isSmoothPanning() const = 0;
    virtual void setIsSmoothPanning(bool value) = 0;

    virtual bool isPlayRepeatsEnabled() const = 0;
    virtual void setIsPlayRepeatsEnabled(bool enabled) = 0;
    virtual muse::async::Notification isPlayRepeatsChanged() const = 0;

    virtual bool isPlayChordSymbolsEnabled() const = 0;
    virtual void setIsPlayChordSymbolsEnabled(bool enabled) = 0;
    virtual muse::async::Notification isPlayChordSymbolsChanged() const = 0;

    virtual bool isPlayPreviewNotesInInputByDuration() const = 0;
    virtual void setIsPlayPreviewNotesInInputByDuration(bool play) = 0;
    virtual muse::async::Notification isPlayPreviewNotesInInputByDurationChanged() const = 0;

    virtual bool isMetronomeEnabled() const = 0;
    virtual void setIsMetronomeEnabled(bool enabled) = 0;
    virtual muse::async::Notification isMetronomeEnabledChanged() const = 0;

    virtual bool isCountInEnabled() const = 0;
    virtual void setIsCountInEnabled(bool enabled) = 0;

    virtual double guiScaling() const = 0;
    virtual double notationScaling() const = 0;

    virtual muse::ValCh<muse::Orientation> canvasOrientation() const = 0;
    virtual void setCanvasOrientation(muse::Orientation orientation) = 0;

    virtual bool isLimitCanvasScrollArea() const = 0;
    virtual void setIsLimitCanvasScrollArea(bool limited) = 0;
    virtual muse::async::Notification isLimitCanvasScrollAreaChanged() const = 0;

    virtual bool colorNotesOutsideOfUsablePitchRange() const = 0;
    virtual void setColorNotesOutsideOfUsablePitchRange(bool value) = 0;
    virtual muse::async::Channel<bool> colorNotesOutsideOfUsablePitchRangeChanged() const = 0;

    virtual bool warnGuitarBends() const = 0;
    virtual void setWarnGuitarBends(bool value) = 0;
    virtual muse::async::Channel<bool> warnGuitarBendsChanged() const = 0;

    virtual int delayBetweenNotesInRealTimeModeMilliseconds() const = 0;
    virtual void setDelayBetweenNotesInRealTimeModeMilliseconds(int delayMs) = 0;
    virtual muse::async::Channel<int> delayBetweenNotesInRealTimeModeMillisecondsChanged() const = 0;

    virtual int notePlayDurationMilliseconds() const = 0;
    virtual void setNotePlayDurationMilliseconds(int durationMs) = 0;
    virtual muse::async::Channel<int> notePlayDurationMillisecondsChanged() const = 0;

    virtual void setTemplateModeEnabled(std::optional<bool> enabled) = 0;
    virtual void setTestModeEnabled(std::optional<bool> enabled) = 0;

    virtual muse::io::path_t instrumentListPath() const = 0;

    virtual muse::io::paths_t scoreOrderListPaths() const = 0;
    virtual muse::async::Notification scoreOrderListPathsChanged() const = 0;

    virtual muse::io::paths_t userScoreOrderListPaths() const = 0;
    virtual void setUserScoreOrderListPaths(const muse::io::paths_t& paths) = 0;

    virtual muse::io::path_t stringTuningsPresetsPath() const = 0;

    virtual bool isSnappedToGrid(muse::Orientation gridOrientation) const = 0;
    virtual void setIsSnappedToGrid(muse::Orientation gridOrientation, bool isSnapped) = 0;

    virtual int gridSizeSpatium(muse::Orientation gridOrientation) const = 0;
    virtual void setGridSize(muse::Orientation gridOrientation, int sizeSpatium) = 0;

    virtual bool needToShowAddTextErrorMessage() const = 0;
    virtual void setNeedToShowAddTextErrorMessage(bool show) = 0;

    virtual bool needToShowAddFiguredBassErrorMessage() const = 0;
    virtual void setNeedToShowAddFiguredBassErrorMessage(bool show) = 0;

    virtual bool needToShowAddGuitarBendErrorMessage() const = 0;
    virtual void setNeedToShowAddGuitarBendErrorMessage(bool show) = 0;

    virtual bool needToShowMScoreError(const std::string& errorKey) const = 0;
    virtual void setNeedToShowMScoreError(const std::string& errorKey, bool show) = 0;

    virtual muse::ValCh<int> pianoKeyboardNumberOfKeys() const = 0;
    virtual void setPianoKeyboardNumberOfKeys(int number) = 0;

    virtual muse::ValCh<bool> midiUseWrittenPitch() const = 0;
    virtual void setMidiUseWrittenPitch(bool useWrittenPitch) = 0;

    virtual bool useNewPercussionPanel() const = 0;
    virtual void setUseNewPercussionPanel(bool use) = 0;
    virtual muse::async::Notification useNewPercussionPanelChanged() const = 0;

    virtual bool percussionPanelUseNotationPreview() const = 0;
    virtual void setPercussionPanelUseNotationPreview(bool use) = 0;
    virtual muse::async::Notification percussionPanelUseNotationPreviewChanged() const = 0;

    virtual PercussionPanelAutoShowMode percussionPanelAutoShowMode() const = 0;
    virtual void setPercussionPanelAutoShowMode(PercussionPanelAutoShowMode autoShowMode) = 0;
    virtual muse::async::Notification percussionPanelAutoShowModeChanged() const = 0;

    virtual bool autoClosePercussionPanel() const = 0;
    virtual void setAutoClosePercussionPanel(bool autoClose) = 0;
    virtual muse::async::Notification autoClosePercussionPanelChanged() const = 0;

    virtual bool showPercussionPanelPadSwapDialog() const = 0;
    virtual void setShowPercussionPanelPadSwapDialog(bool show) = 0;
    virtual muse::async::Notification showPercussionPanelPadSwapDialogChanged() const = 0;

    virtual bool percussionPanelMoveMidiNotesAndShortcuts() const = 0;
    virtual void setPercussionPanelMoveMidiNotesAndShortcuts(bool move) = 0;
    virtual muse::async::Notification percussionPanelMoveMidiNotesAndShortcutsChanged() const = 0;

    virtual muse::io::path_t styleFileImportPath() const = 0;
    virtual void setStyleFileImportPath(const muse::io::path_t& path) = 0;
    virtual muse::async::Channel<std::string> styleFileImportPathChanged() const = 0;

    virtual int styleDialogLastPageIndex() const = 0;
    virtual void setStyleDialogLastPageIndex(int value) = 0;

    virtual int styleDialogLastSubPageIndex() const = 0;
    virtual void setStyleDialogLastSubPageIndex(int value) = 0;

    virtual void resetStyleDialogPageIndices() = 0;
};
}

#endif // MU_NOTATION_INOTATIONCONFIGURATION_H
