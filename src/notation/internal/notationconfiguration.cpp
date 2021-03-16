//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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

// Global variable
namespace Ms {
QString revision;
}
// -----

static const std::string module_name("notation");

static const Settings::Key ANCHORLINE_COLOR(module_name, "ui/score/voice4/color");

static const Settings::Key BACKGROUND_COLOR(module_name, "ui/canvas/background/color");
static const Settings::Key BACKGROUND_WALLPAPER_PATH(module_name, "ui/canvas/background/wallpaper");

//! TODO Understand the conflict between "use color" and "use user color"
static const Settings::Key FOREGROUND_USE_COLOR(module_name, "ui/canvas/foreground/useColor");
static const Settings::Key FOREGROUND_USE_USER_COLOR(module_name, "ui/canvas/foreground/useColor");

static const Settings::Key FOREGROUND_COLOR(module_name, "ui/canvas/foreground/color");
static const Settings::Key FOREGROUND_WALLPAPER(module_name, "ui/canvas/foreground/wallpaper");

static const Settings::Key SELECTION_PROXIMITY(module_name, "ui/canvas/misc/selectionProximity");

static const Settings::Key CURRENT_ZOOM(module_name, "ui/canvas/misc/currentZoom");

static const Settings::Key STYLES_DIR_KEY(module_name, "application/paths/myStyles");

static const Settings::Key IS_MIDI_INPUT_ENABLED(module_name, "io/midi/enableInput");
static const Settings::Key IS_AUTOMATICALLY_PAN_ENABLED(module_name, "application/playback/panPlayback");
static const Settings::Key IS_PLAY_REPEATS_ENABLED(module_name, "application/playback/playRepeats");
static const Settings::Key IS_METRONOME_ENABLED(module_name, "application/playback/metronomeEnabled");
static const Settings::Key IS_COUNT_IN_ENABLED(module_name, "application/playback/countInEnabled");

static const Settings::Key TOOLBAR_KEY(module_name, "ui/toolbar/");

static const Settings::Key IS_CANVAS_ORIENTATION_VERTICAL_KEY(module_name, "ui/canvas/scroll/verticalOrientation");

void NotationConfiguration::init()
{
    settings()->setDefaultValue(ANCHORLINE_COLOR, Val(QColor("#C31989")));

    settings()->setDefaultValue(BACKGROUND_COLOR, Val(QColor("#385f94")));
    settings()->valueChanged(BACKGROUND_COLOR).onReceive(nullptr, [this](const Val&) {
        m_backgroundChanged.notify();
    });

    settings()->setDefaultValue(BACKGROUND_WALLPAPER_PATH, Val());
    settings()->valueChanged(BACKGROUND_WALLPAPER_PATH).onReceive(nullptr, [this](const Val&) {
        m_backgroundChanged.notify();
    });

    settings()->setDefaultValue(FOREGROUND_COLOR, Val(QColor("#f9f9f9")));
    settings()->valueChanged(FOREGROUND_COLOR).onReceive(nullptr, [this](const Val& val) {
        LOGD() << "FOREGROUND_COLOR changed: " << val.toString();
        m_foregroundColorChanged.send(foregroundColor());
    });

    settings()->setDefaultValue(FOREGROUND_USE_USER_COLOR, Val(true));
    settings()->valueChanged(FOREGROUND_USE_USER_COLOR).onReceive(nullptr, [this](const Val& val) {
        LOGD() << "FOREGROUND_USE_USER_COLOR changed: " << val.toString();
        m_foregroundColorChanged.send(foregroundColor());
    });

    settings()->setDefaultValue(CURRENT_ZOOM, Val(100));
    settings()->valueChanged(CURRENT_ZOOM).onReceive(nullptr, [this](const Val& val) {
        m_currentZoomChanged.send(val.toInt());
    });

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

    // libmscore
    preferences().setBackupDirPath(globalConfiguration()->backupPath().toQString());
}

QColor NotationConfiguration::anchorLineColor() const
{
    return settings()->value(ANCHORLINE_COLOR).toQColor();
}

QColor NotationConfiguration::backgroundColor() const
{
    return settings()->value(BACKGROUND_COLOR).toQColor();
}

void NotationConfiguration::setBackgroundColor(const QColor& color)
{
    settings()->setValue(BACKGROUND_COLOR, Val(color));
}

io::path NotationConfiguration::backgroundWallpaperPath() const
{
    return settings()->value(BACKGROUND_WALLPAPER_PATH).toString();
}

void NotationConfiguration::setBackgroundWallpaperPath(const io::path& path)
{
    settings()->setValue(BACKGROUND_WALLPAPER_PATH, Val(path.toStdString()));
}

async::Notification NotationConfiguration::backgroundChanged() const
{
    return m_backgroundChanged;
}

