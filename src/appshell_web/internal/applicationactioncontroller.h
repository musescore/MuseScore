/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_APPSHELL_APPLICATIONCONTROLLER_H
#define MU_APPSHELL_APPLICATIONCONTROLLER_H

#include <QObject>

#include "actions/actionable.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"

namespace mu::appshell {
class ApplicationActionController : public QObject, public muse::Injectable, public muse::actions::Actionable, public muse::async::Asyncable
{
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };

public:
    ApplicationActionController(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    void preInit();
    void init();

private:
};
}

#endif // MU_APPSHELL_APPLICATIONCONTROLLER_H
