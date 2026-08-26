/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "extensions/iextensioncontextresolver.h"

#include "global/async/asyncable.h"

#include "global/modularity/ioc.h"
#include "../iglobalcontext.h"

namespace mu::context {
class ExtensionContextResolver : public muse::extensions::IExtensionContextResolver, public muse::Contextable, public muse::async::Asyncable
{
    muse::ContextInject<IGlobalContext> globalContext = { this };

public:
    ExtensionContextResolver(const muse::modularity::ContextPtr& ctx)
        : muse::Contextable(ctx) {}

    void init();

    bool isContextAllowed(const std::string& context) const override;

    muse::async::Notification contextChanged() const override;

private:
    muse::async::Notification m_contextChanged;
};
}
