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
#include "palettemodule.h"

#include <QQmlEngine>
#include "modularity/ioc.h"

//#include "view/paletterootmodel.h"

//#include "internal/palette/paletteworkspace.h"

using namespace mu::scene::palette;

static void palette_init_qrc()
{
    Q_INIT_RESOURCE(palette);
}

std::string PaletteModule::moduleName() const
{
    return "palette";
}

void PaletteModule::registerExports()
{
    //framework::ioc()->registerExport<ISceneNotationConfiguration>(moduleName(), m_configuration);
}

void PaletteModule::resolveImports()
{
}

void PaletteModule::registerResources()
{
    palette_init_qrc();
}

void PaletteModule::registerUiTypes()
{
//    qmlRegisterType<PaletteRootModel>("MuseScore.Palette", 1, 0, "PaletteRootModel");

//    qmlRegisterType<PaletteWorkspace>("MuseScore.Palette", 1, 0, "PaletteWorkspace");
}

void PaletteModule::onInit()
{
}
