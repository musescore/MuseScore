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
#pragma once

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "global/iglobalconfiguration.h"
#include "io/ifilesystem.h"
#include "ui/iuiconfiguration.h"
#include "context/iglobalcontext.h"
#include "engraving/iengravingconfiguration.h"

#include "../inotationconfiguration.h"

namespace mu::notation {
class NotationConfiguration : public INotationConfiguration, public muse::async::Asyncable
{
    INJECT(muse::IGlobalConfiguration, globalConfiguration)
    INJECT(muse::io::IFileSystem, fileSystem)
    INJECT(muse::ui::IUiConfiguration, uiConfiguration)
    INJECT(engraving::IEngravingConfiguration, engravingConfiguration)
    INJECT(context::IGlobalContext, context)

public:
    void init();

    QColor backgroundColor() const override;
    void setBackgroundColor(const QColor& color) override;

    muse::io::path_t backgroundWallpaperPath() const override;
    const QPixmap& backgroundWallpaper() const override;
    void setBackgroundWallpaperPath(const muse::io::path_t& path) override;

    bool backgroundUseColor() const override;
    void setBackgroundUseColor(bool value) override;

    void resetBackground() override;

    muse::async::Notification backgroundChanged() const override;

    QColor foregroundColor() const override;
    void setForegroundColor(const QColor& color) override;

    muse::io::path_t foregroundWallpaperPath() const override;
    const QPixmap& foregroundWallpaper() const override;
    void setForegroundWallpaperPath(const muse::io::path_t& path) override;

    bool foregroundUseColor() const override;
    void setForegroundUseColor(bool value) override;

    void resetForeground() override;

    muse::async::Notification foregroundChanged() const override;

    muse::io::path_t wallpapersDefaultDirPath() const override;

    QColor borderColor() const override;
    int borderWidth() const override;

    QColor playbackCursorColor() const override;
    QColor loopMarkerColor() const override;
    int cursorOpacity() const override;

    bool thinNoteInputCursor() const override;

    QColor selectionColor(engraving::voice_idx_t voiceIndex = 0) const override;

    QColor dropRectColor() const override;

    muse::draw::Color noteInputPreviewColor() const override;

    bool useNoteInputCursorInInputByDuration() const override;
    void setUseNoteInputCursorInInputByDuration(bool use) override;
    muse::async::Notification useNoteInputCursorInInputByDurationChanged() const override;

    int selectionProximity() const override;
    void setSelectionProximity(int proximity) override;
    muse::async::Channel<int> selectionProximityChanged() const override;

    ZoomType defaultZoomType() const override;
    void setDefaultZoomType(ZoomType zoomType) override;

    int defaultZoom() const override;
    void setDefaultZoom(int zoomPercentage) override;
    muse::async::Notification defaultZoomChanged() const override;

    qreal scalingFromZoomPercentage(int zoomPercentage) const override;
    int zoomPercentageFromScaling(qreal scaling) const override;

    QList<int> possibleZoomPercentageList() const override;

    int mouseZoomPrecision() const override;
    void setMouseZoomPrecision(int precision) override;
    muse::async::Notification mouseZoomPrecisionChanged() const override;

    std::string fontFamily() const override;
    int fontSize() const override;

    muse::io::path_t userStylesPath() const override;
    void setUserStylesPath(const muse::io::path_t& path) override;
    muse::async::Channel<muse::io::path_t> userStylesPathChanged() const override;

    muse::io::path_t defaultStyleFilePath() const override;
    void setDefaultStyleFilePath(const muse::io::path_t& path) override;
    muse::async::Channel<muse::io::path_t> defaultStyleFilePathChanged() const override;

    muse::io::path_t partStyleFilePath() const override;
    void setPartStyleFilePath(const muse::io::path_t& path) override;
    muse::async::Channel<muse::io::path_t> partStyleFilePathChanged() const override;

    NoteInputMethod defaultNoteInputMethod() const override;
    void setDefaultNoteInputMethod(NoteInputMethod method) override;
    muse::async::Notification defaultNoteInputMethodChanged() const override;

    bool addAccidentalDotsArticulationsToNextNoteEntered() const override;
    void setAddAccidentalDotsArticulationsToNextNoteEntered(bool value) override;
    muse::async::Notification addAccidentalDotsArticulationsToNextNoteEnteredChanged() const override;

    muse::io::path_t userMusicFontsPath() const override;
    void setUserMusicFontsPath(const muse::io::path_t& path) override;
    muse::async::Channel<muse::io::path_t> userMusicFontsPathChanged() const override;

