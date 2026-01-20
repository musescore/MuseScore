/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

 #include "multiwindowprovider.h"

 #include "log.h"

using namespace muse;
using namespace mu::app;

static int m_lastId = 0;

bool MultiWindowProvider::openNewAppInstance(const QStringList& args)
{
    LOGDA() << args;

    modularity::ContextPtr ctx = std::make_shared<modularity::Context>();
    ++m_lastId;
    ctx->id = m_lastId;

    application()->newSession(ctx);

    return true;
}
