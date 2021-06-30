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

Ret FileSystemStub::copy(const io::path&, const io::path&, bool replace) const
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

Ret FileSystemStub::writeToFile(const io::path&, const QByteArray&) const
{
    return make_ret(Ret::Code::NotSupported);
}
