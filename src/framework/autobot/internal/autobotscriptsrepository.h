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
#ifndef MUSE_AUTOBOT_AUTOBOTSCRIPTSREPOSITORY_H
#define MUSE_AUTOBOT_AUTOBOTSCRIPTSREPOSITORY_H

#include "../iautobotscriptsrepository.h"

#include "modularity/ioc.h"
#include "../iautobotconfiguration.h"
#include "io/ifilesystem.h"

namespace muse::autobot {
class AutobotScriptsRepository : public IAutobotScriptsRepository, public Injectable
{
    Inject<IAutobotConfiguration> configuration = { this };
    GlobalInject<io::IFileSystem> fileSystem;
public:
    AutobotScriptsRepository(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    Scripts scripts() const override;
};
}

#endif // MUSE_AUTOBOT_AUTOBOTSCRIPTSREPOSITORY_H
