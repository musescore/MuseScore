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
#ifndef MU_NOTATION_NOTATIONCONFIGURATION_H
#define MU_NOTATION_NOTATIONCONFIGURATION_H

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

    muse::io::path_t userStylesPath() const override;
    void setUserStylesPath(const muse::io::path_t& path) override;
    muse::async::Channel<muse::io::path_t> userStylesPathChanged() const override;

    muse::io::path_t defaultStyleFilePath() const override;
    void setDefaultStyleFilePath(const muse::io::path_t& path) override;

    muse::io::path_t partStyleFilePath() const override;
    void setPartStyleFilePath(const muse::io::path_t& path) override;

    bool isMidiInputEnabled() const override;
    void setIsMidiInputEnabled(bool enabled) override;

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

    bool isMetronomeEnabled() const override;
    void setIsMetronomeEnabled(bool enabled) override;

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

    bool warnGuitarBends() const override;
    void setWarnGuitarBends(bool value) override;

    int delayBetweenNotesInRealTimeModeMilliseconds() const override;
    void setDelayBetweenNotesInRealTimeModeMilliseconds(int delayMs) override;

    int notePlayDurationMilliseconds() const override;
    void setNotePlayDurationMilliseconds(int durationMs) override;

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

    bool autoShowPercussionPanel() const override;
    void setAutoShowPercussionPanel(bool autoShow) override;

    muse::io::path_t styleFileImportPath() const override;
    void setStyleFileImportPath(const muse::io::path_t& path) override;

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
    muse::async::Channel<muse::Orientation> m_canvasOrientationChanged;
    muse::async::Channel<muse::io::path_t> m_userStylesPathChanged;
    muse::async::Notification m_scoreOrderListPathsChanged;
    muse::async::Notification m_isLimitCanvasScrollAreaChanged;
    muse::async::Notification m_isPlayRepeatsChanged;
    muse::async::Notification m_isPlayChordSymbolsChanged;
    muse::ValCh<int> m_pianoKeyboardNumberOfKeys;
    muse::ValCh<bool> m_midiInputUseWrittenPitch;
    muse::async::Channel<QColor> m_anchorColorChanged;

    int m_styleDialogLastPageIndex = 0;
    int m_styleDialogLastSubPageIndex = 0;
};
}

#endif // MU_NOTATION_NOTATIONCONFIGURATION_H
