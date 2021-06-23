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
#include "workspace.h"

#include "translation.h"
#include "workspacefile.h"

#include "libmscore/mscore.h"

#include "log.h"

using namespace mu;
using namespace mu::workspace;
using namespace mu::framework;

Workspace::Workspace(const io::path& filePath)
    : m_filePath(filePath)
{
}

std::string Workspace::name() const
{
    return io::basename(m_filePath).toStdString();
}

std::string Workspace::title() const
{
    return name();
}

bool Workspace::isManaged(const Option& key) const
{
    NOT_IMPLEMENTED;
    return false;
}

void Workspace::setIsManaged(const Option& key, bool val) const
{
    NOT_IMPLEMENTED;
}

RetVal<QByteArray> Workspace::readRawData(const std::string& name) const
{
    NOT_IMPLEMENTED;
    return RetVal<QByteArray>();
}

Ret Workspace::writeRawData(const std::string& name, const QByteArray& data)
{
    NOT_IMPLEMENTED;
    return Ret();
}

RetVal<Data> Workspace::readData(const std::string& name) const
{
    NOT_IMPLEMENTED;
    return RetVal<Data>();
}

Ret Workspace::writeData(const std::string& name, const Data& data)
{
    NOT_IMPLEMENTED;
    return Ret();
}

bool Workspace::isInited() const
{
    return m_isInited;
}

io::path Workspace::filePath() const
{
    return m_filePath;
}

Ret Workspace::read()
{
    clear();

    WorkspaceFile file(m_filePath);
    QByteArray data = file.readRootFile();
    if (data.isEmpty()) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ret ret = readWorkspace(data);
    if (!ret) {
        return ret;
    }

    m_isInited = true;

    return make_ret(Ret::Code::Ok);
}

void Workspace::clear()
{
    m_hasUnsavedChanges = false;
    m_isInited = false;
}

Ret Workspace::readWorkspace(const QByteArray& xmlData)
{
    return make_ret(Ret::Code::Ok);
}

Ret Workspace::write()
{
    if (!m_hasUnsavedChanges) {
        return make_ret(Ret::Code::Ok);
    }

    return Ret();
}
