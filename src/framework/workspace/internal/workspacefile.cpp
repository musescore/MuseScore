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
#include "workspacefile.h"

#include "stringutils.h"

#include "io/buffer.h"

#include "global/serialization/zipreader.h"
#include "global/serialization/zipwriter.h"

#include "global/serialization/xmlstreamreader.h"
#include "global/serialization/xmlstreamwriter.h"

#include "workspaceerrors.h"

#include "log.h"

using namespace muse;
using namespace muse::workspace;

WorkspaceFile::WorkspaceFile(const io::path_t& filePath)
    : m_filePath(filePath)
{}

WorkspaceFile::WorkspaceFile(const io::path_t& filePath, const WorkspaceFile* other)
    : WorkspaceFile(filePath)
{
    m_meta = other->m_meta;
    m_data = other->m_data;
}

io::path_t WorkspaceFile::filePath() const
{
    return m_filePath;
}

Ret WorkspaceFile::load()
{
    m_data.clear();
    m_meta.clear();

    ZipReader zip(m_filePath);

    Meta::read(zip, m_meta);
    if (m_meta.empty() || zip.hasError()) {
        LOGE() << "failed read meta data";
        return make_ret(Err::FailedUnPack);
    }

    std::vector<ZipReader::FileInfo> files = zip.fileInfoList();
    for (const ZipReader::FileInfo& fi : files) {
        if (fi.isFile) {
            ByteArray fdata = zip.fileData(fi.filePath.toStdString());
            m_data[fi.filePath.toStdString()] = fdata.toQByteArray();
        }
    }

    if (zip.hasError()) {
        LOGE() << "failed read data";
        return make_ret(Err::FailedUnPack);
    }

    zip.close();

    m_needSave = false;

    return make_ret(Ret::Code::Ok);
}

Ret WorkspaceFile::save()
{
    std::vector<std::string> paths;
    for (const auto& d : m_data) {
        paths.push_back(d.first);
    }

    ZipWriter zip(m_filePath);

    Container::write(zip, paths);
    if (zip.hasError()) {
        LOGE() << "failed write files to zip";
        return make_ret(Err::FailedPack);
    }

    Meta::write(zip, m_meta);
    if (zip.hasError()) {
        LOGE() << "failed write files to zip";
        return make_ret(Err::FailedPack);
    }

    for (const auto& d : m_data) {
        if (d.second.isEmpty()) {
            continue;
        }

        zip.addFile(d.first, ByteArray::fromQByteArray(d.second));
    }

    if (zip.hasError()) {
        LOGE() << "failed write files to zip";
        return make_ret(Err::FailedPack);
    }

    zip.close();

    m_needSave = false;

    return make_ok();
}

void WorkspaceFile::Container::write(ZipWriter& zip, const std::vector<std::string>& paths)
{
    ByteArray data;
    io::Buffer buf(&data);
    buf.open(io::IODevice::WriteOnly);
    XmlStreamWriter xml(&buf);
    xml.startDocument();

    xml.startElement("container");
    xml.startElement("rootfiles");

    for (const std::string& f : paths) {
        xml.element("rootfile", { { "full-path", f } });
    }

    xml.endElement();
    xml.endElement();
    xml.flush();

    zip.addFile("META-INF/container.xml", data);
}

void WorkspaceFile::Meta::write(ZipWriter& zip, const std::map<std::string, Val>& meta)
{
    ByteArray data;
    io::Buffer buf(&data);
    buf.open(io::IODevice::WriteOnly);
    XmlStreamWriter xml(&buf);
    xml.startDocument();

    xml.startElement("metadata");
    xml.startElement("items");

    for (const auto& m : meta) {
        xml.element("item", { { "name", m.first } }, m.second.toString());
    }

    xml.endElement();
    xml.endElement();
    xml.flush();

    zip.addFile("META-INF/metadata.xml", data);
}

void WorkspaceFile::Meta::read(ZipReader& zip, std::map<std::string, Val>& meta)
{
    ByteArray data = zip.fileData("META-INF/metadata.xml");
    if (data.empty()) {
        LOGE() << "not found META-INF/metadata.xml";
        return;
    }

    XmlStreamReader xml(data);
    while (xml.readNextStartElement()) {
        if ("metadata" != xml.name()) {
            xml.skipCurrentElement();
            continue;
        }

        while (xml.readNextStartElement()) {
            if ("items" != xml.name()) {
                xml.skipCurrentElement();
                continue;
            }

            while (xml.readNextStartElement()) {
                if ("item" != xml.name()) {
                    xml.skipCurrentElement();
                    continue;
                }

                std::string name = xml.attribute("name").toStdString();
                std::string val = xml.readText().toStdString();
                meta[name] = Val(val);
            }
        }
    }
}

bool WorkspaceFile::isLoaded() const
{
    return !m_meta.empty();
}

bool WorkspaceFile::isNeedSave() const
{
    return m_needSave;
}

Val WorkspaceFile::meta(const std::string& key) const
{
    auto it = m_meta.find(key);
    if (it != m_meta.end()) {
        return it->second;
    }
    return Val();
}

void WorkspaceFile::setMeta(const std::string& key, const Val& val)
{
    if (muse::contains(m_meta, key) && m_meta[key] == val) {
        return;
    }

    markDirty();

    m_meta[key] = val;
}

QByteArray WorkspaceFile::data(const std::string& name) const
{
    auto it = m_data.find(name);
    if (it != m_data.end()) {
        return it->second;
    }
    return QByteArray();
}

void WorkspaceFile::setData(const std::string& name, const QByteArray& data)
{
    if (muse::contains(m_data, name) && m_data[name] == data) {
        return;
    }

    markDirty();

    m_data[name] = data;
}

void WorkspaceFile::markDirty()
{
    m_needSave = true;
}
