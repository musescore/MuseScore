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
#include "filesystemstub.h"

using namespace mu::system;
using namespace mu;

Ret FileSystemStub::exists(const io::path&) const
{
    return make_ret(Ret::Code::NotSupported);
}

Ret FileSystemStub::remove(const io::path&) const
{
    return make_ret(Ret::Code::NotSupported);
}

Ret FileSystemStub::makePath(const io::path&) const
{
    return make_ret(Ret::Code::NotSupported);
}

RetVal<io::paths> FileSystemStub::scanFiles(const io::path&, const QStringList&, IFileSystem::ScanMode) const
{
    RetVal<io::paths> result;
    result.ret = make_ret(Ret::Code::NotSupported);
    return result;
}

RetVal<QByteArray> FileSystemStub::readFile(const io::path&) const
{
    RetVal<QByteArray> result;
    result.ret = make_ret(Ret::Code::NotSupported);
    return result;
}
