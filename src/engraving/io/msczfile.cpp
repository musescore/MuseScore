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
#include "msczfile.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QBuffer>

#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"

#include "log.h"

using namespace mu::engraving;

MsczFile::MsczFile(const QString& filePath)
{
    m_file.setFileName(filePath);
}

MsczFile::~MsczFile()
{
    close();
}

bool MsczFile::open()
{
    if (!m_file.open(QIODevice::ReadWrite)) {
        LOGE() << "failed open file: " << filePath();
        return false;
    }

    if (!readInfo(m_info)) {
        LOGE() << "failed read meta, file: " << filePath();
        return false;
    }

    return true;
}

bool MsczFile::flush()
{
    return m_file.flush();
}

void MsczFile::close()
{
    flush();
    m_file.close();
}

void MsczFile::setFilePath(const QString& filePath)
{
    m_file.setFileName(filePath);
}

QString MsczFile::filePath() const
{
    return m_file.fileName();
}

const MsczFile::Info& MsczFile::info() const
{
    return m_info;
}

QByteArray MsczFile::mscx() const
{
    return fileData(info().mscxFileName);
}

void MsczFile::setMscx(const QString& fileName, const QByteArray& data)
{
    m_info.mscxFileName = fileName;
    addFileData(fileName, data);
}

QByteArray MsczFile::thumbnail() const
{
    return fileData("Thumbnails/thumbnail.png");
}

void MsczFile::setThumbnail(const QByteArray& data)
{
    addFileData("Thumbnails/thumbnail.png", data);
}

QByteArray MsczFile::fileData(const QString& fileName) const
{
    MQZipReader zip(&m_file);
    QByteArray fdata = zip.fileData(fileName);
    if (zip.status() != MQZipReader::NoError) {
        LOGE() << "failed read data, status: " << zip.status();
        zip.close();
        return QByteArray();
    }

    zip.close();
    return fdata;
}

bool MsczFile::addFileData(const QString& fileName, const QByteArray& data)
{
    MQZipWriter zip(&m_file);
    zip.addFile(fileName, data);
    if (zip.status() != MQZipWriter::NoError) {
        LOGE() << "failed write files to zip, status: " << zip.status();
        zip.close();
        return false;
    }

    zip.close();
    return true;
}

bool MsczFile::readInfo(Info& info) const
{
    MQZipReader zip(&m_file);

    QVector<MQZipReader::FileInfo> files = zip.fileInfoList();
    for (const MQZipReader::FileInfo& fi : files) {
        if (fi.isFile) {
            if (fi.filePath.endsWith(".mscx")) {
                info.mscxFileName = fi.filePath;
            }
        }
    }

    if (zip.status() != MQZipReader::NoError) {
        LOGE() << "failed read data, status: " << zip.status();
        zip.close();
        return false;
    }

    zip.close();
    return true;
}

void MsczFile::writeMeta(MQZipWriter& zip, const std::map<QString, QString>& meta)
{
    QByteArray data;
    QXmlStreamWriter xml(&data);
    xml.writeStartDocument();
    xml.writeStartElement("metadata");
    xml.writeStartElement("items");

    for (const auto& m : meta) {
        xml.writeStartElement("item");
        xml.writeAttribute("name", m.first);
        xml.writeCharacters(m.second);
        xml.writeEndElement();
    }

    xml.writeEndElement();
    xml.writeEndElement();
    xml.writeEndDocument();

    zip.addFile("META-INF/metadata.xml", data);
}

void MsczFile::readMeta(MQZipReader& zip, std::map<QString, QString>& meta)
{
    QByteArray data = zip.fileData("META-INF/metadata.xml");
    if (data.isEmpty()) {
        LOGE() << "not found META-INF/metadata.xml";
        return;
    }

    QXmlStreamReader xml(data);
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

                QString name = xml.attributes().value("name").toString();
                QString val = xml.readElementText();
                meta[name] = val;
            }
        }
    }
}

void MsczFile::writeContainer(MQZipWriter& zip, const std::vector<QString>& paths)
{
    QByteArray data;
    QBuffer buf(&data);
    buf.open(QIODevice::WriteOnly);
    QXmlStreamWriter xml(&buf);
    xml.writeStartDocument();
    xml.writeStartElement("container");
    xml.writeStartElement("rootfiles");

    for (const QString& f : paths) {
        xml.writeStartElement("rootfile");
        xml.writeAttribute("full-path", f);
        xml.writeEndElement();
    }

    xml.writeEndElement();
    xml.writeEndElement();
    xml.writeEndDocument();

    zip.addFile("META-INF/container.xml", data);
}
