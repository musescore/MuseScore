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

//! TODO Understand the conflict between "use color" and "use user color"
static const Settings::Key FOREGROUND_USE_COLOR(module_name, "ui/canvas/foreground/useColor");
static const Settings::Key FOREGROUND_USE_USER_COLOR(module_name, "ui/canvas/foreground/useColor");

static const Settings::Key FOREGROUND_COLOR(module_name, "ui/canvas/foreground/color");
static const Settings::Key FOREGROUND_WALLPAPER(module_name, "ui/canvas/foreground/wallpaper");

static const Settings::Key SELECTION_PROXIMITY(module_name, "ui/canvas/misc/selectionProximity");

static const Settings::Key CURRENT_ZOOM(module_name, "ui/canvas/misc/currentZoom");

static const Settings::Key STYLES_DIR_KEY(module_name, "application/paths/myStyles");

static const Settings::Key IS_MIDI_INPUT_ENABLED(module_name, "io/midi/enableInput");

static const Settings::Key TOOLBAR_KEY(module_name, "ui/toolbar/");

void NotationConfiguration::init()
{
    settings()->setDefaultValue(ANCHORLINE_COLOR, Val(QColor("#C31989")));

    settings()->setDefaultValue(BACKGROUND_COLOR, Val(theme()->backgroundSecondaryColor()));
    settings()->valueChanged(BACKGROUND_COLOR).onReceive(nullptr, [this](const Val& val) {
        LOGD() << "BACKGROUND_COLOR changed: " << val.toString();
        m_backgroundColorChanged.send(val.toQColor());
    });

    settings()->setDefaultValue(FOREGROUND_COLOR, Val(theme()->backgroundPrimaryColor()));
    settings()->valueChanged(FOREGROUND_COLOR).onReceive(nullptr, [this](const Val& val) {
        LOGD() << "FOREGROUND_COLOR changed: " << val.toString();
        m_foregroundColorChanged.send(foregroundColor());
    });

    theme()->themeChanged().onNotify(this, [this]() {
        m_backgroundColorChanged.send(backgroundColor());
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

    // libmscore
    preferences().setBackupDirPath(globalConfiguration()->backupPath().toQString());
}

QColor NotationConfiguration::anchorLineColor() const
{
    return settings()->value(ANCHORLINE_COLOR).toQColor();
}

QColor NotationConfiguration::backgroundColor() const
{
    QColor color = resolveColor(BACKGROUND_COLOR);
    if (!color.isValid()) {
        color = theme()->backgroundSecondaryColor();
    }

    return color;
}

async::Channel<QColor> NotationConfiguration::backgroundColorChanged() const
{
    return m_backgroundColorChanged;
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

    QColor color = resolveColor(FOREGROUND_COLOR);
    if (!color.isValid()) {
        color = theme()->backgroundSecondaryColor();
    }

    return color;
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

int NotationConfiguration::fontSize() const
{
    return uiConfiguration()->fontSize(IUiConfiguration::FontSizeType::BODY);
}

io::path NotationConfiguration::stylesDirPath() const
{
    return settings()->value(STYLES_DIR_KEY).toString();
}

bool NotationConfiguration::isMidiInputEnabled() const
{
    return settings()->value(IS_MIDI_INPUT_ENABLED).toBool();
}

float NotationConfiguration::guiScaling() const
{
    return uiConfiguration()->guiScaling();
}

float NotationConfiguration::notationScaling() const
{
    return uiConfiguration()->physicalDotsPerInch() / Ms::DPI;
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

QColor NotationConfiguration::resolveColor(const Settings::Key& key) const
{
    QColor color = settings()->value(key).toQColor();
    QColor defaultColor = settings()->defaultValue(key).toQColor();
    if (!color.isValid() || color == defaultColor) {
        return QColor();
    }

    return color;
}
