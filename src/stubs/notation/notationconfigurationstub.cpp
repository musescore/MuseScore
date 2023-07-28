/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "notationconfigurationstub.h"

using namespace mu;
using namespace mu::notation;

QColor NotationConfigurationStub::backgroundColor() const
{
    return QColor();
}

void NotationConfigurationStub::setBackgroundColor(const QColor&)
{
}

io::path_t NotationConfigurationStub::backgroundWallpaperPath() const
{
    return io::path_t();
}

const QPixmap& NotationConfigurationStub::backgroundWallpaper() const
{
    static QPixmap p;
    return p;
}

void NotationConfigurationStub::setBackgroundWallpaperPath(const io::path_t&)
{
}

bool NotationConfigurationStub::backgroundUseColor() const
{
    return false;
}

void NotationConfigurationStub::setBackgroundUseColor(bool)
{
}

void NotationConfigurationStub::resetBackground()
{
}

async::Notification NotationConfigurationStub::backgroundChanged() const
{
    static async::Notification n;
    return n;
}

QColor NotationConfigurationStub::foregroundColor() const
{
    return QColor();
}

void NotationConfigurationStub::setForegroundColor(const QColor&)
{
}

io::path_t NotationConfigurationStub::foregroundWallpaperPath() const
{
    return io::path_t();
}

const QPixmap& NotationConfigurationStub::foregroundWallpaper() const
{
    static QPixmap p;
    return p;
}

void NotationConfigurationStub::setForegroundWallpaperPath(const io::path_t&)
{
}

bool NotationConfigurationStub::foregroundUseColor() const
{
    return false;
}

void NotationConfigurationStub::setForegroundUseColor(bool)
{
}

void NotationConfigurationStub::resetForeground()
{
}

async::Notification NotationConfigurationStub::foregroundChanged() const
{
    static async::Notification n;
    return n;
}

io::path_t NotationConfigurationStub::wallpapersDefaultDirPath() const
{
    return io::path_t();
}

QColor NotationConfigurationStub::borderColor() const
{
    return QColor();
}

int NotationConfigurationStub::borderWidth() const
{
    return 1;
}

QColor NotationConfigurationStub::anchorLineColor() const
{
    return QColor();
}

QColor NotationConfigurationStub::playbackCursorColor() const
{
    return QColor();
}

QColor NotationConfigurationStub::loopMarkerColor() const
{
    return QColor();
}

int NotationConfigurationStub::cursorOpacity() const
{
    return 1;
}

QColor NotationConfigurationStub::selectionColor(engraving::voice_idx_t) const
{
    return QColor();
}

QColor NotationConfigurationStub::dropRectColor() const
{
    return QColor();
}

int NotationConfigurationStub::selectionProximity() const
{
    return 1;
}

void NotationConfigurationStub::setSelectionProximity(int)
{
}

ZoomType NotationConfigurationStub::defaultZoomType() const
{
    return ZoomType::PageWidth;
}

void NotationConfigurationStub::setDefaultZoomType(ZoomType)
{
}

int NotationConfigurationStub::defaultZoom() const
{
    return 100;
}

void NotationConfigurationStub::setDefaultZoom(int)
{
}

QList<int> NotationConfigurationStub::possibleZoomPercentageList() const
{
    return QList<int>();
}

qreal NotationConfigurationStub::scalingFromZoomPercentage(int) const
{
    return 0.0;
}

int NotationConfigurationStub::zoomPercentageFromScaling(qreal) const
{
    return 1;
}

int NotationConfigurationStub::mouseZoomPrecision() const
{
    return 1;
}

void NotationConfigurationStub::setMouseZoomPrecision(int)
{
}

std::string NotationConfigurationStub::fontFamily() const
{
    return std::string();
}

int NotationConfigurationStub::fontSize() const
{
    return 20;
}

io::path_t NotationConfigurationStub::userStylesPath() const
{
    return io::path_t();
}

void NotationConfigurationStub::setUserStylesPath(const io::path_t&)
{
}

async::Channel<io::path_t> NotationConfigurationStub::userStylesPathChanged() const
{
    static async::Channel<io::path_t> ch;
    return ch;
}

io::path_t NotationConfigurationStub::defaultStyleFilePath() const
{
    return io::path_t();
}

void NotationConfigurationStub::setDefaultStyleFilePath(const io::path_t&)
{
}

io::path_t NotationConfigurationStub::partStyleFilePath() const
{
    return io::path_t();
}

void NotationConfigurationStub::setPartStyleFilePath(const io::path_t&)
{
}

bool NotationConfigurationStub::isMidiInputEnabled() const
{
    return false;
}

void NotationConfigurationStub::setIsMidiInputEnabled(bool)
{
}

