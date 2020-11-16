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
#include "workspace.h"

#include "log.h"
#include "translation.h"
#include "workspacefile.h"

#include "global/xmlreader.h"
#include "global/xmlwriter.h"

#include <QBuffer>

using namespace mu;
using namespace mu::workspace;
using namespace mu::framework;

static constexpr std::string_view SOURCE_TAG("source");

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

AbstractDataPtr Workspace::data(WorkspaceTag tag, const std::string& name) const
{
    DataKey key { tag, name };
    auto it = m_data.find(key);
    if (it != m_data.end()) {
        return it->second;
    }
    return nullptr;
}

Val Workspace::settingValue(const std::string& key) const
{
    std::shared_ptr<AbstractData> d = data(WorkspaceTag::Preferences, "");
    if (!d) {
        return Val();
    }

    SettingsData* sd = dynamic_cast<SettingsData*>(d.get());
    IF_ASSERT_FAILED(sd) {
        return Val();
    }

    auto it = sd->vals.find(key);
    if (it == sd->vals.end()) {
        return Val();
    }

    return it->second;
}

std::vector<std::string> Workspace::toolbarActions(const std::string& toolbarName) const
{
    AbstractDataPtr d = data(WorkspaceTag::Toolbar, toolbarName);
    if (!d) {
        return std::vector<std::string>();
    }

    ToolbarData* td = dynamic_cast<ToolbarData*>(d.get());
    IF_ASSERT_FAILED(td) {
        return std::vector<std::string>();
    }

    return td->actions;
}

async::Channel<AbstractDataPtr> Workspace::dataChanged() const
{
    return m_dataChanged;
}

void Workspace::addData(AbstractDataPtr data)
{
    DataKey key { data->tag, data->name };
    m_data[key] = data;

    m_dataChanged.send(data);
}

bool Workspace::isInited() const
{
    return m_isInited;
}

Ret Workspace::read()
{
    WorkspaceFile f(m_filePath);
    QByteArray data = f.readRootFile();
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

Ret Workspace::readWorkspace(const QByteArray& xmlData)
{
    QBuffer buffer;
    buffer.setData(xmlData);
    buffer.open(IODevice::ReadOnly);

    XmlReader reader(&buffer);
    while (reader.canRead()) {
        reader.readNextStartElement();

        if (reader.tagName() == SOURCE_TAG) {
            m_source = reader.readString();
            break;
        }
    }

    for (IWorkspaceDataStreamPtr stream : streamRegister()->streams()) {
        AbstractDataPtrList dataList = stream->read(buffer);

        for (AbstractDataPtr data : dataList) {
            DataKey key { data->tag, data->name };
            m_data[key] = data;
        }

        buffer.seek(0);
    }

    return make_ret(Ret::Code::Ok);
}

Ret Workspace::write()
{
    return Ret();
}
