/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#ifndef MU_NOTATION_NOTATIONCONFIGURATIONMOCK_H
#define MU_NOTATION_NOTATIONCONFIGURATIONMOCK_H

#include <gmock/gmock.h>

#include "notation/inotationconfiguration.h"

namespace mu::notation {
class NotationConfigurationMock : public INotationConfiguration
{
public:
    MOCK_METHOD(QColor, backgroundColor, (), (const, override));
    MOCK_METHOD(void, setBackgroundColor, (const QColor& color), (override));

    MOCK_METHOD(io::path_t, backgroundWallpaperPath, (), (const, override));
    MOCK_METHOD(const QPixmap&, backgroundWallpaper, (), (const, override));
    MOCK_METHOD(void, setBackgroundWallpaperPath, (const io::path_t& path), (override));

    MOCK_METHOD(bool, backgroundUseColor, (), (const, override));
    MOCK_METHOD(void, setBackgroundUseColor, (bool), (override));

    MOCK_METHOD(void, resetBackground, (), (override));

    MOCK_METHOD(async::Notification, backgroundChanged, (), (const, override));

    MOCK_METHOD(QColor, foregroundColor, (), (const, override));
    MOCK_METHOD(void, setForegroundColor, (const QColor&), (override));

    MOCK_METHOD(io::path_t, foregroundWallpaperPath, (), (const, override));
    MOCK_METHOD(const QPixmap&, foregroundWallpaper, (), (const, override));
    MOCK_METHOD(void, setForegroundWallpaperPath, (const io::path_t&), (override));

    MOCK_METHOD(bool, foregroundUseColor, (), (const, override));
    MOCK_METHOD(void, setForegroundUseColor, (bool), (override));

    MOCK_METHOD(void, resetForeground, (), (override));

    MOCK_METHOD(async::Notification, foregroundChanged, (), (const, override));

    MOCK_METHOD(io::path_t, wallpapersDefaultDirPath, (), (const, override));

    MOCK_METHOD(QColor, borderColor, (), (const, override));
    MOCK_METHOD(int, borderWidth, (), (const, override));

    MOCK_METHOD(QColor, anchorLineColor, (), (const, override));

    MOCK_METHOD(QColor, playbackCursorColor, (), (const, override));
    MOCK_METHOD(QColor, loopMarkerColor, (), (const, override));
    MOCK_METHOD(int, cursorOpacity, (), (const, override));

    MOCK_METHOD(QColor, selectionColor, (engraving::voice_idx_t), (const, override));

    MOCK_METHOD(QColor, dropRectColor, (), (const, override));

    MOCK_METHOD(int, selectionProximity, (), (const, override));
    MOCK_METHOD(void, setSelectionProximity, (int), (override));

    MOCK_METHOD(ZoomType, defaultZoomType, (), (const, override));
    MOCK_METHOD(void, setDefaultZoomType, (ZoomType), (override));

    MOCK_METHOD(int, defaultZoom, (), (const, override));
    MOCK_METHOD(void, setDefaultZoom, (int), (override));

    MOCK_METHOD(QList<int>, possibleZoomPercentageList, (), (const, override));

    MOCK_METHOD(qreal, scalingFromZoomPercentage, (int), (const, override));
    MOCK_METHOD(int, zoomPercentageFromScaling, (qreal), (const, override));

    MOCK_METHOD(int, mouseZoomPrecision, (), (const, override));
    MOCK_METHOD(void, setMouseZoomPrecision, (int), (override));

    MOCK_METHOD(std::string, fontFamily, (), (const, override));
    MOCK_METHOD(int, fontSize, (), (const, override));

    MOCK_METHOD(io::path_t, userStylesPath, (), (const, override));
    MOCK_METHOD(void, setUserStylesPath, (const io::path_t&), (override));
    MOCK_METHOD(async::Channel<io::path_t>, userStylesPathChanged, (), (const, override));

    MOCK_METHOD(io::path_t, defaultStyleFilePath, (), (const, override));
    MOCK_METHOD(void, setDefaultStyleFilePath, (const io::path_t&), (override));

    MOCK_METHOD(io::path_t, partStyleFilePath, (), (const, override));
    MOCK_METHOD(void, setPartStyleFilePath, (const io::path_t&), (override));

    MOCK_METHOD(bool, isMidiInputEnabled, (), (const, override));
    MOCK_METHOD(void, setIsMidiInputEnabled, (bool), (override));

    MOCK_METHOD(bool, isAutomaticallyPanEnabled, (), (const, override));
    MOCK_METHOD(void, setIsAutomaticallyPanEnabled, (bool), (override));

    MOCK_METHOD(bool, isSmoothPanning, (), (const, override));
    MOCK_METHOD(void, setIsSmoothPanning, (bool), (override));