bool NotationConfigurationStub::isAutomaticallyPanEnabled() const
{
    return false;
}

void NotationConfigurationStub::setIsAutomaticallyPanEnabled(bool)
{
}

bool NotationConfigurationStub::isPlayRepeatsEnabled() const
{
    return false;
}

void NotationConfigurationStub::setIsPlayRepeatsEnabled(bool)
{
}

async::Notification NotationConfigurationStub::isPlayRepeatsChanged() const
{
    static async::Notification n;
    return n;
}

bool NotationConfigurationStub::isPlayChordSymbolsEnabled() const
{
    return false;
}

void NotationConfigurationStub::setIsPlayChordSymbolsEnabled(bool)
{
}

async::Notification NotationConfigurationStub::isPlayChordSymbolsChanged() const
{
    static async::Notification n;
    return n;
}

bool NotationConfigurationStub::isMetronomeEnabled() const
{
    return false;
}

void NotationConfigurationStub::setIsMetronomeEnabled(bool)
{
}

bool NotationConfigurationStub::isCountInEnabled() const
{
    return false;
}

void NotationConfigurationStub::setIsCountInEnabled(bool)
{
}

double NotationConfigurationStub::guiScaling() const
{
    return 1.0;
}

double NotationConfigurationStub::notationScaling() const
{
    return 1.0;
}

ValCh<framework::Orientation> NotationConfigurationStub::canvasOrientation() const
{
    static ValCh<framework::Orientation> vch;
    return vch;
}

void NotationConfigurationStub::setCanvasOrientation(framework::Orientation)
{
}

bool NotationConfigurationStub::isLimitCanvasScrollArea() const
{
    return true;
}

void NotationConfigurationStub::setIsLimitCanvasScrollArea(bool)
{
}

async::Notification NotationConfigurationStub::isLimitCanvasScrollAreaChanged() const
{
    static async::Notification n;
    return n;
}

bool NotationConfigurationStub::colorNotesOutsideOfUsablePitchRange() const
{
    return false;
}

void NotationConfigurationStub::setColorNotesOutsideOfUsablePitchRange(bool)
{
}

int NotationConfigurationStub::delayBetweenNotesInRealTimeModeMilliseconds() const
{
    return 100;
}

void NotationConfigurationStub::setDelayBetweenNotesInRealTimeModeMilliseconds(int)
{
}

int NotationConfigurationStub::notePlayDurationMilliseconds() const
{
    return 100;
}

void NotationConfigurationStub::setNotePlayDurationMilliseconds(int)
{
}

void NotationConfigurationStub::setTemplateModeEnabled(std::optional<bool>)
{
}

void NotationConfigurationStub::setTestModeEnabled(std::optional<bool>)
{
}

io::path_t NotationConfigurationStub::instrumentListPath() const
{
    return io::path_t();
}

io::paths_t NotationConfigurationStub::scoreOrderListPaths() const
{
    return io::paths_t();
}

async::Notification NotationConfigurationStub::scoreOrderListPathsChanged() const
{
    static async::Notification n;
    return n;
}

io::paths_t NotationConfigurationStub::userScoreOrderListPaths() const
{
    return io::paths_t();
}

void NotationConfigurationStub::setUserScoreOrderListPaths(const io::paths_t&)
{
}

bool NotationConfigurationStub::isSnappedToGrid(framework::Orientation) const
{
    return false;
}

void NotationConfigurationStub::setIsSnappedToGrid(framework::Orientation, bool)
{
}

int NotationConfigurationStub::gridSizeSpatium(framework::Orientation) const
{
    return 20;
}

void NotationConfigurationStub::setGridSize(framework::Orientation, int)
{
}

bool NotationConfigurationStub::needToShowAddTextErrorMessage() const
{
    return false;
}

void NotationConfigurationStub::setNeedToShowAddTextErrorMessage(bool)
{
}

bool NotationConfigurationStub::needToShowAddFiguredBassErrorMessage() const
{
    return false;
}

void NotationConfigurationStub::setNeedToShowAddFiguredBassErrorMessage(bool)
{
}

bool NotationConfigurationStub::needToShowMScoreError(const std::string&) const
{
    return false;
}

void NotationConfigurationStub::setNeedToShowMScoreError(const std::string&, bool)
{
}

ValCh<int> NotationConfigurationStub::pianoKeyboardNumberOfKeys() const
{
    static ValCh<int> vch;
    return vch;
}

void NotationConfigurationStub::setPianoKeyboardNumberOfKeys(int)
{
}

io::path_t NotationConfigurationStub::styleFileImportPath() const
{
    return io::path_t();
}

void NotationConfigurationStub::setStyleFileImportPath(const io::path_t&)
{
}
