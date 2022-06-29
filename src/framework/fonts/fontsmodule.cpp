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
#include "fontsmodule.h"

#include <QtGlobal>

#include "modularity/ioc.h"
#include "internal/fontssetup.h"

using namespace mu::fonts;

static std::shared_ptr<FontsSetup> s_fontsSetup = std::make_shared<FontsSetup>();

static void init_fonts_qrc()
{
    Q_INIT_RESOURCE(fonts_Leland);
    Q_INIT_RESOURCE(fonts_Bravura);
    Q_INIT_RESOURCE(fonts_Campania);
    Q_INIT_RESOURCE(fonts_Edwin);
    Q_INIT_RESOURCE(fonts_FreeSans);
    Q_INIT_RESOURCE(fonts_FreeSerif);
    Q_INIT_RESOURCE(fonts_Gootville);
    Q_INIT_RESOURCE(fonts_Leland);
    Q_INIT_RESOURCE(fonts_MScore);
    Q_INIT_RESOURCE(fonts_MuseJazz);
    Q_INIT_RESOURCE(fonts_Smufl);
    Q_INIT_RESOURCE(fonts_Petaluma);
    Q_INIT_RESOURCE(fonts_FinaleMaestro);
    Q_INIT_RESOURCE(fonts_FinaleBroadway);
    Q_INIT_RESOURCE(fonts_Tabulature);
}

std::string FontsModule::moduleName() const
{
    return "fonts";
}

void FontsModule::registerResources()
{
    init_fonts_qrc();
}

void FontsModule::onInit(const mu::framework::IApplication::RunMode&)
{
    s_fontsSetup->setup();
}
