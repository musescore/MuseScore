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
#include "notationconfiguration.h"

#include "libmscore/preferences.h"
#include "libmscore/mscore.h"

#include "log.h"
#include "settings.h"
#include "io/path.h"

#include "notationtypes.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::framework;
using namespace mu::async;
using namespace mu::ui;

// Global variable
namespace Ms {
QString revision;
}
// -----

static const std::string module_name("notation");

static const Settings::Key BACKGROUND_COLOR(module_name, "ui/canvas/background/color");
static const Settings::Key BACKGROUND_WALLPAPER_PATH(module_name, "ui/canvas/background/wallpaper");
static const Settings::Key BACKGROUND_USE_COLOR(module_name, "ui/canvas/background/useColor");

static const Settings::Key FOREGROUND_COLOR(module_name, "ui/canvas/foreground/color");
static const Settings::Key FOREGROUND_WALLPAPER_PATH(module_name, "ui/canvas/foreground/wallpaper");
static const Settings::Key FOREGROUND_USE_COLOR(module_name, "ui/canvas/foreground/useColor");

static const Settings::Key SELECTION_PROXIMITY(module_name, "ui/canvas/misc/selectionProximity");

static const Settings::Key DEFAULT_ZOOM_TYPE(module_name, "ui/canvas/zoomDefaultType");
static const Settings::Key DEFAULT_ZOOM(module_name, "ui/canvas/zoomDefaultLevel");
static const Settings::Key CURRENT_ZOOM(module_name, "ui/canvas/misc/currentZoom");
static const Settings::Key KEYBOARD_ZOOM_PRECISION(module_name, "ui/canvas/zoomPrecisionKeyboard");
static const Settings::Key MOUSE_ZOOM_PRECISION(module_name, "ui/canvas/zoomPrecisionMouse");

static const Settings::Key USER_STYLES_PATH(module_name, "application/paths/myStyles");

static const Settings::Key IS_MIDI_INPUT_ENABLED(module_name, "io/midi/enableInput");
static const Settings::Key IS_AUTOMATICALLY_PAN_ENABLED(module_name, "application/playback/panPlayback");
static const Settings::Key IS_PLAY_REPEATS_ENABLED(module_name, "application/playback/playRepeats");
static const Settings::Key IS_METRONOME_ENABLED(module_name, "application/playback/metronomeEnabled");
static const Settings::Key IS_COUNT_IN_ENABLED(module_name, "application/playback/countInEnabled");

static const Settings::Key TOOLBAR_KEY(module_name, "ui/toolbar/");

static const Settings::Key IS_CANVAS_ORIENTATION_VERTICAL_KEY(module_name, "ui/canvas/scroll/verticalOrientation");
static const Settings::Key IS_LIMIT_CANVAS_SCROLL_AREA_KEY(module_name, "ui/canvas/scroll/limitScrollArea");

static const Settings::Key COLOR_NOTES_OUTSIDE_OF_USABLE_PITCH_RANGE(module_name, "score/note/warnPitchRange");
static const Settings::Key REALTIME_DELAY(module_name, "io/midi/realtimeDelay");
static const Settings::Key NOTE_DEFAULT_PLAY_DURATION(module_name, "score/note/defaultPlayDuration");

static const Settings::Key VOICE1_COLOR_KEY(module_name, "ui/score/voice1/color");
static const Settings::Key VOICE2_COLOR_KEY(module_name, "ui/score/voice2/color");
static const Settings::Key VOICE3_COLOR_KEY(module_name, "ui/score/voice3/color");
static const Settings::Key VOICE4_COLOR_KEY(module_name, "ui/score/voice4/color");

static const Settings::Key FIRST_INSTRUMENT_LIST_KEY(module_name, "application/paths/instrumentList1");
static const Settings::Key SECOND_INSTRUMENT_LIST_KEY(module_name, "application/paths/instrumentList2");
static const Settings::Key FIRST_SCORE_ORDER_LIST_KEY(module_name, "application/paths/scoreOrderList1");
static const Settings::Key SECOND_SCORE_ORDER_LIST_KEY(module_name, "application/paths/scoreOrderList2");

static std::map<int, Settings::Key> voicesKeys {
    { 0, VOICE1_COLOR_KEY },
    { 1, VOICE2_COLOR_KEY },
    { 2, VOICE3_COLOR_KEY },
    { 3, VOICE4_COLOR_KEY }
};

