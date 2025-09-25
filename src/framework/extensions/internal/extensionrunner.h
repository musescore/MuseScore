/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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
#ifndef MUSE_EXTENSIONS_EXTENSIONRUNNER_H
#define MUSE_EXTENSIONS_EXTENSIONRUNNER_H

#include "modularity/ioc.h"
#include "global/types/ret.h"
#include "../extensionstypes.h"

namespace muse::extensions {
class ExtensionRunner
{
public:
    ExtensionRunner(const modularity::ContextPtr& iocCtx);

    Ret run(const Action& action);

private:
    const modularity::ContextPtr m_iocContext;
};
}

#endif // MUSE_EXTENSIONS_EXTENSIONRUNNER_H
