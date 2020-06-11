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
#include "scenenotationconfigure.h"

#include "settings.h"
#include "notationscenemodule.h"

using namespace mu::scene::notation;
using namespace mu::framework;

static std::string module_name("notation_scene");

static const Settings::Key BACKGROUND_COLOR(module_name, "ui/canvas/background/color");

void SceneNotationConfigure::init()
{
    using Val = Settings::Val;

    settings()->addItem(BACKGROUND_COLOR, Val(QColor("#D6E0E9")));
}

QColor SceneNotationConfigure::backgroundColor() const
{
    return settings()->value(BACKGROUND_COLOR).toColor();
}