void NotationConfiguration::init()
{
    settings()->setDefaultValue(BACKGROUND_USE_COLOR, Val(true));
    settings()->valueChanged(BACKGROUND_USE_COLOR).onReceive(nullptr, [this](const Val&) {
        m_backgroundChanged.notify();
    });

    settings()->setDefaultValue(BACKGROUND_COLOR, Val(QColor("#385f94")));
    settings()->valueChanged(BACKGROUND_COLOR).onReceive(nullptr, [this](const Val&) {
        m_backgroundChanged.notify();
    });

    settings()->setDefaultValue(BACKGROUND_WALLPAPER_PATH, Val());
    settings()->valueChanged(BACKGROUND_WALLPAPER_PATH).onReceive(nullptr, [this](const Val&) {
        m_backgroundChanged.notify();
    });

    settings()->setDefaultValue(FOREGROUND_USE_COLOR, Val(true));
    settings()->valueChanged(FOREGROUND_USE_COLOR).onReceive(nullptr, [this](const Val&) {
        m_foregroundChanged.notify();
    });

    settings()->setDefaultValue(FOREGROUND_COLOR, Val(QColor("#f9f9f9")));
    settings()->valueChanged(FOREGROUND_COLOR).onReceive(nullptr, [this](const Val&) {
        m_foregroundChanged.notify();
    });

    settings()->setDefaultValue(FOREGROUND_WALLPAPER_PATH, Val());
    settings()->valueChanged(FOREGROUND_WALLPAPER_PATH).onReceive(nullptr, [this](const Val&) {
        m_foregroundChanged.notify();
    });

    settings()->setDefaultValue(DEFAULT_ZOOM_TYPE, Val(static_cast<int>(ZoomType::Percentage)));
    settings()->setDefaultValue(DEFAULT_ZOOM, Val(100));
    settings()->setDefaultValue(KEYBOARD_ZOOM_PRECISION, Val(2));
    settings()->setDefaultValue(MOUSE_ZOOM_PRECISION, Val(6));
    settings()->setDefaultValue(CURRENT_ZOOM, Val(100));
    settings()->valueChanged(CURRENT_ZOOM).onReceive(nullptr, [this](const Val& val) {
        m_currentZoomChanged.send(val.toInt());
    });

    settings()->setDefaultValue(USER_STYLES_PATH, Val(globalConfiguration()->userDataPath() + "/Styles"));
    settings()->valueChanged(USER_STYLES_PATH).onReceive(nullptr, [this](const Val& val) {
        m_userStylesPathChanged.send(val.toPath());
    });
    fileSystem()->makePath(userStylesPath());

    settings()->setDefaultValue(SELECTION_PROXIMITY, Val(6));
    settings()->setDefaultValue(IS_MIDI_INPUT_ENABLED, Val(false));
    settings()->setDefaultValue(IS_AUTOMATICALLY_PAN_ENABLED, Val(true));
    settings()->setDefaultValue(IS_PLAY_REPEATS_ENABLED, Val(false));
    settings()->setDefaultValue(IS_METRONOME_ENABLED, Val(false));
    settings()->setDefaultValue(IS_COUNT_IN_ENABLED, Val(false));

    settings()->setDefaultValue(IS_CANVAS_ORIENTATION_VERTICAL_KEY, Val(false));
    settings()->valueChanged(IS_CANVAS_ORIENTATION_VERTICAL_KEY).onReceive(nullptr, [this](const Val&) {
        m_canvasOrientationChanged.send(canvasOrientation().val);
    });

    settings()->setDefaultValue(IS_LIMIT_CANVAS_SCROLL_AREA_KEY, Val(false));

    settings()->setDefaultValue(COLOR_NOTES_OUTSIDE_OF_USABLE_PITCH_RANGE, Val(true));
    settings()->setDefaultValue(REALTIME_DELAY, Val(750));
    settings()->setDefaultValue(NOTE_DEFAULT_PLAY_DURATION, Val(300));

    std::vector<std::pair<Settings::Key, QColor> > voicesColors {
        { VOICE1_COLOR_KEY, QColor(0x0065BF) },
        { VOICE2_COLOR_KEY, QColor(0x007F00) },
        { VOICE3_COLOR_KEY, QColor(0xC53F00) },
        { VOICE4_COLOR_KEY, QColor(0xC31989) }
    };

    for (int i = 0; i < static_cast<int>(voicesColors.size()); ++i) {
        Settings::Key key = voicesColors[i].first;
        QColor color = voicesColors[i].second;
        settings()->setDefaultValue(key, Val(color));
        settings()->setCanBeMannualyEdited(key, true);
        settings()->valueChanged(key).onReceive(nullptr, [this, i](const Val& color) {
            Ms::MScore::selectColor[i] = color.toQColor();
            m_selectionColorChanged.send(i);
        });
    }

    settings()->setDefaultValue(FIRST_INSTRUMENT_LIST_KEY,
                                Val(globalConfiguration()->appDataPath().toStdString() + "instruments/instruments.xml"));
    settings()->valueChanged(FIRST_INSTRUMENT_LIST_KEY).onReceive(nullptr, [this](const Val&) {
        m_instrumentListPathsChanged.notify();
    });

    settings()->setDefaultValue(SECOND_INSTRUMENT_LIST_KEY, Val(""));
    settings()->valueChanged(SECOND_INSTRUMENT_LIST_KEY).onReceive(nullptr, [this](const Val&) {
        m_instrumentListPathsChanged.notify();
    });

    settings()->setDefaultValue(FIRST_SCORE_ORDER_LIST_KEY,
                                Val(globalConfiguration()->appDataPath().toStdString() + "instruments/orders.xml"));
    settings()->valueChanged(FIRST_SCORE_ORDER_LIST_KEY).onReceive(nullptr, [this](const Val&) {
        m_scoreOrderListPathsChanged.notify();
    });

    settings()->setDefaultValue(SECOND_SCORE_ORDER_LIST_KEY, Val(""));
    settings()->valueChanged(SECOND_SCORE_ORDER_LIST_KEY).onReceive(nullptr, [this](const Val&) {
        m_scoreOrderListPathsChanged.notify();
    });

    // libmscore
    preferences().setBackupDirPath(globalConfiguration()->userBackupPath().toQString());
    preferences().setDefaultStyleFilePath(defaultStyleFilePath().toQString());

    Ms::MScore::warnPitchRange = colorNotesOusideOfUsablePitchRange();
    Ms::MScore::defaultPlayDuration = notePlayDurationMilliseconds();
}

