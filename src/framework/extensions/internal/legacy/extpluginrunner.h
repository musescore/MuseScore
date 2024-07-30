/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#ifndef MUSE_EXTENSIONS_EXTPLUGINRUNNER_H
#define MUSE_EXTENSIONS_EXTPLUGINRUNNER_H

#include "global/types/ret.h"

#include "../../extensionstypes.h"

#include "modularity/ioc.h"
#include "../../iextensionsuiengine.h"

namespace muse::extensions::legacy {
//! NOTE Run old plugins without UI
//! But they are still qml files, so they are run as qml
class ExtPluginRunner : public Injectable
{
    Inject<IExtensionsUiEngine> engine = { this };

public:
    ExtPluginRunner(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    Ret run(const Action& action);
};
}

#endif // MUSE_EXTENSIONS_EXTPLUGINRUNNER_H
