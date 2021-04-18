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

#include "libmscore/mscore.h"

#include <QBuffer>

using namespace mu;
using namespace mu::workspace;
using namespace mu::system;

static constexpr std::string_view WORKSPACE_MUSESCORE_TAG("museScore");
static constexpr std::string_view WORKSPACE_VERSION_ATTRIBUTE("version");
static constexpr std::string_view WORKSPACE_WORKSPACE_TAG("workspace");
static constexpr std::string_view WORKSPACE_SOURCE_TAG("source");

static constexpr std::string_view WORKSPACE_TAGS_TAG("tags");
static const std::string WORKSPACE_TAGS_SEPARATOR(",");

static std::map<WorkspaceTag, std::string> tagToName {
    { WorkspaceTag::Palettes, "Palettes" },
    { WorkspaceTag::Settings, "Settings" },
    { WorkspaceTag::Toolbar, "Toolbar" },
    { WorkspaceTag::UiArrangement, "UiArrangement" }
};

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

WorkspaceTagList Workspace::tags() const
{
    return m_tags;
}

void Workspace::setTags(const WorkspaceTagList& tags)
{
    m_tags = tags;
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

std::string Workspace::tagsNames() const
{
    std::string result;
    for (WorkspaceTag tag: m_tags) {
        result += tagToName[tag] + WORKSPACE_TAGS_SEPARATOR;
    }

    for (size_t i = 0; i < WORKSPACE_TAGS_SEPARATOR.length(); ++i) {
        result.pop_back();
    }

    return result;
}

std::vector<WorkspaceTag> Workspace::parseTags(const std::string& tagsStr) const
{
    std::vector<WorkspaceTag> result;

    auto tagByName = [](const std::string& tagName) {
        for (auto it = tagToName.begin(); it != tagToName.end(); ++it) {
            if (it->second == tagName) {
                return it->first;
            }
        }

        return WorkspaceTag::Unknown;
    };

    std::string tags = tagsStr;

    size_t pos = 0;
    while ((pos = tags.find(WORKSPACE_TAGS_SEPARATOR)) != std::string::npos) {
        std::string tagName = tags.substr(0, pos);

        result.push_back(tagByName(tagName));

        tags.erase(0, pos + WORKSPACE_TAGS_SEPARATOR.length());
    }

    WorkspaceTag tag = tagByName(tags);
    if (tag != WorkspaceTag::Unknown) {
        result.push_back(tag);
    }

    return result;
}

Ret Workspace::readWorkspace(const QByteArray& xmlData)
{
    QBuffer buffer;
    buffer.setData(xmlData);
    buffer.open(IODevice::ReadOnly);

    XmlReader reader(&buffer);
    while (reader.canRead()) {
        reader.readNextStartElement();

        if (reader.tagName() == WORKSPACE_SOURCE_TAG) {
            m_source = reader.readString();
        } else if (reader.tagName() == WORKSPACE_TAGS_TAG) {
            m_tags = parseTags(reader.readString());
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
    writer.writeStartElement(WORKSPACE_MUSESCORE_TAG);
    writer.writeAttribute(WORKSPACE_VERSION_ATTRIBUTE, MSC_VERSION);
    writer.writeStartElement(WORKSPACE_WORKSPACE_TAG);

    //! NOTE: at least one element should be written
    //! before any stream will start writing
    //! otherwise tags in output file will be closed incorrectly
    writer.writeTextElement(WORKSPACE_SOURCE_TAG, m_source);

    writer.writeTextElement(WORKSPACE_TAGS_TAG, tagsNames());

    for (const IWorkspaceDataStreamPtr& stream : streamRegister()->streams()) {
        AbstractDataPtrList dataList = this->dataList(stream->tag());
        stream->write(dataList, buffer);
    }

    writer.writeEndElement();
    writer.writeEndElement();
    writer.writeEndDocument();

    WorkspaceFile file(m_filePath);
    Ret ret = file.writeRootFile(name() + ".xml", buffer.data());
    m_hasUnsavedChanges = false;

    return ret;
}
