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
#include "notationconfigurationstub.h"

using namespace mu;
using namespace mu::notation;

QColor NotationConfigurationStub::notationColor() const
{
    return QColor();
}

QColor NotationConfigurationStub::backgroundColor() const
{
    return QColor();
}

void NotationConfigurationStub::setBackgroundColor(const QColor&)
{
}

muse::io::path_t NotationConfigurationStub::backgroundWallpaperPath() const
{
    return muse::io::path_t();
}

const QPixmap& NotationConfigurationStub::backgroundWallpaper() const
{
    static QPixmap p;
    return p;
}

void NotationConfigurationStub::setBackgroundWallpaperPath(const muse::io::path_t&)
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

muse::async::Notification NotationConfigurationStub::backgroundChanged() const
{
    static muse::async::Notification n;
    return n;
}

QColor NotationConfigurationStub::foregroundColor() const
{
    return QColor();
}

void NotationConfigurationStub::setForegroundColor(const QColor&)
{
}

muse::io::path_t NotationConfigurationStub::foregroundWallpaperPath() const
{
    return muse::io::path_t();
}

const QPixmap& NotationConfigurationStub::foregroundWallpaper() const
{
    static QPixmap p;
    return p;
}

void NotationConfigurationStub::setForegroundWallpaperPath(const muse::io::path_t&)
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

muse::async::Notification NotationConfigurationStub::foregroundChanged() const
{
    static muse::async::Notification n;
    return n;
}

muse::io::path_t NotationConfigurationStub::wallpapersDefaultDirPath() const
{
    return muse::io::path_t();
}

QColor NotationConfigurationStub::borderColor() const
{
    return QColor();
}

int NotationConfigurationStub::borderWidth() const
{
    return 1;
}

QColor NotationConfigurationStub::anchorColor() const
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

bool NotationConfigurationStub::thinNoteInputCursor() const
{
    return false;
}

QColor NotationConfigurationStub::selectionColor(engraving::voice_idx_t) const
{
    return QColor();
}

QColor NotationConfigurationStub::highlightSelectionColor(engraving::voice_idx_t) const
{
    return QColor();
}

QColor NotationConfigurationStub::dropRectColor() const
{
    return QColor();
}

muse::draw::Color NotationConfigurationStub::noteInputPreviewColor() const
{
    return muse::draw::Color();
}

bool NotationConfigurationStub::useNoteInputCursorInInputByDuration() const
{
    return false;
}

void NotationConfigurationStub::setUseNoteInputCursorInInputByDuration(bool)
{
}

muse::async::Notification NotationConfigurationStub::useNoteInputCursorInInputByDurationChanged() const
{
    static muse::async::Notification n;
    return n;
}

int NotationConfigurationStub::selectionProximity() const
{
    return 1;
}

void NotationConfigurationStub::setSelectionProximity(int)
{
}