QColor NotationConfiguration::anchorLineColor() const
{
    return selectionColor(3);
}

QColor NotationConfiguration::backgroundColor() const
{
    return settings()->value(BACKGROUND_COLOR).toQColor();
}

void NotationConfiguration::setBackgroundColor(const QColor& color)
{
    settings()->setSharedValue(BACKGROUND_COLOR, Val(color));
}

io::path NotationConfiguration::backgroundWallpaperPath() const
{
    return settings()->value(BACKGROUND_WALLPAPER_PATH).toString();
}

void NotationConfiguration::setBackgroundWallpaperPath(const io::path& path)
{
    settings()->setSharedValue(BACKGROUND_WALLPAPER_PATH, Val(path.toStdString()));
}

bool NotationConfiguration::backgroundUseColor() const
{
    return settings()->value(BACKGROUND_USE_COLOR).toBool();
}

void NotationConfiguration::setBackgroundUseColor(bool value)
{
    settings()->setSharedValue(BACKGROUND_USE_COLOR, Val(value));
}

async::Notification NotationConfiguration::backgroundChanged() const
{
    return m_backgroundChanged;
}

QColor NotationConfiguration::foregroundColor() const
{
    return settings()->value(FOREGROUND_COLOR).toQColor();
}

void NotationConfiguration::setForegroundColor(const QColor& color)
{
    settings()->setSharedValue(FOREGROUND_COLOR, Val(color));
}

io::path NotationConfiguration::foregroundWallpaperPath() const
{
    return settings()->value(FOREGROUND_WALLPAPER_PATH).toString();
}

void NotationConfiguration::setForegroundWallpaperPath(const io::path& path)
{
    return settings()->setSharedValue(FOREGROUND_WALLPAPER_PATH, Val(path.toStdString()));
}

bool NotationConfiguration::foregroundUseColor() const
{
    return settings()->value(FOREGROUND_USE_COLOR).toBool();
}

void NotationConfiguration::setForegroundUseColor(bool value)
{
    settings()->setSharedValue(FOREGROUND_USE_COLOR, Val(value));
}

async::Notification NotationConfiguration::foregroundChanged() const
{
    return m_foregroundChanged;
}

