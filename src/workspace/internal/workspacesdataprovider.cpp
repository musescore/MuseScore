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
#include "workspacesdataprovider.h"

#include "log.h"

using namespace mu::workspace;

mu::RetVal<QByteArray> WorkspacesDataProvider::rawData(DataKey key) const
{
    IWorkspacePtr current = manager()->currentWorkspace();
    IF_ASSERT_FAILED(current) {
        return RetVal<QByteArray>(make_ret(Ret::Code::InternalError));
    }

    if (current->isManaged(key)) {
        return current->rawData(key);
    }

    IWorkspacePtr def = manager()->defaultWorkspace();
    IF_ASSERT_FAILED(def) {
        return RetVal<QByteArray>(make_ret(Ret::Code::InternalError));
    }

    return def->rawData(key);
}

mu::Ret WorkspacesDataProvider::setRawData(DataKey key, const QByteArray& data)
{
    IWorkspacePtr current = manager()->currentWorkspace();
    IF_ASSERT_FAILED(current) {
        return make_ret(Ret::Code::InternalError);
    }

    if (current->isManaged(key)) {
        return current->setRawData(key, data);
    }

    IWorkspacePtr def = manager()->defaultWorkspace();
    IF_ASSERT_FAILED(def) {
        return make_ret(Ret::Code::InternalError);
    }

    return def->setRawData(key, data);
}

mu::async::Notification WorkspacesDataProvider::dataChanged(DataKey key)
{
    NOT_IMPLEMENTED;
    return async::Notification();
}
