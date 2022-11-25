/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "musesamplermodule.h"

#include "modularity/ioc.h"
#include "audio/isynthresolver.h"

#include "ui/iuiactionsregister.h"

#include "imusesamplerinfo.h"
#include "internal/musesamplerconfiguration.h"
#include "internal/musesamplerresolver.h"
#include "internal/musesampleruiactions.h"
#include "internal/musesampleractioncontroller.h"

using namespace mu;
using namespace mu::modularity;
using namespace mu::musesampler;

static std::shared_ptr<MuseSamplerConfiguration> s_configuration = std::make_shared<MuseSamplerConfiguration>();
static std::shared_ptr<MuseSamplerActionController> s_actionController = std::make_shared<MuseSamplerActionController>();
static std::shared_ptr<MuseSamplerResolver> s_resolver = std::make_shared<MuseSamplerResolver>();

std::string MuseSamplerModule::moduleName() const
{
    return "musesampler";
}

void MuseSamplerModule::registerExports()
{
    ioc()->registerExport<IMuseSamplerConfiguration>(moduleName(), s_configuration);
    ioc()->registerExport<IMuseSamplerInfo>(moduleName(), s_resolver);
}

void MuseSamplerModule::resolveImports()
{
    auto synthResolver = ioc()->resolve<audio::synth::ISynthResolver>(moduleName());

    if (synthResolver) {
        synthResolver->registerResolver(audio::AudioSourceType::MuseSampler, s_resolver);
    }

    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<MuseSamplerUiActions>());
    }
}

void MuseSamplerModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (framework::IApplication::RunMode::Editor != mode) {
        return;
    }

    s_actionController->init();
    s_resolver->init();
}
