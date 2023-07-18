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
#ifndef MU_VST_VSTPLUGINSSCANNER_H
#define MU_VST_VSTPLUGINSSCANNER_H

#include "audio/iaudiopluginsscanner.h"

#include "modularity/ioc.h"
#include "ivstconfiguration.h"
#include "io/ifilesystem.h"

namespace mu::vst {
class VstPluginsScanner : public audio::IAudioPluginsScanner
{
    INJECT(IVstConfiguration, configuration)
    INJECT(io::IFileSystem, fileSystem)

public:
    io::paths_t scanPlugins() const override;

private:
    io::paths_t pluginPathsFromCustomLocations(const io::paths_t& customPaths) const;
};
}

#endif // MU_VST_VSTPLUGINSSCANNER_H