io::path NotationConfiguration::wallpapersDefaultDirPath() const
{
    return globalConfiguration()->appDataPath() + "/wallpapers";
}

QColor NotationConfiguration::borderColor() const
{
    return QColor(0, 0, 0, 102);
}

int NotationConfiguration::borderWidth() const
{
    return 1;
}

QColor NotationConfiguration::playbackCursorColor() const
{
    return selectionColor();
}

int NotationConfiguration::cursorOpacity() const
{
    return 50;
}

QColor NotationConfiguration::loopMarkerColor() const
{
    return QColor(0x2456AA);
}

QColor NotationConfiguration::layoutBreakColor() const
{
    return QColor(0xA0A0A4);
}

QColor NotationConfiguration::selectionColor(int voiceIndex) const
{
    if (!isVoiceIndexValid(voiceIndex)) {
        return QColor();
    }

    return settings()->value(voicesKeys[voiceIndex]).toQColor();
}

void NotationConfiguration::setSelectionColor(int voiceIndex, const QColor& color)
{
    if (!isVoiceIndexValid(voiceIndex)) {
        return;
    }

    settings()->setSharedValue(voicesKeys[voiceIndex], Val(color));
}

async::Channel<int> NotationConfiguration::selectionColorChanged()
{
    return m_selectionColorChanged;
}

int NotationConfiguration::selectionProximity() const
{
    return settings()->value(SELECTION_PROXIMITY).toInt();
}

void NotationConfiguration::setSelectionProximity(int proxymity)
{
    settings()->setSharedValue(SELECTION_PROXIMITY, Val(proxymity));
}

ZoomType NotationConfiguration::defaultZoomType() const
{
    return static_cast<ZoomType>(settings()->value(DEFAULT_ZOOM_TYPE).toInt());
}

void NotationConfiguration::setDefaultZoomType(ZoomType zoomType)
{
    settings()->setSharedValue(DEFAULT_ZOOM_TYPE, Val(static_cast<int>(zoomType)));
}

int NotationConfiguration::defaultZoom() const
{
    return settings()->value(DEFAULT_ZOOM).toInt();
}

void NotationConfiguration::setDefaultZoom(int zoomPercentage)
{
    settings()->setSharedValue(DEFAULT_ZOOM, Val(zoomPercentage));
}

mu::ValCh<int> NotationConfiguration::currentZoom() const
{
    mu::ValCh<int> zoom;
    zoom.ch = m_currentZoomChanged;
    zoom.val = settings()->value(CURRENT_ZOOM).toInt();

    return zoom;
}

void NotationConfiguration::setCurrentZoom(int zoomPercentage)
{
    settings()->setSharedValue(CURRENT_ZOOM, Val(zoomPercentage));
}

QList<int> NotationConfiguration::possibleZoomPercentageList() const
{
    return {
        5, 10, 15, 25, 50, 75, 100, 150, 200, 400, 800, 1600
    };
}

int NotationConfiguration::mouseZoomPrecision() const
{
    return settings()->value(MOUSE_ZOOM_PRECISION).toInt();
}

void NotationConfiguration::setMouseZoomPrecision(int precision)
{
    settings()->setSharedValue(MOUSE_ZOOM_PRECISION, Val(precision));
}

std::string NotationConfiguration::fontFamily() const
{
    return uiConfiguration()->fontFamily();
}

int NotationConfiguration::fontSize() const
{
    return uiConfiguration()->fontSize(FontSizeType::BODY);
}

io::path NotationConfiguration::userStylesPath() const
{
    return settings()->value(USER_STYLES_PATH).toPath();
}

void NotationConfiguration::setUserStylesPath(const io::path& path)
{
    settings()->setSharedValue(USER_STYLES_PATH, Val(path));
}

async::Channel<io::path> NotationConfiguration::userStylesPathChanged() const
{
    return m_userStylesPathChanged;
}

io::path NotationConfiguration::defaultStyleFilePath() const
{
    return engravingConfiguration()->defaultStyleFilePath();
}

void NotationConfiguration::setDefaultStyleFilePath(const io::path& path)
{
    engravingConfiguration()->setDefaultStyleFilePath(path.toQString());
}

io::path NotationConfiguration::partStyleFilePath() const
{
    return engravingConfiguration()->partStyleFilePath();
}

void NotationConfiguration::setPartStyleFilePath(const io::path& path)
{
    engravingConfiguration()->setPartStyleFilePath(path.toQString());
}

