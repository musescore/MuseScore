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
#include "extensionrunner.h"

#include "scriptengine.h"
#include "../extensionserrors.h"

#include "log.h"

using namespace muse;
using namespace muse::extensions;

ExtensionRunner::ExtensionRunner(const modularity::ContextPtr& iocCtx)
    : m_iocContext(iocCtx)
{
}

Ret ExtensionRunner::run(const Action& action)
{
    ScriptEngine engine(m_iocContext, action.apiversion);
    engine.setScriptPath(action.path);
    Ret ret = engine.evaluate();
    if (!ret) {
        LOGE() << "failed evaluate js script: " << action.path
               << ", err: " << ret.toString();
        return make_ret(Err::ExtLoadError);
    }

    ret = engine.call(action.func);
    if (!ret) {
        LOGE() << "failed call main function of script: " << action.path
               << ", err: " << ret.toString();
        return make_ret(Err::ExtBadFormat);
    }

    return ret;
}
