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
#include "palettestubmodule.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"
#include "ui/iinteractiveuriregister.h"

#include "paletteconfigurationstub.h"

using namespace mu::palette;
using namespace mu::modularity;
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
