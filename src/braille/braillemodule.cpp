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

#include "braillemodule.h"

#include "modularity/ioc.h"
#include "project/inotationwritersregister.h"

#include "internal/brailleconfiguration.h"
#include "internal/brailleconverter.h"
#include "internal/notationbraille.h"
#include "internal/braillewriter.h"

#include "inotationbraille.h"

using namespace muse;
using namespace mu::braille;
using namespace mu::engraving;
using namespace mu::notation;
using namespace mu::project;

static const std::string mname("braille");

std::string BrailleModule::moduleName() const
{
    return mname;
}

void BrailleModule::registerExports()
{
    m_brailleConfiguration = std::make_shared<BrailleConfiguration>();

    globalIoc()->registerExport<IBrailleConfiguration>(mname, m_brailleConfiguration);
}

void BrailleModule::resolveImports()
{
    auto writers = globalIoc()->resolve<INotationWritersRegister>(mname);
    if (writers) {
        writers->reg({ "brf" }, std::make_shared<BrailleWriter>());
    }
}

void BrailleModule::onInit(const IApplication::RunMode&)
{
    m_brailleConfiguration->init();
}

muse::modularity::IContextSetup* BrailleModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new BrailleModuleContext(ctx);
}

// Context

void BrailleModuleContext::registerExports()
{
    m_brailleConverter = std::make_shared<BrailleConverter>();
    m_notationBraille = std::make_shared<NotationBraille>(iocContext());

    ioc()->registerExport<IBrailleConverter>(mname, m_brailleConverter);
    ioc()->registerExport<INotationBraille>(mname, m_notationBraille);
}

void BrailleModuleContext::onInit(const muse::IApplication::RunMode&)
{
    m_notationBraille->init();
}
