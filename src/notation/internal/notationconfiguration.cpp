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

static const std::string module_name("notation");

static const Settings::Key LIGHT_SCORE_BACKGROUND_COLOR(module_name, "ui/canvas/background/lightTheme_score_background_color");
static const Settings::Key DARK_SCORE_BACKGROUND_COLOR(module_name, "ui/canvas/background/darkTheme_score_background_color");
static const Settings::Key HC_BLACK_SCORE_BACKGROUND_COLOR(module_name, "ui/canvas/background/hc_black_score_background_color");
static const Settings::Key HC_WHITE_SCORE_BACKGROUND_COLOR(module_name, "ui/canvas/background/hc_white_score_background_color");
static const Settings::Key BACKGROUND_WALLPAPER_PATH(module_name, "ui/canvas/background/wallpaper");
static const Settings::Key BACKGROUND_USE_COLOR(module_name, "ui/canvas/background/useColor");

static const Settings::Key FOREGROUND_COLOR(module_name, "ui/canvas/foreground/color");
static const Settings::Key FOREGROUND_WALLPAPER_PATH(module_name, "ui/canvas/foreground/wallpaper");
static const Settings::Key FOREGROUND_USE_COLOR(module_name, "ui/canvas/foreground/useColor");

static const Settings::Key SELECTION_PROXIMITY(module_name, "ui/canvas/misc/selectionProximity");

static const Settings::Key DEFAULT_ZOOM_TYPE(module_name, "ui/canvas/zoomDefaultType");
static const Settings::Key DEFAULT_ZOOM(module_name, "ui/canvas/zoomDefaultLevel");
static const Settings::Key KEYBOARD_ZOOM_PRECISION(module_name, "ui/canvas/zoomPrecisionKeyboard");
static const Settings::Key MOUSE_ZOOM_PRECISION(module_name, "ui/canvas/zoomPrecisionMouse");

static const Settings::Key USER_STYLES_PATH(module_name, "application/paths/myStyles");

static const Settings::Key IS_MIDI_INPUT_ENABLED(module_name, "io/midi/enableInput");
static const Settings::Key IS_AUTOMATICALLY_PAN_ENABLED(module_name, "application/playback/panPlayback");
static const Settings::Key IS_PLAY_REPEATS_ENABLED(module_name, "application/playback/playRepeats");
static const Settings::Key IS_PLAY_CHORD_SYMBOLS_ENABLED(module_name, "application/playback/playChordSymbols");
static const Settings::Key IS_METRONOME_ENABLED(module_name, "application/playback/metronomeEnabled");
static const Settings::Key IS_COUNT_IN_ENABLED(module_name, "application/playback/countInEnabled");

static const Settings::Key TOOLBAR_KEY(module_name, "ui/toolbar/");

static const Settings::Key IS_CANVAS_ORIENTATION_VERTICAL_KEY(module_name, "ui/canvas/scroll/verticalOrientation");
static const Settings::Key IS_LIMIT_CANVAS_SCROLL_AREA_KEY(module_name, "ui/canvas/scroll/limitScrollArea");

static const Settings::Key COLOR_NOTES_OUTSIDE_OF_USABLE_PITCH_RANGE(module_name, "score/note/warnPitchRange");
static const Settings::Key REALTIME_DELAY(module_name, "io/midi/realtimeDelay");
static const Settings::Key NOTE_DEFAULT_PLAY_DURATION(module_name, "score/note/defaultPlayDuration");

static const Settings::Key FIRST_SCORE_ORDER_LIST_KEY(module_name, "application/paths/scoreOrderList1");
static const Settings::Key SECOND_SCORE_ORDER_LIST_KEY(module_name, "application/paths/scoreOrderList2");

static const Settings::Key IS_SNAPPED_TO_VERTICAL_GRID_KEY(module_name,  "ui/application/raster/isSnappedToVerticalGrid");
static const Settings::Key IS_SNAPPED_TO_HORIZONTAL_GRID_KEY(module_name,  "ui/application/raster/isSnappedToHorizontalGrid");
static const Settings::Key HORIZONTAL_GRID_SIZE_KEY(module_name,  "ui/application/raster/horizontal");
static const Settings::Key VERTICAL_GRID_SIZE_KEY(module_name,  "ui/application/raster/vertical");

