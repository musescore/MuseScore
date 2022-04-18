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

#include "internal/musesamplerconfiguration.h"
#include "internal/musesamplerresolver.h"

using namespace mu;
using namespace mu::modularity;
using namespace mu::musesampler;

static std::shared_ptr<MuseSamplerConfiguration> s_configuration = std::make_shared<MuseSamplerConfiguration>();

std::string MuseSamplerModule::moduleName() const
{
    return "musesampler";
}

void MuseSamplerModule::registerExports()
{
    ioc()->registerExport(moduleName(), s_configuration);
}

void MuseSamplerModule::resolveImports()
{
    auto synthResolver = ioc()->resolve<audio::synth::ISynthResolver>(moduleName());

    if (synthResolver) {
        synthResolver->registerResolver(audio::AudioSourceType::MuseSampler, std::make_shared<MuseSamplerResolver>());
    }
}