bool NotationConfiguration::isMidiInputEnabled() const
{
    return settings()->value(IS_MIDI_INPUT_ENABLED).toBool();
}

void NotationConfiguration::setIsMidiInputEnabled(bool enabled)
{
    settings()->setSharedValue(IS_MIDI_INPUT_ENABLED, Val(enabled));
}

bool NotationConfiguration::isAutomaticallyPanEnabled() const
{
    return settings()->value(IS_AUTOMATICALLY_PAN_ENABLED).toBool();
}

void NotationConfiguration::setIsAutomaticallyPanEnabled(bool enabled)
{
    settings()->setSharedValue(IS_AUTOMATICALLY_PAN_ENABLED, Val(enabled));
    Ms::MScore::panPlayback = enabled;
}

bool NotationConfiguration::isPlayRepeatsEnabled() const
{
    return settings()->value(IS_PLAY_REPEATS_ENABLED).toBool();
}

void NotationConfiguration::setIsPlayRepeatsEnabled(bool enabled)
{
    settings()->setSharedValue(IS_PLAY_REPEATS_ENABLED, Val(enabled));
    Ms::MScore::playRepeats = enabled;
}

bool NotationConfiguration::isMetronomeEnabled() const
{
    return settings()->value(IS_METRONOME_ENABLED).toBool();
}

void NotationConfiguration::setIsMetronomeEnabled(bool enabled)
{
    settings()->setSharedValue(IS_METRONOME_ENABLED, Val(enabled));
}

bool NotationConfiguration::isCountInEnabled() const
{
    return settings()->value(IS_COUNT_IN_ENABLED).toBool();
}

void NotationConfiguration::setIsCountInEnabled(bool enabled)
{
    settings()->setSharedValue(IS_COUNT_IN_ENABLED, Val(enabled));
}

float NotationConfiguration::guiScaling() const
{
    return uiConfiguration()->guiScaling();
}

float NotationConfiguration::notationScaling() const
{
    return uiConfiguration()->physicalDotsPerInch() / Ms::DPI;
}

std::string NotationConfiguration::notationRevision() const
{
    return Ms::revision.toStdString();
}

int NotationConfiguration::notationDivision() const
{
    return Ms::MScore::division;
}

ValCh<Orientation> NotationConfiguration::canvasOrientation() const
{
    ValCh<Orientation> orientation;
    orientation.ch = m_canvasOrientationChanged;
    bool isVertical = settings()->value(IS_CANVAS_ORIENTATION_VERTICAL_KEY).toBool();
    orientation.val = isVertical ? Orientation::Vertical : Orientation::Horizontal;

    return orientation;
}

void NotationConfiguration::setCanvasOrientation(Orientation orientation)
{
    bool isVertical = orientation == Orientation::Vertical;
    Ms::MScore::setVerticalOrientation(isVertical);

    settings()->setSharedValue(IS_CANVAS_ORIENTATION_VERTICAL_KEY, Val(isVertical));
}

bool NotationConfiguration::isLimitCanvasScrollArea() const
{
    return settings()->value(IS_LIMIT_CANVAS_SCROLL_AREA_KEY).toBool();
}

void NotationConfiguration::setIsLimitCanvasScrollArea(bool limited)
{
    settings()->setSharedValue(IS_LIMIT_CANVAS_SCROLL_AREA_KEY, Val(limited));
}

bool NotationConfiguration::colorNotesOusideOfUsablePitchRange() const
{
    return settings()->value(COLOR_NOTES_OUTSIDE_OF_USABLE_PITCH_RANGE).toBool();
}

void NotationConfiguration::setColorNotesOusideOfUsablePitchRange(bool value)
{
    Ms::MScore::warnPitchRange = value;
    settings()->setSharedValue(COLOR_NOTES_OUTSIDE_OF_USABLE_PITCH_RANGE, Val(value));
}

int NotationConfiguration::delayBetweenNotesInRealTimeModeMilliseconds() const
{
    return settings()->value(REALTIME_DELAY).toInt();
}

void NotationConfiguration::setDelayBetweenNotesInRealTimeModeMilliseconds(int delayMs)
{
    settings()->setSharedValue(REALTIME_DELAY, Val(delayMs));
}

int NotationConfiguration::notePlayDurationMilliseconds() const
{
    return settings()->value(NOTE_DEFAULT_PLAY_DURATION).toInt();
}

