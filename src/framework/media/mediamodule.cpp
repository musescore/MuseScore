/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "mediamodule.h"

#include "modularity/ioc.h"

#include "imediaconfiguration.h"
#include "ivideoencoderresolver.h"
#include "internal/mediaconfiguration.h"
#include "internal/videoencoderresolver.h"

using namespace muse::media;
using namespace muse::modularity;

std::string MediaModule::moduleName() const
{
    return "media";
}

void MediaModule::registerExports()
{
    m_configuration = std::make_shared<MediaConfiguration>(iocContext());
    m_videoEncoderResolver = std::make_shared<VideoEncoderResolver>();

    ioc()->registerExport<IMediaConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IVideoEncoderResolver>(moduleName(), m_videoEncoderResolver);
}

void MediaModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
    m_videoEncoderResolver->init();
}

void MediaModule::onDeinit()
{
    m_videoEncoderResolver->deinit();
}
