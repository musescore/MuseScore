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

#pragma once

#include "global/types/retval.h"
#include "global/io/path.h"
#include "audio/audiotypes.h"

namespace muse::audioplugins {
class IAudioPluginMetaReader
{
public:
    virtual ~IAudioPluginMetaReader() = default;

    virtual bool canReadMeta(const io::path_t& pluginPath) const = 0;
    virtual RetVal<audio::AudioResourceMetaList> readMeta(const io::path_t& pluginPath) const = 0;
};

using IAudioPluginMetaReaderPtr = std::shared_ptr<IAudioPluginMetaReader>;
}
