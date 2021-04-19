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
#include "extensionsservicestub.h"

using namespace mu::extensions;
using namespace mu;

ValCh<ExtensionsHash> ExtensionsServiceStub::extensions() const
{
    return ValCh<ExtensionsHash>();
}

RetCh<ExtensionProgress> ExtensionsServiceStub::install(const QString&)
{
    RetCh<ExtensionProgress> result;
    result.ret = make_ret(Ret::Code::NotSupported);
    return result;
}

RetCh<ExtensionProgress> ExtensionsServiceStub::update(const QString&)
{
    RetCh<ExtensionProgress> result;
    result.ret = make_ret(Ret::Code::NotSupported);
    return result;
}

Ret ExtensionsServiceStub::uninstall(const QString&)
{
    return make_ret(Ret::Code::NotSupported);
}

RetCh<Extension> ExtensionsServiceStub::extensionChanged() const
{
    RetCh<Extension> result;
    result.ret = make_ret(Ret::Code::NotSupported);
    return result;
}
