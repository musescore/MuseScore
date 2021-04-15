/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#include "networkmanagerstub.h"

using namespace mu::network;

mu::Ret NetworkManagerStub::get(const QUrl&, mu::system::IODevice*)
{
    return make_ret(Ret::Code::NotSupported);
}

mu::Ret NetworkManagerStub::head(const QUrl&)
{
    return make_ret(Ret::Code::NotSupported);
}

mu::Ret NetworkManagerStub::post(const QUrl&, mu::system::IODevice*, mu::system::IODevice*)
{
    return make_ret(Ret::Code::NotSupported);
}

mu::Ret NetworkManagerStub::put(const QUrl&, mu::system::IODevice*, mu::system::IODevice*)
{
    return make_ret(Ret::Code::NotSupported);
}

mu::Ret NetworkManagerStub::del(const QUrl&, mu::system::IODevice*)
{
    return make_ret(Ret::Code::NotSupported);
}

mu::framework::ProgressChannel NetworkManagerStub::progressChannel() const
{
    return framework::ProgressChannel();
}

void NetworkManagerStub::abort()
{
}