    MOCK_METHOD(bool, isPlayRepeatsEnabled, (), (const, override));
    MOCK_METHOD(void, setIsPlayRepeatsEnabled, (bool), (override));
    MOCK_METHOD(async::Notification, isPlayRepeatsChanged, (), (const, override));

    MOCK_METHOD(bool, isPlayChordSymbolsEnabled, (), (const, override));
    MOCK_METHOD(void, setIsPlayChordSymbolsEnabled, (bool), (override));
    MOCK_METHOD(async::Notification, isPlayChordSymbolsChanged, (), (const, override));

    MOCK_METHOD(bool, isMetronomeEnabled, (), (const, override));
    MOCK_METHOD(void, setIsMetronomeEnabled, (bool), (override));

    MOCK_METHOD(bool, isCountInEnabled, (), (const, override));
    MOCK_METHOD(void, setIsCountInEnabled, (bool), (override));

    MOCK_METHOD(double, guiScaling, (), (const, override));
    MOCK_METHOD(double, notationScaling, (), (const, override));

    MOCK_METHOD(ValCh<framework::Orientation>, canvasOrientation, (), (const, override));
    MOCK_METHOD(void, setCanvasOrientation, (framework::Orientation), (override));

    MOCK_METHOD(bool, isLimitCanvasScrollArea, (), (const, override));
    MOCK_METHOD(void, setIsLimitCanvasScrollArea, (bool), (override));
    MOCK_METHOD(async::Notification, isLimitCanvasScrollAreaChanged, (), (const, override));

    MOCK_METHOD(bool, colorNotesOutsideOfUsablePitchRange, (), (const, override));
    MOCK_METHOD(void, setColorNotesOutsideOfUsablePitchRange, (bool), (override));

    MOCK_METHOD(bool, warnGuitarBends, (), (const, override));
    MOCK_METHOD(void, setWarnGuitarBends, (bool), (override));

    MOCK_METHOD(int, delayBetweenNotesInRealTimeModeMilliseconds, (), (const, override));
    MOCK_METHOD(void, setDelayBetweenNotesInRealTimeModeMilliseconds, (int), (override));

    MOCK_METHOD(int, notePlayDurationMilliseconds, (), (const, override));
    MOCK_METHOD(void, setNotePlayDurationMilliseconds, (int), (override));

    MOCK_METHOD(void, setTemplateModeEnabled, (std::optional<bool>), (override));
    MOCK_METHOD(void, setTestModeEnabled, (std::optional<bool>), (override));

    MOCK_METHOD(io::path_t, instrumentListPath, (), (const, override));

    MOCK_METHOD(io::paths_t, scoreOrderListPaths, (), (const, override));
    MOCK_METHOD(async::Notification, scoreOrderListPathsChanged, (), (const, override));

    MOCK_METHOD(io::paths_t, userScoreOrderListPaths, (), (const, override));
    MOCK_METHOD(void, setUserScoreOrderListPaths, (const io::paths_t&), (override));

    MOCK_METHOD(io::path_t, stringTuningsPresetsPath, (), (const, override));

    MOCK_METHOD(bool, isSnappedToGrid, (framework::Orientation), (const, override));
    MOCK_METHOD(void, setIsSnappedToGrid, (framework::Orientation, bool), (override));

    MOCK_METHOD(int, gridSizeSpatium, (framework::Orientation), (const, override));
    MOCK_METHOD(void, setGridSize, (framework::Orientation, int), (override));

    MOCK_METHOD(bool, needToShowAddTextErrorMessage, (), (const, override));
    MOCK_METHOD(void, setNeedToShowAddTextErrorMessage, (bool), (override));

    MOCK_METHOD(bool, needToShowAddFiguredBassErrorMessage, (), (const, override));
    MOCK_METHOD(void, setNeedToShowAddFiguredBassErrorMessage, (bool), (override));

    MOCK_METHOD(bool, needToShowAddGuitarBendErrorMessage, (), (const, override));
    MOCK_METHOD(void, setNeedToShowAddGuitarBendErrorMessage, (bool), (override));

    MOCK_METHOD(bool, needToShowMScoreError, (const std::string&), (const, override));
    MOCK_METHOD(void, setNeedToShowMScoreError, (const std::string&, bool), (override));

    MOCK_METHOD(ValCh<int>, pianoKeyboardNumberOfKeys, (), (const, override));
    MOCK_METHOD(void, setPianoKeyboardNumberOfKeys, (int), (override));

    MOCK_METHOD(io::path_t, styleFileImportPath, (), (const, override));
    MOCK_METHOD(void, setStyleFileImportPath, (const io::path_t&), (override));
};
}

#endif // MU_NOTATION_NOTATIONCONFIGURATIONMOCK_H