muse::async::Channel<int> NotationConfigurationStub::selectionProximityChanged() const
{
    static muse::async::Channel<int> ch;
    return ch;
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

muse::async::Notification NotationConfigurationStub::defaultZoomChanged() const
{
    static muse::async::Notification n;
    return n;
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

muse::async::Notification NotationConfigurationStub::mouseZoomPrecisionChanged() const
{
    static muse::async::Notification n;
    return n;
}

std::string NotationConfigurationStub::fontFamily() const
{
    return std::string();
}

int NotationConfigurationStub::fontSize() const
{
    return 20;
}

muse::io::path_t NotationConfigurationStub::userStylesPath() const
{
    return muse::io::path_t();
}

void NotationConfigurationStub::setUserStylesPath(const muse::io::path_t&)
{
}

muse::async::Channel<muse::io::path_t> NotationConfigurationStub::userStylesPathChanged() const
{
    static muse::async::Channel<muse::io::path_t> ch;
    return ch;
}

muse::io::path_t NotationConfigurationStub::defaultStyleFilePath() const
{
    return muse::io::path_t();
}

void NotationConfigurationStub::setDefaultStyleFilePath(const muse::io::path_t&)
{
}

muse::async::Channel<muse::io::path_t> NotationConfigurationStub::defaultStyleFilePathChanged() const
{
    static muse::async::Channel<muse::io::path_t> ch;
    return ch;
}

muse::io::path_t NotationConfigurationStub::partStyleFilePath() const
{
    return muse::io::path_t();
}

void NotationConfigurationStub::setPartStyleFilePath(const muse::io::path_t&)
{
}

muse::async::Channel<muse::io::path_t> NotationConfigurationStub::partStyleFilePathChanged() const
{
    static muse::async::Channel<muse::io::path_t> ch;
    return ch;
}

NoteInputMethod NotationConfigurationStub::defaultNoteInputMethod() const
{
    return NoteInputMethod::UNKNOWN;
}

void NotationConfigurationStub::setDefaultNoteInputMethod(NoteInputMethod)
{
}

muse::async::Notification NotationConfigurationStub::defaultNoteInputMethodChanged() const
{
    static muse::async::Notification n;
    return n;
}

bool NotationConfigurationStub::addAccidentalDotsArticulationsToNextNoteEntered() const
{
    return false;
}

void NotationConfigurationStub::setAddAccidentalDotsArticulationsToNextNoteEntered(bool)
{
}

muse::async::Notification NotationConfigurationStub::addAccidentalDotsArticulationsToNextNoteEnteredChanged() const
{
    static muse::async::Notification n;
    return n;
}

bool NotationConfigurationStub::isMidiInputEnabled() const
{
    return false;
}

void NotationConfigurationStub::setIsMidiInputEnabled(bool)
{
}

muse::async::Notification NotationConfigurationStub::isMidiInputEnabledChanged() const
{
    static muse::async::Notification n;
    return n;
}

bool NotationConfigurationStub::startNoteInputAtSelectedNoteRestWhenPressingMidiKey() const
{
    return false;
}

void NotationConfigurationStub::setStartNoteInputAtSelectedNoteRestWhenPressingMidiKey(bool)
{
}

muse::async::Notification NotationConfigurationStub::startNoteInputAtSelectedNoteRestWhenPressingMidiKeyChanged() const
{
    static muse::async::Notification n;
    return n;
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

muse::async::Notification NotationConfigurationStub::isPlayRepeatsChanged() const
{
    static muse::async::Notification n;
    return n;
}

bool NotationConfigurationStub::isPlayChordSymbolsEnabled() const
{
    return false;
}

void NotationConfigurationStub::setIsPlayChordSymbolsEnabled(bool)
{
}

muse::async::Notification NotationConfigurationStub::isPlayChordSymbolsChanged() const
{
    static muse::async::Notification n;
    return n;
}

bool NotationConfigurationStub::isPlayPreviewNotesInInputByDuration() const
{
    return false;
}

void NotationConfigurationStub::setIsPlayPreviewNotesInInputByDuration(bool)
{
}

muse::async::Notification NotationConfigurationStub::isPlayPreviewNotesInInputByDurationChanged() const
{
    static muse::async::Notification n;
    return n;
}

bool NotationConfigurationStub::isMetronomeEnabled() const
{
    return false;
}

void NotationConfigurationStub::setIsMetronomeEnabled(bool)
{
}

muse::async::Notification NotationConfigurationStub::isMetronomeEnabledChanged() const
{
    static muse::async::Notification n;
    return n;
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

ValCh<muse::Orientation> NotationConfigurationStub::canvasOrientation() const
{
    static ValCh<muse::Orientation> vch;
    return vch;
}

void NotationConfigurationStub::setCanvasOrientation(muse::Orientation)
{
}

bool NotationConfigurationStub::isLimitCanvasScrollArea() const
{
    return true;
}

void NotationConfigurationStub::setIsLimitCanvasScrollArea(bool)
{
}

muse::async::Notification NotationConfigurationStub::isLimitCanvasScrollAreaChanged() const
{
    static muse::async::Notification n;
    return n;
}

bool NotationConfigurationStub::colorNotesOutsideOfUsablePitchRange() const
{
    return false;
}

void NotationConfigurationStub::setColorNotesOutsideOfUsablePitchRange(bool)
{
}

muse::async::Channel<bool> NotationConfigurationStub::colorNotesOutsideOfUsablePitchRangeChanged() const
{
    static muse::async::Channel<bool> ch;
    return ch;
}

int NotationConfigurationStub::delayBetweenNotesInRealTimeModeMilliseconds() const
{
    return 100;
}

void NotationConfigurationStub::setDelayBetweenNotesInRealTimeModeMilliseconds(int)
{
}

muse::async::Channel<int> NotationConfigurationStub::delayBetweenNotesInRealTimeModeMillisecondsChanged() const
{
    static muse::async::Channel<int> ch;
    return ch;
}

int NotationConfigurationStub::notePlayDurationMilliseconds() const
{
    return 100;
}

void NotationConfigurationStub::setNotePlayDurationMilliseconds(int)
{
}

muse::async::Channel<int> NotationConfigurationStub::notePlayDurationMillisecondsChanged() const
{
    static muse::async::Channel<int> ch;
    return ch;
}

void NotationConfigurationStub::setTemplateModeEnabled(std::optional<bool>)
{
}

void NotationConfigurationStub::setTestModeEnabled(std::optional<bool>)
{
}

muse::io::path_t NotationConfigurationStub::instrumentListPath() const
{
    return muse::io::path_t();
}

io::paths_t NotationConfigurationStub::scoreOrderListPaths() const
{
    return io::paths_t();
}

muse::async::Notification NotationConfigurationStub::scoreOrderListPathsChanged() const
{
    static muse::async::Notification n;
    return n;
}

io::paths_t NotationConfigurationStub::userScoreOrderListPaths() const
{
    return io::paths_t();
}

void NotationConfigurationStub::setUserScoreOrderListPaths(const io::paths_t&)
{
}

bool NotationConfigurationStub::isSnappedToGrid(muse::Orientation) const
{
    return false;
}

void NotationConfigurationStub::setIsSnappedToGrid(muse::Orientation, bool)
{
}

int NotationConfigurationStub::gridSizeSpatium(muse::Orientation) const
{
    return 20;
}

void NotationConfigurationStub::setGridSize(muse::Orientation, int)
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

ValCh<bool> NotationConfigurationStub::midiUseWrittenPitch() const
{
    static ValCh<bool> vch;
    return vch;
}

void NotationConfigurationStub::setMidiUseWrittenPitch(bool)
{
}

bool NotationConfigurationStub::useNewPercussionPanel() const
{
    return false;
}

void NotationConfigurationStub::setUseNewPercussionPanel(bool)
{
}

muse::async::Notification NotationConfigurationStub::useNewPercussionPanelChanged() const
{
    static muse::async::Notification n;
    return n;
}

bool NotationConfigurationStub::percussionPanelUseNotationPreview() const
{
    return false;
}

void NotationConfigurationStub::setPercussionPanelUseNotationPreview(bool)
{
}

muse::async::Notification NotationConfigurationStub::percussionPanelUseNotationPreviewChanged() const
{
    static muse::async::Notification n;
    return n;
}

bool NotationConfigurationStub::autoShowPercussionPanel() const
{
    return true;
}

void NotationConfigurationStub::setAutoShowPercussionPanel(bool)
{
}

muse::async::Notification NotationConfigurationStub::autoShowPercussionPanelChanged() const
{
    static muse::async::Notification n;
    return n;
}

bool NotationConfigurationStub::showPercussionPanelPadSwapDialog() const
{
    return true;
}

void NotationConfigurationStub::setShowPercussionPanelPadSwapDialog(bool)
{
}

muse::async::Notification NotationConfigurationStub::showPercussionPanelPadSwapDialogChanged() const
{
    static muse::async::Notification n;
    return n;
}

bool NotationConfigurationStub::percussionPanelMoveMidiNotesAndShortcuts() const
{
    return true;
}

void NotationConfigurationStub::setPercussionPanelMoveMidiNotesAndShortcuts(bool)
{
}

muse::async::Notification NotationConfigurationStub::percussionPanelMoveMidiNotesAndShortcutsChanged() const
{
    static muse::async::Notification n;
    return n;
}

muse::io::path_t NotationConfigurationStub::styleFileImportPath() const
{
    return muse::io::path_t();
}

void NotationConfigurationStub::setStyleFileImportPath(const muse::io::path_t&)
{
}

muse::async::Channel<std::string> NotationConfigurationStub::styleFileImportPathChanged() const
{
    static muse::async::Channel<std::string> ch;
    return ch;
}
