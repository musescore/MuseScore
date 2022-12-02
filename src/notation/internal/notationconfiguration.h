/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_NOTATION_NOTATIONCONFIGURATION_H
#define MU_NOTATION_NOTATIONCONFIGURATION_H

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "global/iglobalconfiguration.h"
#include "io/ifilesystem.h"
#include "ui/iuiconfiguration.h"
#include "engraving/iengravingconfiguration.h"

#include "../inotationconfiguration.h"

namespace mu::notation {
class NotationConfiguration : public INotationConfiguration, public async::Asyncable
{
    INJECT(notation, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(notation, io::IFileSystem, fileSystem)
    INJECT(notation, ui::IUiConfiguration, uiConfiguration)
    INJECT(notation, engraving::IEngravingConfiguration, engravingConfiguration)

public:
    void init();

    QColor anchorLineColor() const override;

    QColor backgroundColor() const override;
    void setBackgroundColor(const QColor& color) override;

    io::path_t backgroundWallpaperPath() const override;
    const QPixmap& backgroundWallpaper() const override;
    void setBackgroundWallpaperPath(const io::path_t& path) override;

    bool backgroundUseColor() const override;
    void setBackgroundUseColor(bool value) override;

    void resetBackground() override;

    async::Notification backgroundChanged() const override;

    QColor foregroundColor() const override;
    void setForegroundColor(const QColor& color) override;

    io::path_t foregroundWallpaperPath() const override;
    const QPixmap& foregroundWallpaper() const override;
    void setForegroundWallpaperPath(const io::path_t& path) override;

    bool foregroundUseColor() const override;
    void setForegroundUseColor(bool value) override;

    void resetForeground() override;

    async::Notification foregroundChanged() const override;

    io::path_t wallpapersDefaultDirPath() const override;

    QColor borderColor() const override;
    int borderWidth() const override;

    QColor playbackCursorColor() const override;
    QColor loopMarkerColor() const override;
    int cursorOpacity() const override;

    QColor selectionColor(engraving::voice_idx_t voiceIndex = 0) const override;

    QColor dropRectColor() const override;

    int selectionProximity() const override;
    void setSelectionProximity(int proximity) override;

    ZoomType defaultZoomType() const override;
    void setDefaultZoomType(ZoomType zoomType) override;

    int defaultZoom() const override;
    void setDefaultZoom(int zoomPercentage) override;

    qreal scalingFromZoomPercentage(int zoomPercentage) const override;
    int zoomPercentageFromScaling(qreal scaling) const override;

    QList<int> possibleZoomPercentageList() const override;

    int mouseZoomPrecision() const override;
    void setMouseZoomPrecision(int precision) override;

    std::string fontFamily() const override;
    int fontSize() const override;

    io::path_t userStylesPath() const override;
    void setUserStylesPath(const io::path_t& path) override;
    async::Channel<io::path_t> userStylesPathChanged() const override;

    io::path_t defaultStyleFilePath() const override;
    void setDefaultStyleFilePath(const io::path_t& path) override;

    io::path_t partStyleFilePath() const override;
    void setPartStyleFilePath(const io::path_t& path) override;

    bool isMidiInputEnabled() const override;
    void setIsMidiInputEnabled(bool enabled) override;

    bool isAutomaticallyPanEnabled() const override;
    void setIsAutomaticallyPanEnabled(bool enabled) override;

    bool isPlayRepeatsEnabled() const override;
    void setIsPlayRepeatsEnabled(bool enabled) override;
    async::Notification isPlayRepeatsChanged() const override;

    bool isPlayChordSymbolsEnabled() const override;
    void setIsPlayChordSymbolsEnabled(bool enabled) override;
    async::Notification isPlayChordSymbolsChanged() const override;

    bool isMetronomeEnabled() const override;
    void setIsMetronomeEnabled(bool enabled) override;

    bool isCountInEnabled() const override;
    void setIsCountInEnabled(bool enabled) override;

    double guiScaling() const override;
    double notationScaling() const override;

    ValCh<framework::Orientation> canvasOrientation() const override;
    void setCanvasOrientation(framework::Orientation orientation) override;

    bool isLimitCanvasScrollArea() const override;
    void setIsLimitCanvasScrollArea(bool limited) override;
    async::Notification isLimitCanvasScrollAreaChanged() const override;

    bool colorNotesOutsideOfUsablePitchRange() const override;
    void setColorNotesOutsideOfUsablePitchRange(bool value) override;

    int delayBetweenNotesInRealTimeModeMilliseconds() const override;
    void setDelayBetweenNotesInRealTimeModeMilliseconds(int delayMs) override;

    int notePlayDurationMilliseconds() const override;
    void setNotePlayDurationMilliseconds(int durationMs) override;

    void setTemplateModeEnabled(bool enabled) override;
    void setTestModeEnabled(bool enabled) override;

    io::path_t instrumentListPath() const override;

    io::paths_t scoreOrderListPaths() const override;
    async::Notification scoreOrderListPathsChanged() const override;

    io::paths_t userScoreOrderListPaths() const override;
    void setUserScoreOrderListPaths(const io::paths_t& paths) override;

    bool isSnappedToGrid(framework::Orientation gridOrientation) const override;
    void setIsSnappedToGrid(framework::Orientation gridOrientation, bool isSnapped) override;

    int gridSizeSpatium(framework::Orientation gridOrientation) const override;
    void setGridSize(framework::Orientation gridOrientation, int sizeSpatium) override;

    bool needToShowAddTextErrorMessage() const override;
    void setNeedToShowAddTextErrorMessage(bool show) override;

    bool needToShowAddFiguredBassErrorMessage() const override;
    void setNeedToShowAddFiguredBassErrorMessage(bool show) override;

    bool needToShowMScoreError(const std::string& errorKey) const override;
    void setNeedToShowMScoreError(const std::string& errorKey, bool show) override;

    ValCh<int> pianoKeyboardNumberOfKeys() const override;
    void setPianoKeyboardNumberOfKeys(int number) override;

    io::path_t styleFileImportPath() const override;
    void setStyleFileImportPath(const io::path_t& path) override;

private:
    io::path_t firstScoreOrderListPath() const;
    void setFirstScoreOrderListPath(const io::path_t& path);

    io::path_t secondScoreOrderListPath() const;
    void setSecondScoreOrderListPath(const io::path_t& path);

    async::Notification m_backgroundChanged;
    async::Notification m_foregroundChanged;
    async::Channel<framework::Orientation> m_canvasOrientationChanged;
    async::Channel<io::path_t> m_userStylesPathChanged;
    async::Notification m_scoreOrderListPathsChanged;
    async::Notification m_isLimitCanvasScrollAreaChanged;
    async::Notification m_isPlayRepeatsChanged;
    async::Notification m_isPlayChordSymbolsChanged;
    ValCh<int> m_pianoKeyboardNumberOfKeys;
};
}

#endif // MU_NOTATION_NOTATIONCONFIGURATION_H
