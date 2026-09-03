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
#include "toastmodule.h"

#include "modularity/ioc.h"

#include "internal/toastservice.h"
#include "internal/toastprovider.h"

using namespace muse::toast;
using namespace muse;
using namespace muse::modularity;

static const std::string mname("toast");

std::string ToastModule::moduleName() const
{
    return mname;
}

void ToastModule::registerExports()
{
    m_toastProvider = std::make_shared<ToastProvider>();
    m_toastService = std::make_shared<ToastService>();

    globalIoc()->registerExport<IToastProvider>(mname, m_toastProvider);
    globalIoc()->registerExport<IToastService>(mname, m_toastService);
}

IContextSetup* ToastModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new ToastContext(ctx);
}

// =====================================================
// ToastContext
// =====================================================

void ToastContext::onDeinit()
{
}
