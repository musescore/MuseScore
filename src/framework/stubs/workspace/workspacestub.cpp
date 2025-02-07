/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "workspacestub.h"

using namespace muse;
using namespace muse::workspace;

std::string WorkspaceStub::name() const
{
    return std::string();
}

void WorkspaceStub::setName(const std::string&)
{
}

bool WorkspaceStub::isBuiltin() const
{
    return false;
}

bool WorkspaceStub::isEdited() const
{
    return false;
}

RetVal<QByteArray> WorkspaceStub::rawData(const DataKey&) const
{
    return RetVal<QByteArray>(make_ret(Ret::Code::NotImplemented));
}

Ret WorkspaceStub::setRawData(const DataKey&, const QByteArray&)
{
    return make_ret(Ret::Code::NotImplemented);
}

void WorkspaceStub::reset()
{
}

async::Notification WorkspaceStub::reloadNotification()
{
    return async::Notification();
}
