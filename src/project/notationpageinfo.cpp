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

#include "notationpageinfo.h"

#include "types/projecturis.h"

using namespace mu::project;

NotationPageInfo::NotationPageInfo(const muse::modularity::ContextPtr& iocCtx)
    : muse::Contextable(iocCtx)
{
    projectSession()->projectChanged().onNotify(this, [this]() {
        m_changed.notify();
    });
}

QString NotationPageInfo::uri() const
{
    return QString::fromStdString(openProjectScenario()->resolveNotationPageUri().toString());
}

QStringList NotationPageInfo::pageUris()
{
    return {
        QString::fromStdString(NOTATION_PAGE_URI.toString()),
        QString::fromStdString(NOTATION_REVIEW_PAGE_URI.toString())
    };
}

bool NotationPageInfo::isProjectOpened() const
{
    return projectSession()->project() != nullptr;
}

muse::async::Notification NotationPageInfo::changed() const
{
    return m_changed;
}
