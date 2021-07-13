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
#include "msczwriter.h"

#include <QXmlStreamWriter>
#include <QFile>
#include <QFileInfo>
#include <QBuffer>

#include "thirdparty/qzip/qzipwriter_p.h"

#include "log.h"

using namespace mu::engraving;

MsczWriter::MsczWriter(const QString& filePath)
    : m_filePath(filePath), m_device(new QFile(filePath)), m_selfDeviceOwner(true)
{
}

MsczWriter::MsczWriter(QIODevice* device)
    : m_device(device), m_selfDeviceOwner(false)
{
}

MsczWriter::~MsczWriter()
{
    close();

    delete m_writer;

    if (m_selfDeviceOwner) {
        delete m_device;
    }
}

bool MsczWriter::open()
{
    if (!m_device->isOpen()) {
        if (!m_device->open(QIODevice::WriteOnly)) {
            LOGE() << "failed open file: " << filePath();
            return false;
        }
    }

    return true;
}

void MsczWriter::close()
{
    writeMeta();

    if (m_writer) {
        m_writer->close();
    }
    m_device->close();
}

bool MsczWriter::isOpened() const
{
    return m_device->isOpen();
}

void MsczWriter::setDevice(QIODevice* device)
{
    if (m_writer) {
        delete m_writer;
        m_writer = nullptr;
    }

    if (m_device && m_selfDeviceOwner) {
        delete m_device;
    }

    m_device = device;
    m_selfDeviceOwner = false;
}

void MsczWriter::setFilePath(const QString& filePath)
{
    m_filePath = filePath;

    if (m_writer) {
        delete m_writer;
        m_writer = nullptr;
    }

    if (!m_device) {
        m_device = new QFile(filePath);
        m_selfDeviceOwner = true;
    }
}

QString MsczWriter::filePath() const
{
    return m_filePath;
}

MQZipWriter* MsczWriter::writer() const
{
    if (!m_writer) {
        m_writer = new MQZipWriter(m_device);
    }
    return m_writer;
}

bool MsczWriter::addFileData(const QString& fileName, const QByteArray& data)
{
    writer()->addFile(fileName, data);
    if (writer()->status() != MQZipWriter::NoError) {
        LOGE() << "failed write files to zip, status: " << writer()->status();
        return false;
    }

    m_meta.addFile(fileName);

    return true;
}

void MsczWriter::writeScoreFile(const QByteArray& data)
{
    QString completeBaseName = QFileInfo(filePath()).completeBaseName();
    IF_ASSERT_FAILED(!completeBaseName.isEmpty()) {
        completeBaseName = "score";
    }
    QString fileName = completeBaseName + ".mscx";
    addFileData(fileName, data);
}

void MsczWriter::writeThumbnailFile(const QByteArray& data)
{
    addFileData("Thumbnails/thumbnail.png", data);
}

void MsczWriter::addImageFile(const QString& fileName, const QByteArray& data)
{
    addFileData("Pictures/" + fileName, data);
}

void MsczWriter::writeAudioFile(const QByteArray& data)
{
    addFileData("audio.ogg", data);
}

void MsczWriter::writeAudioSettingsJsonFile(const QByteArray& data)
{
    addFileData("audiosettings.json", data);
}

void MsczWriter::writeMeta()
{
    if (m_meta.isWrited) {
        return;
    }

    writeContainer(m_meta.files);

    m_meta.isWrited = true;
}

void MsczWriter::writeContainer(const std::vector<QString>& paths)
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

    addFileData("META-INF/container.xml", data);
}
