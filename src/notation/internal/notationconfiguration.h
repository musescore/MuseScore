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

#include "../inotationconfiguration.h"
#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "ui/iuiconfiguration.h"
#include "iglobalconfiguration.h"
#include "settings.h"
#include "system/ifilesystem.h"
#include "engraving/iengravingconfiguration.h"

namespace mu::notation {
class NotationConfiguration : public INotationConfiguration, public async::Asyncable
{
    INJECT(notation, ui::IUiConfiguration, uiConfiguration)
    INJECT(notation, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(notation, engraving::IEngravingConfiguration, engravingConfiguration)
    INJECT(notation, system::IFileSystem, fileSystem)

public:
    void init();

    QColor anchorLineColor() const override;

    QColor backgroundColor() const override;
    void setBackgroundColor(const QColor& color) override;
    void resetCurrentBackgroundColorToDefault() override;

    io::path backgroundWallpaperPath() const override;
    void setBackgroundWallpaperPath(const io::path& path) override;

    bool backgroundUseColor() const override;
    void setBackgroundUseColor(bool value) override;

    async::Notification backgroundChanged() const override;

    QColor foregroundColor() const override;
    void setForegroundColor(const QColor& color) override;

    io::path foregroundWallpaperPath() const override;
    void setForegroundWallpaperPath(const io::path& path) override;

    bool foregroundUseColor() const override;
    void setForegroundUseColor(bool value) override;

    async::Notification foregroundChanged() const override;

    io::path wallpapersDefaultDirPath() const override;

    QColor borderColor() const override;
    int borderWidth() const override;

    QColor playbackCursorColor() const override;
    QColor loopMarkerColor() const override;
    int cursorOpacity() const override;

    QColor selectionColor(int voiceIndex = 0) const override;

    int selectionProximity() const override;
    void setSelectionProximity(int proxymity) override;

    ZoomType defaultZoomType() const override;
    void setDefaultZoomType(ZoomType zoomType) override;

    int defaultZoom() const override;
    void setDefaultZoom(int zoomPercentage) override;

    ValCh<int> currentZoom() const override;
    void setCurrentZoom(int zoomPercentage) override;

    QList<int> possibleZoomPercentageList() const override;

    int mouseZoomPrecision() const override;
    void setMouseZoomPrecision(int precision) override;

    std::string fontFamily() const override;
    int fontSize() const override;

    io::path userStylesPath() const override;
    void setUserStylesPath(const io::path& path) override;
    async::Channel<io::path> userStylesPathChanged() const override;

    io::path defaultStyleFilePath() const override;
    void setDefaultStyleFilePath(const io::path& path) override;

    io::path partStyleFilePath() const override;
    void setPartStyleFilePath(const io::path& path) override;

    bool isMidiInputEnabled() const override;
    void setIsMidiInputEnabled(bool enabled) override;

    bool isAutomaticallyPanEnabled() const override;
    void setIsAutomaticallyPanEnabled(bool enabled) override;

    bool isPlayRepeatsEnabled() const override;
    void setIsPlayRepeatsEnabled(bool enabled) override;
    async::Notification isPlayRepeatsChanged() const override;

    bool isMetronomeEnabled() const override;
    void setIsMetronomeEnabled(bool enabled) override;

    bool isCountInEnabled() const override;
    void setIsCountInEnabled(bool enabled) override;

    double guiScaling() const override;
    double notationScaling() const override;

    std::string notationRevision() const override;
    int notationDivision() const override;

    ValCh<framework::Orientation> canvasOrientation() const override;
    void setCanvasOrientation(framework::Orientation orientation) override;

    bool isLimitCanvasScrollArea() const override;
    void setIsLimitCanvasScrollArea(bool limited) override;
    async::Notification isLimitCanvasScrollAreaChanged() const override;

    bool colorNotesOusideOfUsablePitchRange() const override;
    void setColorNotesOusideOfUsablePitchRange(bool value) override;

    int delayBetweenNotesInRealTimeModeMilliseconds() const override;
    void setDelayBetweenNotesInRealTimeModeMilliseconds(int delayMs) override;

    int notePlayDurationMilliseconds() const override;
    void setNotePlayDurationMilliseconds(int durationMs) override;

    void setTemplateModeEnalbed(bool enabled) override;
    void setTestModeEnabled(bool enabled) override;

    io::paths instrumentListPaths() const override;
    async::Notification instrumentListPathsChanged() const override;

    io::paths userInstrumentListPaths() const override;
    void setUserInstrumentListPaths(const io::paths& paths) override;

    io::paths scoreOrderListPaths() const override;
    async::Notification scoreOrderListPathsChanged() const override;

    io::paths userScoreOrderListPaths() const override;
    void setUserScoreOrderListPaths(const io::paths& paths) override;

    bool isSnappedToGrid(framework::Orientation gridOrientation) const override;
    void setIsSnappedToGrid(framework::Orientation gridOrientation, bool isSnapped) override;

    int gridSizeSpatium(framework::Orientation gridOrientation) const override;
    void setGridSize(framework::Orientation gridOrientation, int sizeSpatium) override;

    bool needToShowAddTextErrorMessage() const override;
    void setNeedToShowAddTextErrorMessage(bool show) override;

    bool needToShowAddFiguredBassErrorMessage() const override;
    void setNeedToShowAddFiguredBassErrorMessage(bool show) override;

    bool needToShowAddBoxesErrorMessage() const override;
    void setNeedToShowAddBoxesErrorMessage(bool show) override;

private:
    io::path firstInstrumentListPath() const;
    void setFirstInstrumentListPath(const io::path& path);

    io::path secondInstrumentListPath() const;
    void setSecondInstrumentListPath(const io::path& path);

    io::path firstScoreOrderListPath() const;
    void setFirstScoreOrderListPath(const io::path& path);

    io::path secondScoreOrderListPath() const;
    void setSecondScoreOrderListPath(const io::path& path);

    async::Notification m_backgroundChanged;
    async::Notification m_foregroundChanged;
    ValCh<int> m_currentZoomPercentage;
    async::Channel<framework::Orientation> m_canvasOrientationChanged;
    async::Channel<io::path> m_userStylesPathChanged;
    async::Notification m_instrumentListPathsChanged;
    async::Notification m_scoreOrderListPathsChanged;
    async::Notification m_isLimitCanvasScrollAreaChanged;
    async::Notification m_isPlayRepeatsChanged;
};
}

#endif // MU_NOTATION_NOTATIONCONFIGURATION_H
