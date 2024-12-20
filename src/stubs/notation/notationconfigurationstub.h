/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#ifndef MU_NOTATION_NOTATIONCONFIGURATIONSTUB_H
#define MU_NOTATION_NOTATIONCONFIGURATIONSTUB_H

#include "notation/inotationconfiguration.h"

namespace mu::notation {
class NotationConfigurationStub : public INotationConfiguration
{
public:
    NotationConfigurationStub() = default;

    QColor backgroundColor() const override;
    void setBackgroundColor(const QColor& color)  override;

    muse::io::path_t backgroundWallpaperPath() const override;
    const QPixmap& backgroundWallpaper() const override;
    void setBackgroundWallpaperPath(const muse::io::path_t& path)  override;

    bool backgroundUseColor() const override;
    void setBackgroundUseColor(bool value)  override;

    void resetBackground()  override;

    muse::async::Notification backgroundChanged() const override;

    QColor foregroundColor() const override;
    void setForegroundColor(const QColor& color)  override;

    muse::io::path_t foregroundWallpaperPath() const override;
    const QPixmap& foregroundWallpaper() const override;
    void setForegroundWallpaperPath(const muse::io::path_t& path)  override;

    bool foregroundUseColor() const override;
    void setForegroundUseColor(bool value)  override;

    void resetForeground()  override;

    muse::async::Notification foregroundChanged() const override;

    muse::io::path_t wallpapersDefaultDirPath() const override;

    QColor borderColor() const override;
    int borderWidth() const override;

    QColor anchorColor() const override;

    QColor playbackCursorColor() const override;
    QColor loopMarkerColor() const override;
    int cursorOpacity() const override;

    QColor selectionColor(engraving::voice_idx_t voiceIndex = 0) const override;

    QColor dropRectColor() const override;

    int selectionProximity() const override;
    void setSelectionProximity(int proximity)  override;

    ZoomType defaultZoomType() const override;
    void setDefaultZoomType(ZoomType zoomType)  override;

    int defaultZoom() const override;
    void setDefaultZoom(int zoomPercentage)  override;

    QList<int> possibleZoomPercentageList() const override;

    qreal scalingFromZoomPercentage(int zoomPercentage) const override;
    int zoomPercentageFromScaling(qreal scaling) const override;

    int mouseZoomPrecision() const override;
    void setMouseZoomPrecision(int precision)  override;

    std::string fontFamily() const override;
    int fontSize() const override;

    muse::io::path_t userStylesPath() const override;
    void setUserStylesPath(const muse::io::path_t& path)  override;
    muse::async::Channel<muse::io::path_t> userStylesPathChanged() const override;

    muse::io::path_t defaultStyleFilePath() const override;
    void setDefaultStyleFilePath(const muse::io::path_t& path)  override;

    muse::io::path_t partStyleFilePath() const override;
    void setPartStyleFilePath(const muse::io::path_t& path)  override;

    bool isMidiInputEnabled() const override;
    void setIsMidiInputEnabled(bool enabled)  override;

    bool isAutomaticallyPanEnabled() const override;
    void setIsAutomaticallyPanEnabled(bool enabled)  override;

    bool isPlayRepeatsEnabled() const override;
    void setIsPlayRepeatsEnabled(bool enabled)  override;
    muse::async::Notification isPlayRepeatsChanged() const override;

    bool isPlayChordSymbolsEnabled() const override;
    void setIsPlayChordSymbolsEnabled(bool enabled)  override;
    muse::async::Notification isPlayChordSymbolsChanged() const override;

    bool isMetronomeEnabled() const override;
    void setIsMetronomeEnabled(bool enabled)  override;

    bool isCountInEnabled() const override;
    void setIsCountInEnabled(bool enabled)  override;

    double guiScaling() const override;
    double notationScaling() const override;

    ValCh<muse::Orientation> canvasOrientation() const override;
    void setCanvasOrientation(muse::Orientation orientation)  override;

    bool isLimitCanvasScrollArea() const override;
    void setIsLimitCanvasScrollArea(bool limited)  override;
    muse::async::Notification isLimitCanvasScrollAreaChanged() const override;

    bool colorNotesOutsideOfUsablePitchRange() const override;
    void setColorNotesOutsideOfUsablePitchRange(bool value)  override;

    int delayBetweenNotesInRealTimeModeMilliseconds() const override;
    void setDelayBetweenNotesInRealTimeModeMilliseconds(int delayMs)  override;

    int notePlayDurationMilliseconds() const override;
    void setNotePlayDurationMilliseconds(int durationMs)  override;

    void setTemplateModeEnabled(std::optional<bool> enabled) override;
    void setTestModeEnabled(std::optional<bool> enabled) override;

    muse::io::path_t instrumentListPath() const override;

    io::paths_t scoreOrderListPaths() const override;
    muse::async::Notification scoreOrderListPathsChanged() const override;

    io::paths_t userScoreOrderListPaths() const override;
    void setUserScoreOrderListPaths(const io::paths_t& paths)  override;

    bool isSnappedToGrid(muse::Orientation gridOrientation) const override;
    void setIsSnappedToGrid(muse::Orientation gridOrientation, bool isSnapped)  override;

    int gridSizeSpatium(muse::Orientation gridOrientation) const override;
    void setGridSize(muse::Orientation gridOrientation, int sizeSpatium)  override;

    bool needToShowAddTextErrorMessage() const override;
    void setNeedToShowAddTextErrorMessage(bool show)  override;

    bool needToShowAddFiguredBassErrorMessage() const override;
    void setNeedToShowAddFiguredBassErrorMessage(bool show)  override;

    bool needToShowMScoreError(const std::string& errorKey) const override;
    void setNeedToShowMScoreError(const std::string& errorKey, bool show)  override;

    ValCh<int> pianoKeyboardNumberOfKeys() const override;
    void setPianoKeyboardNumberOfKeys(int number)  override;

    ValCh<bool> midiUseWrittenPitch() const override;
    void setMidiUseWrittenPitch(bool value)  override;

    bool useNewPercussionPanel() const override;
    void setUseNewPercussionPanel(bool use) override;
    muse::async::Notification useNewPercussionPanelChanged() const override;

    bool autoShowPercussionPanel() const override;
    void setAutoShowPercussionPanel(bool autoShow) override;
    muse::async::Notification autoShowPercussionPanelChanged() const override;

    bool showPercussionPanelPadSwapDialog() const override;
    void setShowPercussionPanelPadSwapDialog(bool show);
    muse::async::Notification showPercussionPanelPadSwapDialogChanged() const override;

    bool percussionPanelMoveMidiNotesAndShortcuts() const override;
    void setPercussionPanelMoveMidiNotesAndShortcuts(bool move);
    muse::async::Notification percussionPanelMoveMidiNotesAndShortcutsChanged() const override;

    muse::io::path_t styleFileImportPath() const override;
    void setStyleFileImportPath(const muse::io::path_t& path)  override;
};
}

#endif // MU_NOTATION_NOTATIONCONFIGURATIONSTUB_H
