/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and redistribute it and/or modify
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

#include <QStringList>

#include "async/asyncable.h"
#include "async/notification.h"
#include "modularity/ioc.h"

#include "iprojectsession.h"
#include "iopenprojectscenario.h"

namespace mu::project {
//! NOTE Information about the notation page for the navigation between the pages of the window
class NotationPageInfo : public muse::Contextable, public muse::async::Asyncable
{
    muse::ContextInject<IProjectSession> projectSession = { this };
    muse::ContextInject<IOpenProjectScenario> openProjectScenario = { this };

public:
    NotationPageInfo(const muse::modularity::ContextPtr& iocCtx);

    //! NOTE The uri to open the notation page with the current project
    QString uri() const;

    //! NOTE All the uris of the notation page
    static QStringList pageUris();

    bool isProjectOpened() const;

    muse::async::Notification changed() const;

private:
    muse::async::Notification m_changed;
};
}
