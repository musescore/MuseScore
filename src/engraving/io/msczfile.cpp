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
#include <QFileInfo>

#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"

#include "log.h"

using namespace mu::engraving;

MsczFile::MsczFile(const QString& filePath)
    : m_filePath(filePath), m_device(new QFile(filePath)), m_selfDeviceOwner(true)
{
}

MsczFile::MsczFile(QIODevice* device)
    : m_device(device), m_selfDeviceOwner(false)
{
}

MsczFile::~MsczFile()
{
    close();

    if (m_selfDeviceOwner) {
        delete m_device;
    }
}

bool MsczFile::exists() const
{
    return QFileInfo::exists(filePath());
}

bool MsczFile::isWritable() const
{
    return QFileInfo(filePath()).isWritable();
}

bool MsczFile::open()
{
    if (!m_device->open(QIODevice::ReadWrite)) {
        LOGE() << "failed open file: " << filePath();
        return false;
    }

    if (!readMeta(m_meta)) {
        LOGE() << "failed read meta, file: " << filePath();
        return false;
    }

    return true;
}

bool MsczFile::flush()
{
    QFileDevice* fd = dynamic_cast<QFileDevice*>(f);
    if (fd) {
        return fd->flush();
    }
    return true;
}

void MsczFile::close()
{
    flush();
    m_device->close();
}

bool MsczFile::isOpened() const
{
    return m_device->isOpen();
}

void MsczFile::setFilePath(const QString& filePath)
{
    m_filePath = filePath;
    if (!m_device) {
        m_device = new QFile(filePath);
        m_selfDeviceOwner = true;
    }
}

QString MsczFile::filePath() const
{
    return m_filePath;
}

const MsczFile::Meta& MsczFile::meta() const
{
    return m_meta;
}

QByteArray MsczFile::readMscx() const
{
    return fileData(meta().mscxFileName);
}

void MsczFile::writeMscx(const QByteArray& data)
{
    addFileData(m_meta.mscxFileName, data);
}

std::vector<QByteArray> MsczFile::readImages() const
{
    NOT_IMPLEMENTED;
    return std::vector<QByteArray>();
}

void MsczFile::addImage(const QString& name, const QByteArray& data)
{
    addFileData("Pictures/" + name, data);
}

QByteArray MsczFile::thumbnail() const
{
    return fileData("Thumbnails/thumbnail.png");
}

void MsczFile::writeThumbnail(const QByteArray& data)
{
    addFileData("Thumbnails/thumbnail.png", data);
}

QByteArray MsczFile::fileData(const QString& fileName) const
{
    MQZipReader zip(m_device);
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
    MQZipWriter zip(m_device);
    zip.addFile(fileName, data);
    if (zip.status() != MQZipWriter::NoError) {
        LOGE() << "failed write files to zip, status: " << zip.status();
        zip.close();
        return false;
    }

    zip.close();
    return true;
}

bool MsczFile::readMeta(Meta& info) const
{
    MQZipReader zip(m_device);

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

void MsczFile::writeMetaData(MQZipWriter& zip, const std::map<QString, QString>& meta)
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

void MsczFile::readMetaData(MQZipReader& zip, std::map<QString, QString>& meta)
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
