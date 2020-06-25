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
#include "scenenotationconfiguration.h"

#include "log.h"
#include "settings.h"

using namespace mu::scene::notation;
using namespace mu::framework;
using namespace mu::async;

static std::string module_name("notation_scene");

static const Settings::Key BACKGROUND_COLOR(module_name, "ui/canvas/background/color");
static const Settings::Key SELECTION_PROXIMITY(module_name, "ui/canvas/misc/selectionProximity");

void SceneNotationConfiguration::init()
{
    settings()->addItem(BACKGROUND_COLOR, Val(QColor("#D6E0E9")));
    settings()->valueChanged(BACKGROUND_COLOR).onReceive(nullptr, [this](const Val& val) {
        LOGD() << "BACKGROUND_COLOR changed: " << val.toString();
        m_backgroundColorChanged.send(val.toQColor());
    });

    settings()->addItem(SELECTION_PROXIMITY, Val(6));
}

QColor SceneNotationConfiguration::backgroundColor() const
{
    return settings()->value(BACKGROUND_COLOR).toQColor();
}

Channel<QColor> SceneNotationConfiguration::backgroundColorChanged()
{
    return m_backgroundColorChanged;
}

int SceneNotationConfiguration::selectionProximity() const
{
    return settings()->value(SELECTION_PROXIMITY).toInt();
}
