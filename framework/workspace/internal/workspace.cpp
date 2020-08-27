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

#include <QXmlStreamReader>

#include "log.h"
#include "translation.h"
#include "workspacefile.h"

#include "libmscore/xml.h"

using namespace mu;
using namespace mu::workspace;

Workspace::Workspace(const io::path& file)
    : m_file(file)
{
}

std::string Workspace::name() const
{
    return io::basename(m_file).toStdString();
}

std::string Workspace::title() const
{
    return name();
}

std::shared_ptr<AbstractData> Workspace::data(const std::string& tag, const std::string& name) const
{
    DataKey key{ tag, name };
    auto it = m_data.find(key);
    if (it != m_data.end()) {
        return it->second;
    }
    return nullptr;
}

Val Workspace::settingValue(const std::string& key) const
{
    std::shared_ptr<AbstractData> d = data("Preferences", "");
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
    std::shared_ptr<AbstractData> d = data("Toolbar", toolbarName);
    if (!d) {
        return std::vector<std::string>();
    }

    ToolbarData* td = dynamic_cast<ToolbarData*>(d.get());
    IF_ASSERT_FAILED(td) {
        return std::vector<std::string>();
    }

    return td->actions;
}

void Workspace::addData(std::shared_ptr<AbstractData> data)
{
    DataKey key{ data->tag, data->name };
    m_data[key] = data;
}

bool Workspace::isInited() const
{
    return m_isInited;
}

Ret Workspace::read()
{
    WorkspaceFile f(m_file);
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

Ret Workspace::readWorkspace(const QByteArray& data)
{
    Ms::XmlReader xml(data);

    while (xml.readNextStartElement()) {
        if (xml.name() != "museScore") {
            return make_ret(Ret::Code::UnknownError);
        }

        while (xml.readNextStartElement()) {
            if (xml.name() != "Workspace") {
                return make_ret(Ret::Code::UnknownError);
            }

            while (xml.readNextStartElement()) {
                QString tag(xml.name().toString());
                if ("name" == tag) {
                    xml.readElementText();
                } else if ("source" == tag) {
                    m_source = xml.readElementText().toStdString();
                } else {
                    std::shared_ptr<IWorkspaceDataStream> reader = streamRegister()->stream(tag.toStdString());
                    if (!reader) {
                        LOGW() << "not registred reader for: " << tag;
                        xml.skipCurrentElement();
                        continue;
                    }

                    std::shared_ptr<AbstractData> data = reader->read(xml);
                    if (!data) {
                        LOGE() << "failed read: " << tag;
                    } else {
                        DataKey key{ data->tag, data->name };
                        m_data[key] = data;
                    }
                }
            }
        }
    }

    return make_ret(Ret::Code::Ok);
}

Ret Workspace::write()
{
    return Ret();
}
