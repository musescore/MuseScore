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
#include "videoexportmodule.h"

#include "modularity/ioc.h"

#include "internal/videoexportconfiguration.h"
#include "internal/videowriter.h"

#include "project/inotationwritersregister.h"

#include "log.h"

using namespace mu::iex::videoexport;
using namespace mu::project;
using namespace mu::modularity;

static std::shared_ptr<VideoExportConfiguration> s_configuration = std::make_shared<VideoExportConfiguration>();

std::string VideoExportModule::moduleName() const
{
    return "iex_videoexport";
}

void VideoExportModule::registerExports()
{
    ioc()->registerExport<VideoExportConfiguration>(moduleName(), s_configuration);
}

void VideoExportModule::resolveImports()
{
    auto writers = ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "mp4" }, std::make_shared<VideoWriter>());
    }
}
