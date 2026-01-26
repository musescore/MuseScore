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

 #include "oneprocessprovider.h"

 #include "log.h"

using namespace muse::mi;

int OneProcessProvider::windowCount() const
{
    return application()->contextCount();
}

bool OneProcessProvider::isProjectAlreadyOpened(const muse::io::path_t& projectPath) const
{
    NOT_IMPLEMENTED;
    UNUSED(projectPath);
    return false;
}

void OneProcessProvider::activateWindowWithProject(const muse::io::path_t& projectPath)
{
    NOT_IMPLEMENTED;
    UNUSED(projectPath);
}

bool OneProcessProvider::isHasWindowWithoutProject() const
{
    NOT_IMPLEMENTED;
    return false;
}

void OneProcessProvider::activateWindowWithoutProject(const QStringList& args)
{
    NOT_IMPLEMENTED;
    UNUSED(args);
}

bool OneProcessProvider::openNewWindow(const QStringList& args)
{
    LOGDA() << args;

    application()->setupNewContext();

    return true;
}
