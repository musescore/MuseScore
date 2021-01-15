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
#include <QDomDocument>

using namespace mu;
using namespace mu::workspace;
using namespace mu::framework;

#define MSC_VERSION "3.01" // FIXME

static constexpr std::string_view WS_MUSESCORE_TAG("museScore");
static constexpr std::string_view WS_VERSION_ATTRIBUTE("version");
static constexpr std::string_view WS_WORKSPACE_TAG("workspace");
static constexpr std::string_view WS_SOURCE_TAG("source");

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

AbstractDataPtrList Workspace::dataList(WorkspaceTag tag) const
{
    AbstractDataPtrList result;

    for (auto it = m_data.cbegin(); it != m_data.cend(); ++it) {
        if (it->first.tag == tag) {
            result.push_back(it->second);
        }
    }

    return result;
}

mu::Val Workspace::settingValue(const std::string& key) const
{
    AbstractDataPtr data = this->data(WorkspaceTag::Settings);
    SettingsDataPtr settings = std::dynamic_pointer_cast<SettingsData>(data);
    return settings ? settings->values[key] : Val();
}

std::vector<std::string> Workspace::toolbarActions(const std::string& toolbarName) const
{
    AbstractDataPtr data = this->data(WorkspaceTag::Toolbar, toolbarName);
    ToolbarDataPtr toolbar = std::dynamic_pointer_cast<ToolbarData>(data);
    return toolbar ? toolbar->actions : std::vector<std::string>();
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
    m_hasUnsavedChanges = true;
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
    m_data.clear();
    m_hasUnsavedChanges = false;
    m_isInited = false;
}

Ret Workspace::readWorkspace(const QByteArray& xmlData)
{
    QBuffer buffer;
    buffer.setData(xmlData);
    buffer.open(IODevice::ReadOnly);

    XmlReader reader(&buffer);
    while (reader.canRead()) {
        reader.readNextStartElement();

        if (reader.tagName() == WS_SOURCE_TAG) {
            m_source = reader.readString();
            break;
        }
    }

    buffer.seek(0);

    for (const IWorkspaceDataStreamPtr& stream : streamRegister()->streams()) {
        for (AbstractDataPtr data : stream->read(buffer)) {
            DataKey key { data->tag, data->name };
            m_data[key] = data;
        }

        buffer.seek(0);
    }

    return make_ret(Ret::Code::Ok);
}

Ret Workspace::write()
{
    if (!m_hasUnsavedChanges) {
        return make_ret(Ret::Code::Ok);
    }

    QBuffer buffer;
    buffer.open(IODevice::WriteOnly);
    XmlWriter writer(&buffer);

    writer.writeStartDocument();
    writer.writeStartElement(WS_MUSESCORE_TAG);
    writer.writeAttribute(WS_VERSION_ATTRIBUTE, MSC_VERSION);
    writer.writeStartElement(WS_WORKSPACE_TAG);

    //! NOTE: at least one element should be written
    //! before any stream will start writing
    //! otherwise tags in output file will be closed incorrectly
    writer.writeTextElement(WS_SOURCE_TAG, m_source);

    for (const IWorkspaceDataStreamPtr& stream : streamRegister()->streams()) {
        AbstractDataPtrList dataList = this->dataList(stream->tag());
        stream->write(dataList, buffer);
    }

    writer.writeEndElement();
    writer.writeEndElement();
    writer.writeEndDocument();

    //! NOTE: correct formatting
    buffer.seek(0);
    QDomDocument document;
    document.setContent(buffer.data());
    constexpr int FORMATTING_INDENT = 4;
    QByteArray xmlData = document.toByteArray(FORMATTING_INDENT);

    WorkspaceFile file(m_filePath);
    Ret ret = file.writeRootFile(name() + ".xml", xmlData);
    buffer.close();
    m_hasUnsavedChanges = false;

    return ret;
}