static const Settings::Key NEED_TO_SHOW_ADD_TEXT_ERROR_MESSAGE_KEY(module_name,  "ui/dialogs/needToShowAddTextErrorMessage");
static const Settings::Key NEED_TO_SHOW_ADD_FIGURED_BASS_ERROR_MESSAGE_KEY(module_name,  "ui/dialogs/needToShowAddFiguredBassErrorMessage");

static const Settings::Key PIANO_KEYBOARD_NUMBER_OF_KEYS(module_name,  "pianoKeyboard/numberOfKeys");

static const Settings::Key STYLE_FILE_IMPORT_PATH_KEY(module_name, "import/style/styleFile");

static constexpr int DEFAULT_GRID_SIZE_SPATIUM = 2;

void NotationConfiguration::init()
{
    settings()->setDefaultValue(BACKGROUND_USE_COLOR, Val(true));
    settings()->valueChanged(BACKGROUND_USE_COLOR).onReceive(nullptr, [this](const Val&) {
        m_backgroundChanged.notify();
    });

    uiConfiguration()->currentThemeChanged().onNotify(this, [this]()
    {
        m_backgroundChanged.notify();
    });

    settings()->setDefaultValue(LIGHT_SCORE_BACKGROUND_COLOR, Val(QColor("#BCC1CC")));
    settings()->valueChanged(LIGHT_SCORE_BACKGROUND_COLOR).onReceive(nullptr, [this](const Val&) {
        m_backgroundChanged.notify();
    });

    settings()->setDefaultValue(DARK_SCORE_BACKGROUND_COLOR, Val(QColor("#27272B")));
    settings()->valueChanged(DARK_SCORE_BACKGROUND_COLOR).onReceive(nullptr, [this](const Val&) {
        m_backgroundChanged.notify();
    });

    settings()->setDefaultValue(HC_BLACK_SCORE_BACKGROUND_COLOR, Val(QColor("#000000")));
    settings()->valueChanged(HC_BLACK_SCORE_BACKGROUND_COLOR).onReceive(nullptr, [this](const Val&) {
        m_backgroundChanged.notify();
    });

    settings()->setDefaultValue(HC_WHITE_SCORE_BACKGROUND_COLOR, Val(QColor("#000000")));
    settings()->valueChanged(HC_WHITE_SCORE_BACKGROUND_COLOR).onReceive(nullptr, [this](const Val&) {
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

    settings()->setDefaultValue(DEFAULT_ZOOM_TYPE, Val(ZoomType::Percentage));
    settings()->setDefaultValue(DEFAULT_ZOOM, Val(100));
    settings()->setDefaultValue(KEYBOARD_ZOOM_PRECISION, Val(2));
    settings()->setDefaultValue(MOUSE_ZOOM_PRECISION, Val(6));

    settings()->setDefaultValue(USER_STYLES_PATH, Val(globalConfiguration()->userDataPath() + "/Styles"));
    settings()->valueChanged(USER_STYLES_PATH).onReceive(nullptr, [this](const Val& val) {
        m_userStylesPathChanged.send(val.toPath());
    });
    fileSystem()->makePath(userStylesPath());

    settings()->setDefaultValue(SELECTION_PROXIMITY, Val(2));
    settings()->setDefaultValue(IS_MIDI_INPUT_ENABLED, Val(true));
    settings()->setDefaultValue(IS_AUTOMATICALLY_PAN_ENABLED, Val(true));
    settings()->setDefaultValue(IS_PLAY_REPEATS_ENABLED, Val(true));
    settings()->setDefaultValue(IS_PLAY_CHORD_SYMBOLS_ENABLED, Val(true));
    settings()->setDefaultValue(IS_METRONOME_ENABLED, Val(false));
    settings()->setDefaultValue(IS_COUNT_IN_ENABLED, Val(false));

    settings()->valueChanged(IS_PLAY_CHORD_SYMBOLS_ENABLED).onReceive(nullptr, [this](const Val&) {
        m_isPlayChordSymbolsChanged.notify();
    });

    settings()->setDefaultValue(IS_CANVAS_ORIENTATION_VERTICAL_KEY, Val(false));
    settings()->valueChanged(IS_CANVAS_ORIENTATION_VERTICAL_KEY).onReceive(nullptr, [this](const Val&) {
        m_canvasOrientationChanged.send(canvasOrientation().val);
    });

    settings()->setDefaultValue(IS_LIMIT_CANVAS_SCROLL_AREA_KEY, Val(false));
    settings()->valueChanged(IS_LIMIT_CANVAS_SCROLL_AREA_KEY).onReceive(this, [this](const Val&) {
        m_isLimitCanvasScrollAreaChanged.notify();
    });

    settings()->setDefaultValue(COLOR_NOTES_OUTSIDE_OF_USABLE_PITCH_RANGE, Val(true));
    settings()->setDefaultValue(REALTIME_DELAY, Val(750));
    settings()->setDefaultValue(NOTE_DEFAULT_PLAY_DURATION, Val(500));

    settings()->setDefaultValue(FIRST_SCORE_ORDER_LIST_KEY,
                                Val(globalConfiguration()->appDataPath().toStdString() + "instruments/orders.xml"));
    settings()->valueChanged(FIRST_SCORE_ORDER_LIST_KEY).onReceive(nullptr, [this](const Val&) {
        m_scoreOrderListPathsChanged.notify();
    });

    settings()->setDefaultValue(SECOND_SCORE_ORDER_LIST_KEY, Val(""));
    settings()->valueChanged(SECOND_SCORE_ORDER_LIST_KEY).onReceive(nullptr, [this](const Val&) {
        m_scoreOrderListPathsChanged.notify();
    });

    settings()->setDefaultValue(HORIZONTAL_GRID_SIZE_KEY, Val(DEFAULT_GRID_SIZE_SPATIUM));
    settings()->setDefaultValue(VERTICAL_GRID_SIZE_KEY, Val(DEFAULT_GRID_SIZE_SPATIUM));

    settings()->setDefaultValue(NEED_TO_SHOW_ADD_TEXT_ERROR_MESSAGE_KEY, Val(true));
    settings()->setDefaultValue(NEED_TO_SHOW_ADD_FIGURED_BASS_ERROR_MESSAGE_KEY, Val(true));

    settings()->setDefaultValue(PIANO_KEYBOARD_NUMBER_OF_KEYS, Val(88));
    m_pianoKeyboardNumberOfKeys.val = settings()->value(PIANO_KEYBOARD_NUMBER_OF_KEYS).toInt();
    settings()->valueChanged(PIANO_KEYBOARD_NUMBER_OF_KEYS).onReceive(this, [this](const Val& val) {
        m_pianoKeyboardNumberOfKeys.set(val.toInt());
    });

    engravingConfiguration()->scoreInversionChanged().onNotify(this, [this]() {
        m_foregroundChanged.notify();
    });

    mu::engraving::MScore::warnPitchRange = colorNotesOutsideOfUsablePitchRange();
    mu::engraving::MScore::defaultPlayDuration = notePlayDurationMilliseconds();

    mu::engraving::MScore::setHRaster(DEFAULT_GRID_SIZE_SPATIUM);
    mu::engraving::MScore::setVRaster(DEFAULT_GRID_SIZE_SPATIUM);
}

QColor NotationConfiguration::anchorLineColor() const
{
    return selectionColor(3);
}

QColor NotationConfiguration::backgroundColor() const
{
    if (uiConfiguration()->currentTheme().codeKey == LIGHT_THEME_CODE) {
        return settings()->value(LIGHT_SCORE_BACKGROUND_COLOR).toQColor();
    } else if (uiConfiguration()->currentTheme().codeKey == DARK_THEME_CODE) {
        return settings()->value(DARK_SCORE_BACKGROUND_COLOR).toQColor();
    } else if (uiConfiguration()->currentTheme().codeKey == HIGH_CONTRAST_BLACK_THEME_CODE) {
        return settings()->value(HC_BLACK_SCORE_BACKGROUND_COLOR).toQColor();
    } else {
        return settings()->value(HC_WHITE_SCORE_BACKGROUND_COLOR).toQColor();
    }
}

void NotationConfiguration::setBackgroundColor(const QColor& color)
{
    if (uiConfiguration()->currentTheme().codeKey == LIGHT_THEME_CODE) {
        settings()->setSharedValue(LIGHT_SCORE_BACKGROUND_COLOR, Val(color));
    } else if (uiConfiguration()->currentTheme().codeKey == DARK_THEME_CODE) {
        settings()->setSharedValue(DARK_SCORE_BACKGROUND_COLOR, Val(color));
    } else if (uiConfiguration()->currentTheme().codeKey == HIGH_CONTRAST_BLACK_THEME_CODE) {
        settings()->setSharedValue(HC_BLACK_SCORE_BACKGROUND_COLOR, Val(color));
    } else {
        settings()->setSharedValue(HC_WHITE_SCORE_BACKGROUND_COLOR, Val(color));
    }
}

io::path_t NotationConfiguration::backgroundWallpaperPath() const
{
    return settings()->value(BACKGROUND_WALLPAPER_PATH).toString();
}

const QPixmap& NotationConfiguration::backgroundWallpaper() const
{
    io::path_t path = backgroundWallpaperPath();

    static QPixmap wallpaper;
    static io::path_t lastPath = path;

    if (path.empty()) {
        wallpaper = QPixmap();
    } else if (path != lastPath) {
        wallpaper = QPixmap(path.toQString());
    }

    lastPath = path;

    return wallpaper;
}

void NotationConfiguration::setBackgroundWallpaperPath(const io::path_t& path)
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

void NotationConfiguration::resetBackground()
{
    settings()->setSharedValue(LIGHT_SCORE_BACKGROUND_COLOR, settings()->defaultValue(LIGHT_SCORE_BACKGROUND_COLOR));
    settings()->setSharedValue(DARK_SCORE_BACKGROUND_COLOR, settings()->defaultValue(DARK_SCORE_BACKGROUND_COLOR));
    settings()->setSharedValue(HC_BLACK_SCORE_BACKGROUND_COLOR, settings()->defaultValue(HC_BLACK_SCORE_BACKGROUND_COLOR));
    settings()->setSharedValue(HC_WHITE_SCORE_BACKGROUND_COLOR, settings()->defaultValue(HC_WHITE_SCORE_BACKGROUND_COLOR));

    settings()->setSharedValue(BACKGROUND_USE_COLOR, settings()->defaultValue(BACKGROUND_USE_COLOR));

    settings()->setSharedValue(BACKGROUND_WALLPAPER_PATH, settings()->defaultValue(BACKGROUND_WALLPAPER_PATH));
}

async::Notification NotationConfiguration::backgroundChanged() const
{
    return m_backgroundChanged;
}

QColor NotationConfiguration::foregroundColor() const
{
    if (engravingConfiguration()->scoreInversionEnabled()) {
        return QColorConstants::Black;
    }

    return settings()->value(FOREGROUND_COLOR).toQColor();
}

void NotationConfiguration::setForegroundColor(const QColor& color)
{
    settings()->setSharedValue(FOREGROUND_COLOR, Val(color));
}

io::path_t NotationConfiguration::foregroundWallpaperPath() const
{
    return settings()->value(FOREGROUND_WALLPAPER_PATH).toString();
}

const QPixmap& NotationConfiguration::foregroundWallpaper() const
{
    io::path_t path = foregroundWallpaperPath();

    static QPixmap wallpaper;
    static io::path_t lastPath = path;

    if (path.empty()) {
        wallpaper = QPixmap();
    } else if (path != lastPath) {
        wallpaper = QPixmap(path.toQString());
    }

    lastPath = path;

    return wallpaper;
}

void NotationConfiguration::setForegroundWallpaperPath(const io::path_t& path)
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

void NotationConfiguration::resetForeground()
{
    settings()->setSharedValue(FOREGROUND_COLOR, settings()->defaultValue(FOREGROUND_COLOR));
    settings()->setSharedValue(FOREGROUND_USE_COLOR, settings()->defaultValue(FOREGROUND_USE_COLOR));
    settings()->setSharedValue(FOREGROUND_WALLPAPER_PATH, settings()->defaultValue(FOREGROUND_WALLPAPER_PATH));

    engravingConfiguration()->setScoreInversionEnabled(false);
}

async::Notification NotationConfiguration::foregroundChanged() const
{
    return m_foregroundChanged;
}

io::path_t NotationConfiguration::wallpapersDefaultDirPath() const
{
    return globalConfiguration()->appDataPath() + "/wallpapers";
}

QColor NotationConfiguration::borderColor() const
{
    if (uiConfiguration()->isHighContrast()) {
        return QColorConstants::White;
    } else {
        return QColor(0, 0, 0, 102);
    }
}

int NotationConfiguration::borderWidth() const
{
    if (uiConfiguration()->isHighContrast()) {
        return 10;
    } else {
        return 1;
    }
}

QColor NotationConfiguration::playbackCursorColor() const
{
    QColor color = selectionColor();
    color.setAlpha(178);
    return color;
}

int NotationConfiguration::cursorOpacity() const
{
    return 50;
}

QColor NotationConfiguration::loopMarkerColor() const
{
    return QColor(0x2456AA);
}

QColor NotationConfiguration::selectionColor(engraving::voice_idx_t voiceIndex) const
{
    return engravingConfiguration()->selectionColor(voiceIndex).toQColor();
}

QColor NotationConfiguration::dropRectColor() const
{
    return QColor(80, 0, 0, 80);
}

int NotationConfiguration::selectionProximity() const
{
    return settings()->value(SELECTION_PROXIMITY).toInt();
}

void NotationConfiguration::setSelectionProximity(int proximity)
{
    settings()->setSharedValue(SELECTION_PROXIMITY, Val(proximity));
}

ZoomType NotationConfiguration::defaultZoomType() const
{
    return settings()->value(DEFAULT_ZOOM_TYPE).toEnum<ZoomType>();
}

void NotationConfiguration::setDefaultZoomType(ZoomType zoomType)
{
    settings()->setSharedValue(DEFAULT_ZOOM_TYPE, Val(zoomType));
}

int NotationConfiguration::defaultZoom() const
{
    return settings()->value(DEFAULT_ZOOM).toInt();
}

void NotationConfiguration::setDefaultZoom(int zoomPercentage)
{
    settings()->setSharedValue(DEFAULT_ZOOM, Val(zoomPercentage));
}

qreal NotationConfiguration::scalingFromZoomPercentage(int zoomPercentage) const
{
    return zoomPercentage / 100.0 * notationScaling();
}

int NotationConfiguration::zoomPercentageFromScaling(qreal scaling) const
{
    return std::round(scaling * 100.0 / notationScaling());
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

io::path_t NotationConfiguration::userStylesPath() const
{
    return settings()->value(USER_STYLES_PATH).toPath();
}

void NotationConfiguration::setUserStylesPath(const io::path_t& path)
{
    settings()->setSharedValue(USER_STYLES_PATH, Val(path));
}

async::Channel<io::path_t> NotationConfiguration::userStylesPathChanged() const
{
    return m_userStylesPathChanged;
}

io::path_t NotationConfiguration::defaultStyleFilePath() const
{
    return engravingConfiguration()->defaultStyleFilePath();
}

void NotationConfiguration::setDefaultStyleFilePath(const io::path_t& path)
{
    engravingConfiguration()->setDefaultStyleFilePath(path.toQString());
}

io::path_t NotationConfiguration::partStyleFilePath() const
{
    return engravingConfiguration()->partStyleFilePath();
}

void NotationConfiguration::setPartStyleFilePath(const io::path_t& path)
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
}

bool NotationConfiguration::isPlayRepeatsEnabled() const
{
    return settings()->value(IS_PLAY_REPEATS_ENABLED).toBool();
}

void NotationConfiguration::setIsPlayRepeatsEnabled(bool enabled)
{
    settings()->setSharedValue(IS_PLAY_REPEATS_ENABLED, Val(enabled));
    mu::engraving::MScore::playRepeats = enabled;
    m_isPlayRepeatsChanged.notify();
}

Notification NotationConfiguration::isPlayRepeatsChanged() const
{
    return m_isPlayRepeatsChanged;
}

bool NotationConfiguration::isPlayChordSymbolsEnabled() const
{
    return settings()->value(IS_PLAY_CHORD_SYMBOLS_ENABLED).toBool();
}

void NotationConfiguration::setIsPlayChordSymbolsEnabled(bool enabled)
{
    settings()->setSharedValue(IS_PLAY_CHORD_SYMBOLS_ENABLED, Val(enabled));
}

async::Notification NotationConfiguration::isPlayChordSymbolsChanged() const
{
    return m_isPlayChordSymbolsChanged;
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

double NotationConfiguration::guiScaling() const
{
    return uiConfiguration()->guiScaling();
}

double NotationConfiguration::notationScaling() const
{
    return uiConfiguration()->physicalDpi() / mu::engraving::DPI;
}

ValCh<framework::Orientation> NotationConfiguration::canvasOrientation() const
{
    ValCh<framework::Orientation> orientation;
    orientation.ch = m_canvasOrientationChanged;
    bool isVertical = settings()->value(IS_CANVAS_ORIENTATION_VERTICAL_KEY).toBool();
    orientation.val = isVertical ? framework::Orientation::Vertical : framework::Orientation::Horizontal;

    return orientation;
}

void NotationConfiguration::setCanvasOrientation(framework::Orientation orientation)
{
    bool isVertical = orientation == framework::Orientation::Vertical;
    mu::engraving::MScore::setVerticalOrientation(isVertical);

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

Notification NotationConfiguration::isLimitCanvasScrollAreaChanged() const
{
    return m_isLimitCanvasScrollAreaChanged;
}

bool NotationConfiguration::colorNotesOutsideOfUsablePitchRange() const
{
    return settings()->value(COLOR_NOTES_OUTSIDE_OF_USABLE_PITCH_RANGE).toBool();
}

void NotationConfiguration::setColorNotesOutsideOfUsablePitchRange(bool value)
{
    mu::engraving::MScore::warnPitchRange = value;
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
    mu::engraving::MScore::defaultPlayDuration = durationMs;
    settings()->setSharedValue(NOTE_DEFAULT_PLAY_DURATION, Val(durationMs));
}

void NotationConfiguration::setTemplateModeEnabled(bool enabled)
{
    mu::engraving::MScore::saveTemplateMode = enabled;
}

void NotationConfiguration::setTestModeEnabled(bool enabled)
{
    mu::engraving::MScore::testMode = enabled;
}

io::path_t NotationConfiguration::instrumentListPath() const
{
    return globalConfiguration()->appDataPath() + "instruments/instruments.xml";
}

io::paths_t NotationConfiguration::scoreOrderListPaths() const
{
    io::paths_t paths;

    io::path_t firstScoreOrderListPath = this->firstScoreOrderListPath();
    paths.push_back(firstScoreOrderListPath);

    io::path_t secondScoreOrderListPath = this->secondScoreOrderListPath();
    if (!secondScoreOrderListPath.empty()) {
        paths.push_back(secondScoreOrderListPath);
    }

    return paths;
}

async::Notification NotationConfiguration::scoreOrderListPathsChanged() const
{
    return m_scoreOrderListPathsChanged;
}

io::paths_t NotationConfiguration::userScoreOrderListPaths() const
{
    io::paths_t paths = {
        firstScoreOrderListPath(),
        secondScoreOrderListPath()
    };

    return paths;
}

void NotationConfiguration::setUserScoreOrderListPaths(const io::paths_t& paths)
{
    if (paths.empty()) {
        return;
    }

    setFirstScoreOrderListPath(paths[0]);
    if (paths.size() > 1) {
        setSecondScoreOrderListPath(paths[1]);
    }
}

bool NotationConfiguration::isSnappedToGrid(framework::Orientation gridOrientation) const
{
    switch (gridOrientation) {
    case framework::Orientation::Horizontal: return settings()->value(IS_SNAPPED_TO_HORIZONTAL_GRID_KEY).toBool();
    case framework::Orientation::Vertical: return settings()->value(IS_SNAPPED_TO_VERTICAL_GRID_KEY).toBool();
    }

    return false;
}

void NotationConfiguration::setIsSnappedToGrid(framework::Orientation gridOrientation, bool isSnapped)
{
    switch (gridOrientation) {
    case framework::Orientation::Horizontal:
        settings()->setSharedValue(IS_SNAPPED_TO_HORIZONTAL_GRID_KEY, Val(isSnapped));
        break;
    case framework::Orientation::Vertical:
        settings()->setSharedValue(IS_SNAPPED_TO_VERTICAL_GRID_KEY, Val(isSnapped));
        break;
    }
}

int NotationConfiguration::gridSizeSpatium(framework::Orientation gridOrientation) const
{
    switch (gridOrientation) {
    case framework::Orientation::Horizontal: return settings()->value(HORIZONTAL_GRID_SIZE_KEY).toInt();
    case framework::Orientation::Vertical: return settings()->value(VERTICAL_GRID_SIZE_KEY).toInt();
    }

    return DEFAULT_GRID_SIZE_SPATIUM;
}

void NotationConfiguration::setGridSize(framework::Orientation gridOrientation, int sizeSpatium)
{
    switch (gridOrientation) {
    case framework::Orientation::Horizontal:
        mu::engraving::MScore::setHRaster(sizeSpatium);
        settings()->setSharedValue(HORIZONTAL_GRID_SIZE_KEY, Val(sizeSpatium));
        break;
    case framework::Orientation::Vertical:
        mu::engraving::MScore::setVRaster(sizeSpatium);
        settings()->setSharedValue(VERTICAL_GRID_SIZE_KEY, Val(sizeSpatium));
        break;
    }
}

bool NotationConfiguration::needToShowAddTextErrorMessage() const
{
    return settings()->value(NEED_TO_SHOW_ADD_TEXT_ERROR_MESSAGE_KEY).toBool();
}

void NotationConfiguration::setNeedToShowAddTextErrorMessage(bool show)
{
    settings()->setSharedValue(NEED_TO_SHOW_ADD_TEXT_ERROR_MESSAGE_KEY, Val(show));
}

bool NotationConfiguration::needToShowAddFiguredBassErrorMessage() const
{
    return settings()->value(NEED_TO_SHOW_ADD_FIGURED_BASS_ERROR_MESSAGE_KEY).toBool();
}

void NotationConfiguration::setNeedToShowAddFiguredBassErrorMessage(bool show)
{
    settings()->setSharedValue(NEED_TO_SHOW_ADD_FIGURED_BASS_ERROR_MESSAGE_KEY, Val(show));
}

bool NotationConfiguration::needToShowMScoreError(const std::string& errorKey) const
{
    Settings::Key key(module_name, "ui/dialogs/needToShowMScoreError/" + errorKey);

    settings()->setDefaultValue(key, Val(true));

    return settings()->value(key).toBool();
}

void NotationConfiguration::setNeedToShowMScoreError(const std::string& errorKey, bool show)
{
    Settings::Key key(module_name, "ui/dialogs/needToShowMScoreError/" + errorKey);
    settings()->setSharedValue(key, Val(show));
}

ValCh<int> NotationConfiguration::pianoKeyboardNumberOfKeys() const
{
    return m_pianoKeyboardNumberOfKeys;
}

void NotationConfiguration::setPianoKeyboardNumberOfKeys(int number)
{
    settings()->setSharedValue(PIANO_KEYBOARD_NUMBER_OF_KEYS, Val(number));
}

io::path_t NotationConfiguration::firstScoreOrderListPath() const
{
    return settings()->value(FIRST_SCORE_ORDER_LIST_KEY).toString();
}

void NotationConfiguration::setFirstScoreOrderListPath(const io::path_t& path)
{
    settings()->setSharedValue(FIRST_SCORE_ORDER_LIST_KEY, Val(path.toStdString()));
}

io::path_t NotationConfiguration::secondScoreOrderListPath() const
{
    return settings()->value(SECOND_SCORE_ORDER_LIST_KEY).toString();
}

void NotationConfiguration::setSecondScoreOrderListPath(const io::path_t& path)
{
    settings()->setSharedValue(SECOND_SCORE_ORDER_LIST_KEY, Val(path.toStdString()));
}

mu::io::path_t NotationConfiguration::styleFileImportPath() const
{
    return settings()->value(STYLE_FILE_IMPORT_PATH_KEY).toString();
}

void NotationConfiguration::setStyleFileImportPath(const io::path_t& path)
{
    settings()->setSharedValue(STYLE_FILE_IMPORT_PATH_KEY, Val(path.toStdString()));
}
