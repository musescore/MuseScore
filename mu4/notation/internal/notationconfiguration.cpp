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

#include "log.h"
#include "settings.h"

#include "io/path.h"
#include "libmscore/preferences.h"

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

void NotationConfiguration::init()
{
    settings()->addItem(ANCHORLINE_COLOR, Val(QColor("#C31989")));

    settings()->addItem(BACKGROUND_COLOR, Val(QColor("#142433")));
    settings()->valueChanged(BACKGROUND_COLOR).onReceive(nullptr, [this](const Val& val) {
        LOGD() << "BACKGROUND_COLOR changed: " << val.toString();
        m_backgroundColorChanged.send(val.toQColor());
    });

    settings()->addItem(FOREGROUND_COLOR, Val(QColor("#f9f9f9")));
    settings()->valueChanged(FOREGROUND_COLOR).onReceive(nullptr, [this](const Val& val) {
        LOGD() << "FOREGROUND_COLOR changed: " << val.toString();
        m_foregroundColorChanged.send(foregroundColor());
    });

    settings()->addItem(FOREGROUND_USE_USER_COLOR, Val(true));
    settings()->valueChanged(FOREGROUND_USE_USER_COLOR).onReceive(nullptr, [this](const Val& val) {
        LOGD() << "FOREGROUND_USE_USER_COLOR changed: " << val.toString();
        m_foregroundColorChanged.send(foregroundColor());
    });

    settings()->addItem(CURRENT_ZOOM, Val(100));
    settings()->valueChanged(CURRENT_ZOOM).onReceive(nullptr, [this](const Val& val) {
        m_currentZoomChanged.send(val.toInt());
    });

    settings()->addItem(SELECTION_PROXIMITY, Val(6));
    settings()->addItem(IS_MIDI_INPUT_ENABLED, Val(false));

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

Channel<QColor> NotationConfiguration::backgroundColorChanged() const
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

QColor NotationConfiguration::defaultForegroundColor() const
{
    return settings()->defaultValue(FOREGROUND_COLOR).toQColor();
}

QColor NotationConfiguration::foregroundColor() const
{
    if (settings()->value(FOREGROUND_USE_USER_COLOR).toBool()) {
        return settings()->value(FOREGROUND_COLOR).toQColor();
    }
    return defaultForegroundColor();
}

Channel<QColor> NotationConfiguration::foregroundColorChanged() const
{
    return m_foregroundColorChanged;
}

io::path NotationConfiguration::foregroundWallpaper() const
{
    return settings()->defaultValue(FOREGROUND_WALLPAPER).toQString();
}

QColor NotationConfiguration::playbackCursorColor() const
{
    //! TODO Figure out what color to use
    QColor c("#ff0000");
    c.setAlpha(50);
    return c;
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
    return uiConfiguration()->fontSize();
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
