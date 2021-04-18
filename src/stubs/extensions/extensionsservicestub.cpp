//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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