    bool isMidiInputEnabled() const override;
    void setIsMidiInputEnabled(bool enabled) override;
    muse::async::Notification isMidiInputEnabledChanged() const override;

    bool startNoteInputAtSelectionWhenPressingMidiKey() const override;
    void setStartNoteInputAtSelectionWhenPressingMidiKey(bool value) override;
    muse::async::Notification startNoteInputAtSelectionWhenPressingMidiKeyChanged() const override;

    bool isAutomaticallyPanEnabled() const override;
    void setIsAutomaticallyPanEnabled(bool enabled) override;

    bool isSmoothPanning() const override;
    void setIsSmoothPanning(bool value) override;

    bool isPlayRepeatsEnabled() const override;
    void setIsPlayRepeatsEnabled(bool enabled) override;
    muse::async::Notification isPlayRepeatsChanged() const override;

    bool isPlayChordSymbolsEnabled() const override;
    void setIsPlayChordSymbolsEnabled(bool enabled) override;
    muse::async::Notification isPlayChordSymbolsChanged() const override;

    bool isPlayPreviewNotesInInputByDuration() const override;
    void setIsPlayPreviewNotesInInputByDuration(bool play) override;
    muse::async::Notification isPlayPreviewNotesInInputByDurationChanged() const override;

    bool isMetronomeEnabled() const override;
    void setIsMetronomeEnabled(bool enabled) override;
    muse::async::Notification isMetronomeEnabledChanged() const override;

    bool isCountInEnabled() const override;
    void setIsCountInEnabled(bool enabled) override;

    double guiScaling() const override;
    double notationScaling() const override;

    muse::ValCh<muse::Orientation> canvasOrientation() const override;
    void setCanvasOrientation(muse::Orientation orientation) override;

    bool isLimitCanvasScrollArea() const override;
    void setIsLimitCanvasScrollArea(bool limited) override;
    muse::async::Notification isLimitCanvasScrollAreaChanged() const override;

    bool colorNotesOutsideOfUsablePitchRange() const override;
    void setColorNotesOutsideOfUsablePitchRange(bool value) override;
    muse::async::Channel<bool> colorNotesOutsideOfUsablePitchRangeChanged() const override;

    bool warnGuitarBends() const override;
    void setWarnGuitarBends(bool value) override;
    muse::async::Channel<bool> warnGuitarBendsChanged() const override;

    int delayBetweenNotesInRealTimeModeMilliseconds() const override;
    void setDelayBetweenNotesInRealTimeModeMilliseconds(int delayMs) override;
    muse::async::Channel<int> delayBetweenNotesInRealTimeModeMillisecondsChanged() const override;

    int notePlayDurationMilliseconds() const override;
    void setNotePlayDurationMilliseconds(int durationMs) override;
    muse::async::Channel<int> notePlayDurationMillisecondsChanged() const override;

    void setTemplateModeEnabled(std::optional<bool> enabled) override;
    void setTestModeEnabled(std::optional<bool> enabled) override;

    muse::io::path_t instrumentListPath() const override;

    muse::io::paths_t scoreOrderListPaths() const override;
    muse::async::Notification scoreOrderListPathsChanged() const override;

    muse::io::paths_t userScoreOrderListPaths() const override;
    void setUserScoreOrderListPaths(const muse::io::paths_t& paths) override;

    muse::io::path_t stringTuningsPresetsPath() const override;

    bool isSnappedToGrid(muse::Orientation gridOrientation) const override;
    void setIsSnappedToGrid(muse::Orientation gridOrientation, bool isSnapped) override;

    int gridSizeSpatium(muse::Orientation gridOrientation) const override;
    void setGridSize(muse::Orientation gridOrientation, int sizeSpatium) override;

    bool needToShowAddTextErrorMessage() const override;
    void setNeedToShowAddTextErrorMessage(bool show) override;

    bool needToShowAddFiguredBassErrorMessage() const override;
    void setNeedToShowAddFiguredBassErrorMessage(bool show) override;

    bool needToShowAddGuitarBendErrorMessage() const override;
    void setNeedToShowAddGuitarBendErrorMessage(bool show) override;

    bool needToShowMScoreError(const std::string& errorKey) const override;
    void setNeedToShowMScoreError(const std::string& errorKey, bool show) override;

    muse::ValCh<int> pianoKeyboardNumberOfKeys() const override;
    void setPianoKeyboardNumberOfKeys(int number) override;

    muse::ValCh<bool> midiUseWrittenPitch() const override;
    void setMidiUseWrittenPitch(bool useWrittenPitch) override;

