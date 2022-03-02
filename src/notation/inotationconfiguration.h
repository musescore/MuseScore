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
#ifndef MU_NOTATION_INOTATIONCONFIGURATION_H
#define MU_NOTATION_INOTATIONCONFIGURATION_H

#include <QColor>

#include "modularity/imoduleexport.h"
#include "async/channel.h"
#include "retval.h"
#include "io/path.h"
#include "notationtypes.h"
#include "global/globaltypes.h"

namespace mu::notation {
class INotationConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(INotationConfiguration)

public:
    virtual ~INotationConfiguration() = default;

    virtual QColor backgroundColor() const = 0;
    virtual void setBackgroundColor(const QColor& color) = 0;
    virtual void resetCurrentBackgroundColorToDefault() = 0;

    virtual io::path backgroundWallpaperPath() const = 0;
    virtual void setBackgroundWallpaperPath(const io::path& path) = 0;

    virtual bool backgroundUseColor() const = 0;
    virtual void setBackgroundUseColor(bool value) = 0;
    virtual async::Notification backgroundChanged() const = 0;

    virtual QColor foregroundColor() const = 0;
    virtual void setForegroundColor(const QColor& color) = 0;

    virtual io::path foregroundWallpaperPath() const = 0;
    virtual void setForegroundWallpaperPath(const io::path& path) = 0;

    virtual bool foregroundUseColor() const = 0;
    virtual void setForegroundUseColor(bool value) = 0;
    virtual async::Notification foregroundChanged() const = 0;

    virtual io::path wallpapersDefaultDirPath() const = 0;

    virtual QColor borderColor() const = 0;
    virtual int borderWidth() const = 0;

    virtual QColor anchorLineColor() const = 0;

    virtual QColor playbackCursorColor() const = 0;
    virtual QColor loopMarkerColor() const = 0;
    virtual int cursorOpacity() const = 0;

    virtual QColor selectionColor(int voiceIndex = 0) const = 0;

    virtual int selectionProximity() const = 0;
    virtual void setSelectionProximity(int proxymity) = 0;

    virtual ZoomType defaultZoomType() const = 0;
    virtual void setDefaultZoomType(ZoomType zoomType) = 0;

    virtual int defaultZoom() const = 0;
    virtual void setDefaultZoom(int zoomPercentage) = 0;

    virtual ValCh<int> currentZoom() const = 0;
    virtual void setCurrentZoom(int zoomPercentage) = 0;

    virtual QList<int> possibleZoomPercentageList() const = 0;

    virtual int mouseZoomPrecision() const = 0;
    virtual void setMouseZoomPrecision(int precision) = 0;

    virtual std::string fontFamily() const = 0;
    virtual int fontSize() const = 0;

    virtual io::path userStylesPath() const = 0;
    virtual void setUserStylesPath(const io::path& path) = 0;
    virtual async::Channel<io::path> userStylesPathChanged() const = 0;

    virtual io::path defaultStyleFilePath() const = 0;
    virtual void setDefaultStyleFilePath(const io::path& path) = 0;

    virtual io::path partStyleFilePath() const = 0;
    virtual void setPartStyleFilePath(const io::path& path) = 0;

    virtual bool isMidiInputEnabled() const = 0;
    virtual void setIsMidiInputEnabled(bool enabled) = 0;

    virtual bool isAutomaticallyPanEnabled() const = 0;
    virtual void setIsAutomaticallyPanEnabled(bool enabled) = 0;

    virtual bool isPlayRepeatsEnabled() const = 0;
    virtual void setIsPlayRepeatsEnabled(bool enabled) = 0;
    virtual async::Notification isPlayRepeatsChanged() const = 0;

    virtual bool isMetronomeEnabled() const = 0;
    virtual void setIsMetronomeEnabled(bool enabled) = 0;

    virtual bool isCountInEnabled() const = 0;
    virtual void setIsCountInEnabled(bool enabled) = 0;

    virtual double guiScaling() const = 0;
    virtual double notationScaling() const = 0;

    virtual std::string notationRevision() const = 0;
    virtual int notationDivision() const = 0;

    virtual ValCh<framework::Orientation> canvasOrientation() const = 0;
    virtual void setCanvasOrientation(framework::Orientation orientation) = 0;

    virtual bool isLimitCanvasScrollArea() const = 0;
    virtual void setIsLimitCanvasScrollArea(bool limited) = 0;
    virtual async::Notification isLimitCanvasScrollAreaChanged() const = 0;

    virtual bool colorNotesOusideOfUsablePitchRange() const = 0;
    virtual void setColorNotesOusideOfUsablePitchRange(bool value) = 0;

    virtual int delayBetweenNotesInRealTimeModeMilliseconds() const = 0;
    virtual void setDelayBetweenNotesInRealTimeModeMilliseconds(int delayMs) = 0;

    virtual int notePlayDurationMilliseconds() const = 0;
    virtual void setNotePlayDurationMilliseconds(int durationMs) = 0;

    virtual void setTemplateModeEnalbed(bool enabled) = 0;
    virtual void setTestModeEnabled(bool enabled) = 0;

    virtual io::paths instrumentListPaths() const = 0;
    virtual async::Notification instrumentListPathsChanged() const = 0;

    virtual io::paths userInstrumentListPaths() const = 0;
    virtual void setUserInstrumentListPaths(const io::paths& paths) = 0;

    virtual io::paths scoreOrderListPaths() const = 0;
    virtual async::Notification scoreOrderListPathsChanged() const = 0;

    virtual io::paths userScoreOrderListPaths() const = 0;
    virtual void setUserScoreOrderListPaths(const io::paths& paths) = 0;

    virtual bool isSnappedToGrid(framework::Orientation gridOrientation) const = 0;
    virtual void setIsSnappedToGrid(framework::Orientation gridOrientation, bool isSnapped) = 0;

    virtual int gridSizeSpatium(framework::Orientation gridOrientation) const = 0;
    virtual void setGridSize(framework::Orientation gridOrientation, int sizeSpatium) = 0;

    virtual bool needToShowAddTextErrorMessage() const = 0;
    virtual void setNeedToShowAddTextErrorMessage(bool show) = 0;

    virtual bool needToShowAddFiguredBassErrorMessage() const = 0;
    virtual void setNeedToShowAddFiguredBassErrorMessage(bool show) = 0;

    virtual bool needToShowAddBoxesErrorMessage() const = 0;
    virtual void setNeedToShowAddBoxesErrorMessage(bool show) = 0;
};
}

#endif // MU_NOTATION_INOTATIONCONFIGURATION_H
