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
    NOT_IMPLEMENTED;
    return RetVal<QByteArray>();
}

mu::Ret WorkspacesDataProvider::setRawData(DataKey key, const QByteArray& data)
{
    NOT_IMPLEMENTED;
    return Ret();
}

mu::RetVal<Data> WorkspacesDataProvider::data(DataKey key) const
{
    NOT_IMPLEMENTED;
    return RetVal<Data>();
}

mu::Ret WorkspacesDataProvider::setData(DataKey key, const Data& data)
{
    NOT_IMPLEMENTED;
    return Ret();
}

mu::async::Notification WorkspacesDataProvider::dataChanged(DataKey key)
{
    NOT_IMPLEMENTED;
    return async::Notification();
}