QColor NotationConfiguration::pageColor() const
{
    return QColor("#ffffff");
}

QColor NotationConfiguration::borderColor() const
{
    return QColor(0, 0, 0, 102);
}

int NotationConfiguration::borderWidth() const
{
    return 1;
}

bool NotationConfiguration::foregroundUseColor() const
{
    return settings()->value(FOREGROUND_USE_COLOR).toBool();
}

QColor NotationConfiguration::foregroundColor() const
{
    if (settings()->value(FOREGROUND_USE_USER_COLOR).toBool()) {
        return settings()->value(FOREGROUND_COLOR).toQColor();
    }

    return settings()->defaultValue(FOREGROUND_COLOR).toQColor();
}

async::Channel<QColor> NotationConfiguration::foregroundColorChanged() const
{
    return m_foregroundColorChanged;
}

io::path NotationConfiguration::foregroundWallpaper() const
{
    return settings()->defaultValue(FOREGROUND_WALLPAPER).toQString();
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

    return Ms::MScore::selectColor[voiceIndex];
}

int NotationConfiguration::selectionProximity() const
{
    return settings()->value(SELECTION_PROXIMITY).toInt();
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
    settings()->setValue(CURRENT_ZOOM, Val(zoomPercentage));
}

std::string NotationConfiguration::fontFamily() const
{
    return uiConfiguration()->fontFamily();
}

int NotationConfiguration::fontSize() const
{
    return uiConfiguration()->fontSize(ui::IUiConfiguration::FontSizeType::BODY);
}

io::path NotationConfiguration::stylesDirPath() const
{
    return settings()->value(STYLES_DIR_KEY).toString();
}

bool NotationConfiguration::isMidiInputEnabled() const
{
    return settings()->value(IS_MIDI_INPUT_ENABLED).toBool();
}

void NotationConfiguration::setIsMidiInputEnabled(bool enabled)
{
    settings()->setValue(IS_MIDI_INPUT_ENABLED, Val(enabled));
}

bool NotationConfiguration::isAutomaticallyPanEnabled() const
{
    return settings()->value(IS_AUTOMATICALLY_PAN_ENABLED).toBool();
}

void NotationConfiguration::setIsAutomaticallyPanEnabled(bool enabled)
{
    settings()->setValue(IS_AUTOMATICALLY_PAN_ENABLED, Val(enabled));
    Ms::MScore::panPlayback = enabled;
}

bool NotationConfiguration::isPlayRepeatsEnabled() const
{
    return settings()->value(IS_PLAY_REPEATS_ENABLED).toBool();
}

void NotationConfiguration::setIsPlayRepeatsEnabled(bool enabled)
{
    settings()->setValue(IS_PLAY_REPEATS_ENABLED, Val(enabled));
    Ms::MScore::playRepeats = enabled;
}

bool NotationConfiguration::isMetronomeEnabled() const
{
    return settings()->value(IS_METRONOME_ENABLED).toBool();
}

void NotationConfiguration::setIsMetronomeEnabled(bool enabled)
{
    settings()->setValue(IS_METRONOME_ENABLED, Val(enabled));
}

bool NotationConfiguration::isCountInEnabled() const
{
    return settings()->value(IS_COUNT_IN_ENABLED).toBool();
}

void NotationConfiguration::setIsCountInEnabled(bool enabled)
{
    settings()->setValue(IS_COUNT_IN_ENABLED, Val(enabled));
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

std::vector<std::string> NotationConfiguration::toolbarActions(const std::string& toolbarName) const
{
    return parseToolbarActions(settings()->value(toolbarSettingsKey(toolbarName)).toString());
}

void NotationConfiguration::setToolbarActions(const std::string& toolbarName, const std::vector<std::string>& actions)
{
    QStringList qactions;
    for (const std::string& action: actions) {
        qactions << QString::fromStdString(action);
    }

    Val value(qactions.join(",").toStdString());
    settings()->setValue(toolbarSettingsKey(toolbarName), value);
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
    settings()->setValue(IS_CANVAS_ORIENTATION_VERTICAL_KEY, Val(isVertical));
}

std::vector<std::string> NotationConfiguration::parseToolbarActions(const std::string& actions) const
{
    if (actions.empty()) {
        return {};
    }

    std::vector<std::string> result;

    QStringList actionsList = QString::fromStdString(actions).split(",");
    for (const QString& action: actionsList) {
        result.push_back(action.toStdString());
    }

    return result;
}

Settings::Key NotationConfiguration::toolbarSettingsKey(const std::string& toolbarName) const
{
    Settings::Key toolbarKey = TOOLBAR_KEY;
    toolbarKey.key += toolbarName;
    return toolbarKey;
}