void NotationConfiguration::setNotePlayDurationMilliseconds(int durationMs)
{
    Ms::MScore::defaultPlayDuration = durationMs;
    settings()->setSharedValue(NOTE_DEFAULT_PLAY_DURATION, Val(durationMs));
}

void NotationConfiguration::setTemplateModeEnalbed(bool enabled)
{
    Ms::MScore::saveTemplateMode = enabled;
}

void NotationConfiguration::setTestModeEnabled(bool enabled)
{
    Ms::MScore::testMode = enabled;
}

io::paths NotationConfiguration::instrumentListPaths() const
{
    io::paths paths;

    io::path firstInstrumentListPath = this->firstInstrumentListPath();
    paths.push_back(firstInstrumentListPath);

    io::path secondInstrumentListPath = this->secondInstrumentListPath();
    if (!secondInstrumentListPath.empty()) {
        paths.push_back(secondInstrumentListPath);
    }

    io::path firstScoreOrderListPath = this->firstScoreOrderListPath();
    paths.push_back(firstScoreOrderListPath);

    io::path secondScoreOrderListPath = this->secondScoreOrderListPath();
    if (!secondScoreOrderListPath.empty()) {
        paths.push_back(secondScoreOrderListPath);
    }

    return paths;
}

async::Notification NotationConfiguration::instrumentListPathsChanged() const
{
    return m_instrumentListPathsChanged;
}

io::paths NotationConfiguration::userInstrumentListPaths() const
{
    io::paths paths = {
        firstInstrumentListPath(),
        secondInstrumentListPath()
    };

    return paths;
}

void NotationConfiguration::setUserInstrumentListPaths(const io::paths& paths)
{
    if (paths.empty()) {
        return;
    }

    setFirstInstrumentListPath(paths[0]);
    if (paths.size() > 1) {
        setSecondInstrumentListPath(paths[1]);
    }
}

io::path NotationConfiguration::firstInstrumentListPath() const
{
    return settings()->value(FIRST_INSTRUMENT_LIST_KEY).toString();
}

void NotationConfiguration::setFirstInstrumentListPath(const io::path& path)
{
    settings()->setSharedValue(FIRST_INSTRUMENT_LIST_KEY, Val(path.toStdString()));
}

io::path NotationConfiguration::secondInstrumentListPath() const
{
    return settings()->value(SECOND_INSTRUMENT_LIST_KEY).toString();
}

void NotationConfiguration::setSecondInstrumentListPath(const io::path& path)
{
    settings()->setSharedValue(SECOND_INSTRUMENT_LIST_KEY, Val(path.toStdString()));
}

io::paths NotationConfiguration::scoreOrderListPaths() const
{
    io::paths paths;

    io::path firstScoreOrderListPath = this->firstScoreOrderListPath();
    paths.push_back(firstScoreOrderListPath);

    io::path secondScoreOrderListPath = this->secondScoreOrderListPath();
    if (!secondScoreOrderListPath.empty()) {
        paths.push_back(secondScoreOrderListPath);
    }

    return paths;
}

async::Notification NotationConfiguration::scoreOrderListPathsChanged() const
{
    return m_scoreOrderListPathsChanged;
}

io::paths NotationConfiguration::userScoreOrderListPaths() const
{
    io::paths paths = {
        firstScoreOrderListPath(),
        secondScoreOrderListPath()
    };

    return paths;
}

void NotationConfiguration::setUserScoreOrderListPaths(const io::paths& paths)
{
    if (paths.empty()) {
        return;
    }

    setFirstScoreOrderListPath(paths[0]);
    if (paths.size() > 1) {
        setSecondScoreOrderListPath(paths[1]);
    }
}

io::path NotationConfiguration::firstScoreOrderListPath() const
{
    return settings()->value(FIRST_SCORE_ORDER_LIST_KEY).toString();
}

void NotationConfiguration::setFirstScoreOrderListPath(const io::path& path)
{
    settings()->setSharedValue(FIRST_SCORE_ORDER_LIST_KEY, Val(path.toStdString()));
}

io::path NotationConfiguration::secondScoreOrderListPath() const
{
    return settings()->value(SECOND_SCORE_ORDER_LIST_KEY).toString();
}

void NotationConfiguration::setSecondScoreOrderListPath(const io::path& path)
{
    settings()->setSharedValue(SECOND_SCORE_ORDER_LIST_KEY, Val(path.toStdString()));
}
