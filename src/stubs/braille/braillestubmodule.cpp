/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "braillestubmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "brailleconfigurationstub.h"

using namespace muse;
using namespace mu::braille;

static void braille_init_qrc()
{
    Q_INIT_RESOURCE(braille);
}

std::string BrailleModule::moduleName() const
{
    return "braille";
}

void BrailleModule::registerResources()
{
    braille_init_qrc();
}

void BrailleModule::registerExports()
{
    ioc()->registerExport<IBrailleConfiguration>(moduleName(), new BrailleConfigurationStub());
}

void BrailleModule::registerUiTypes()
{
    ioc()->resolve<muse::ui::IUiEngine>(moduleName())->addSourceImportPath(braille_QML_IMPORT);
}