    bool useNewPercussionPanel() const override;
    void setUseNewPercussionPanel(bool use) override;
    muse::async::Notification useNewPercussionPanelChanged() const override;

    bool percussionPanelUseNotationPreview() const override;
    void setPercussionPanelUseNotationPreview(bool use) override;
    muse::async::Notification percussionPanelUseNotationPreviewChanged() const override;

    PercussionPanelAutoShowMode percussionPanelAutoShowMode() const override;
    void setPercussionPanelAutoShowMode(PercussionPanelAutoShowMode percussionPanelAutoShowMode) override;
    muse::async::Notification percussionPanelAutoShowModeChanged() const override;

    bool autoClosePercussionPanel() const override;
    void setAutoClosePercussionPanel(bool autoClose) override;
    muse::async::Notification autoClosePercussionPanelChanged() const override;

    bool showPercussionPanelPadSwapDialog() const override;
    void setShowPercussionPanelPadSwapDialog(bool show) override;
    muse::async::Notification showPercussionPanelPadSwapDialogChanged() const override;

    bool percussionPanelMoveMidiNotesAndShortcuts() const override;
    void setPercussionPanelMoveMidiNotesAndShortcuts(bool move) override;
    muse::async::Notification percussionPanelMoveMidiNotesAndShortcutsChanged() const override;

    muse::io::path_t styleFileImportPath() const override;
    void setStyleFileImportPath(const muse::io::path_t& path) override;
    muse::async::Channel<std::string> styleFileImportPathChanged() const override;

    int styleDialogLastPageIndex() const override;
    void setStyleDialogLastPageIndex(int value) override;

    int styleDialogLastSubPageIndex() const override;
    void setStyleDialogLastSubPageIndex(int value) override;

    void resetStyleDialogPageIndices() override;

private:
    muse::io::path_t firstScoreOrderListPath() const;
    void setFirstScoreOrderListPath(const muse::io::path_t& path);

    muse::io::path_t secondScoreOrderListPath() const;
    void setSecondScoreOrderListPath(const muse::io::path_t& path);

    muse::async::Notification m_backgroundChanged;
    muse::async::Notification m_foregroundChanged;

    muse::async::Notification m_defaultNoteInputMethodChanged;
    muse::async::Notification m_addAccidentalDotsArticulationsToNextNoteEnteredChanged;
    muse::async::Notification m_useNoteInputCursorInInputByDurationChanged;
    muse::async::Notification m_isMidiInputEnabledChanged;
    muse::async::Notification m_startNoteInputAtSelectionWhenPressingMidiKeyChanged;

    muse::async::Notification m_defaultZoomChanged;
    muse::async::Notification m_mouseZoomPrecisionChanged;
    muse::async::Channel<muse::Orientation> m_canvasOrientationChanged;
    muse::async::Channel<muse::io::path_t> m_userStylesPathChanged;
    muse::async::Channel<muse::io::path_t> m_userMusicFontsPathChanged;
    muse::async::Notification m_scoreOrderListPathsChanged;
    muse::async::Notification m_isLimitCanvasScrollAreaChanged;
    muse::async::Channel<int> m_selectionProximityChanged;
    muse::async::Channel<bool> m_colorNotesOutsideOfUsablePitchRangeChanged;
    muse::async::Channel<bool> m_warnGuitarBendsChanged;
    muse::async::Channel<int> m_delayBetweenNotesInRealTimeModeMillisecondsChanged;
    muse::async::Channel<int> m_notePlayDurationMillisecondsChanged;
    muse::async::Channel<std::string> m_styleFileImportPathChanged;
    muse::async::Notification m_isPlayRepeatsChanged;
    muse::async::Notification m_isPlayChordSymbolsChanged;
    muse::async::Notification m_isPlayNotesPreviewInInputByDurationChanged;
    muse::async::Notification m_isMetronomeEnabledChanged;
    muse::ValCh<int> m_pianoKeyboardNumberOfKeys;
    muse::ValCh<bool> m_midiInputUseWrittenPitch;
    muse::async::Channel<QColor> m_anchorColorChanged;
    muse::async::Notification m_useNewPercussionPanelChanged;
    muse::async::Notification m_percussionPanelUseNotationPreviewChanged;
    muse::async::Notification m_percussionPanelAutoShowModeChanged;
    muse::async::Notification m_autoClosePercussionPanelChanged;
    muse::async::Notification m_showPercussionPanelPadSwapDialogChanged;
    muse::async::Notification m_percussionPanelMoveMidiNotesAndShortcutsChanged;

    int m_styleDialogLastPageIndex = 0;
    int m_styleDialogLastSubPageIndex = 0;
};
}
