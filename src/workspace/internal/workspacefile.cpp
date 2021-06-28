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

#include <QBuffer>

#include "stringutils.h"

#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"

#include "global/xmlreader.h"
#include "global/xmlwriter.h"

#include "workspaceerrors.h"

#include "log.h"

using namespace mu::workspace;
using namespace mu::system;
using namespace mu::framework;

WorkspaceFile::WorkspaceFile(const io::path& filePath)
    : m_filePath(filePath)
{}

mu::io::path WorkspaceFile::filePath() const
{
    return m_filePath;
}

mu::Ret WorkspaceFile::load()
{
    RetVal<QByteArray> data = fileSystem()->readFile(m_filePath);
    if (!data.ret) {
        LOGE() << "failed read file, err: " << data.ret.toString();
        return data.ret;
    }

    QBuffer buf(&data.val);
    buf.open(QIODevice::ReadOnly);
    MQZipReader zip(&buf);

    Meta::read(zip, m_meta);
    if (m_meta.empty() || zip.status() != MQZipReader::NoError) {
        LOGE() << "failed read meta data";
        return make_ret(Err::FailedUnPack);
    }

    QVector<MQZipReader::FileInfo> files = zip.fileInfoList();
    for (const MQZipReader::FileInfo& fi : files) {
        if (fi.isFile) {
            QByteArray fdata = zip.fileData(fi.filePath);
            m_data[fi.filePath.toStdString()] = fdata;
        }
    }

    if (zip.status() != MQZipReader::NoError) {
        LOGE() << "failed read data, status: " << zip.status();
        return make_ret(Err::FailedUnPack);
    }

    zip.close();
    return make_ret(Ret::Code::Ok);
}

mu::Ret WorkspaceFile::save()
{
    std::vector<std::string> paths;
    for (const auto& d : m_data) {
        paths.push_back(d.first);
    }

    QByteArray data;
    QBuffer buf(&data);
    buf.open(QIODevice::WriteOnly);
    MQZipWriter zip(&buf);

    Container::write(zip, paths);
    if (zip.status() != MQZipWriter::NoError) {
        LOGE() << "failed write files to zip, status: " << zip.status();
        return make_ret(Err::FailedPack);
    }

    Meta::write(zip, m_meta);
    if (zip.status() != MQZipWriter::NoError) {
        LOGE() << "failed write files to zip, status: " << zip.status();
        return make_ret(Err::FailedPack);
    }

    for (const auto& d : m_data) {
        zip.addFile(QString::fromStdString(d.first), d.second);
    }

    if (zip.status() != MQZipWriter::NoError) {
        LOGE() << "failed write files to zip, status: " << zip.status();
        return make_ret(Err::FailedPack);
    }

    zip.close();

    Ret ret = fileSystem()->writeToFile(m_filePath, data);
    if (!ret) {
        LOGE() << "failed write to file, err: " << ret.toString();
    }

    return ret;
}

void WorkspaceFile::Container::write(MQZipWriter& zip, const std::vector<std::string>& paths)
{
    QByteArray data;
    QBuffer buf(&data);
    buf.open(QIODevice::WriteOnly);
    XmlWriter xml(&buf);
    xml.writeStartDocument();
    xml.writeStartElement("container");
    xml.writeStartElement("rootfiles");

    for (const std::string& f : paths) {
        xml.writeStartElement("rootfile");
        xml.writeAttribute("full-path", f);
        xml.writeEndElement();
    }

    xml.writeEndElement();
    xml.writeEndElement();
    xml.writeEndDocument();

    zip.addFile("META-INF/container.xml", data);
}

void WorkspaceFile::Meta::write(MQZipWriter& zip, const std::map<std::string, Val>& meta)
{
    QByteArray data;
    QBuffer buf(&data);
    buf.open(QIODevice::WriteOnly);
    XmlWriter xml(&buf);
    xml.writeStartDocument();
    xml.writeStartElement("metadata");
    xml.writeStartElement("items");

    for (const auto& m : meta) {
        xml.writeStartElement("item");
        xml.writeAttribute("name", m.first);
        xml.writeCharacters(m.second.toString());
        xml.writeEndElement();
    }

    xml.writeEndElement();
    xml.writeEndElement();
    xml.writeEndDocument();

    zip.addFile("META-INF/metadata.xml", data);
}

void WorkspaceFile::Meta::read(MQZipReader& zip, std::map<std::string, Val>& meta)
{
    QByteArray data = zip.fileData("META-INF/metadata.xml");
    if (data.isEmpty()) {
        LOGE() << "not found META-INF/metadata.xml";
        return;
    }

    XmlReader xml(data);
    while (xml.readNextStartElement()) {
        if ("metadata" != xml.tagName()) {
            xml.skipCurrentElement();
            continue;
        }

        while (xml.readNextStartElement()) {
            if ("items" != xml.tagName()) {
                xml.skipCurrentElement();
                continue;
            }

            while (xml.readNextStartElement()) {
                if ("item" != xml.tagName()) {
                    xml.skipCurrentElement();
                    continue;
                }

                std::string name = xml.attribute("name");
                std::string val = xml.readString();
                meta[name] = Val(val);
            }
        }
    }
}

bool WorkspaceFile::isLoaded() const
{
    return !m_meta.empty();
}

mu::Val WorkspaceFile::meta(const std::string& key) const
{
    auto it = m_meta.find(key);
    if (it != m_meta.end()) {
        return it->second;
    }
    return Val();
}

void WorkspaceFile::setMeta(const std::string& key, const Val& val)
{
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
    m_data[name] = data;
}
