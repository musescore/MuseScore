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
#include "palettestubmodule.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"
#include "ui/iinteractiveuriregister.h"

#include "paletteadapterstub.h"
#include "paletteconfigurationstub.h"

using namespace mu::palette;
using namespace mu::framework;
using namespace mu::ui;

static void palette_init_qrc()
{
    Q_INIT_RESOURCE(palette);
}

std::string PaletteStubModule::moduleName() const
{
    return "palette_stub";
}

void PaletteStubModule::registerExports()
{
    ioc()->registerExport<IPaletteAdapter>(moduleName(), new PaletteAdapterStub());
    ioc()->registerExport<IPaletteConfiguration>(moduleName(), new PaletteConfigurationStub());
}

void PaletteStubModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://palette/properties"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/Palette/PalettePropertiesDialog.qml"));

        ir->registerUri(Uri("musescore://palette/cellproperties"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/Palette/PaletteCellPropertiesDialog.qml"));
    }
}

void PaletteStubModule::registerResources()
{
    palette_init_qrc();
}

void PaletteStubModule::registerUiTypes()
{
    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(palette_QML_IMPORT);
}
