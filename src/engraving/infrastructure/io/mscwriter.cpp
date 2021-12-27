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
#include "mscwriter.h"

#include <QXmlStreamWriter>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QBuffer>
#include <QTextStream>

#include "thirdparty/qzip/qzipwriter_p.h"

#include "log.h"

using namespace mu::engraving;

MscWriter::MscWriter(const Params& params)
    : m_params(params)
{
}

MscWriter::~MscWriter()
{
    close();
}

void MscWriter::setParams(const Params& params)
{
    IF_ASSERT_FAILED(!isOpened()) {
        return;
    }

    if (m_writer) {
        delete m_writer;
        m_writer = nullptr;
    }

    m_params = params;
}

const MscWriter::Params& MscWriter::params() const
{
    return m_params;
}

bool MscWriter::open()
{
    return writer()->open(m_params.device, m_params.filePath);
}

void MscWriter::close()
{
    if (m_writer) {
        writeMeta();

        m_writer->close();

        delete m_writer;
        m_writer = nullptr;
    }
}

bool MscWriter::isOpened() const
{
    return m_writer ? m_writer->isOpened() : false;
}

MscWriter::IWriter* MscWriter::writer() const
{
    if (!m_writer) {
        switch (m_params.mode) {
        case MscIoMode::Zip:
            m_writer = new ZipWriter();
            break;
        case MscIoMode::Dir:
            m_writer = new DirWriter();
            break;
        case MscIoMode::XmlFile:
            m_writer = new XmlFileWriter();
            break;
        case MscIoMode::Unknown:
            UNREACHABLE;
            break;
        }
    }

    return m_writer;
}

bool MscWriter::addFileData(const QString& fileName, const QByteArray& data)
{
    if (!writer()->addFileData(fileName, data)) {
        LOGE() << "failed write file: " << fileName;
        return false;
    }

    m_meta.addFile(fileName);

    return true;
}

void MscWriter::writeStyleFile(const QByteArray& data)
{
    addFileData("score_style.mss", data);
}

void MscWriter::writeScoreFile(const QByteArray& data)
{
    QString completeBaseName = QFileInfo(m_params.filePath).completeBaseName();
    IF_ASSERT_FAILED(!completeBaseName.isEmpty()) {
        completeBaseName = "score";
    }
    QString fileName = completeBaseName + ".mscx";
    addFileData(fileName, data);
}

void MscWriter::addExcerptStyleFile(const QString& name, const QByteArray& data)
{
    QString fileName = name + ".mss";
    addFileData("Excerpts/" + fileName, data);
}

void MscWriter::addExcerptFile(const QString& name, const QByteArray& data)
{
    QString fileName = name + ".mscx";
    addFileData("Excerpts/" + fileName, data);
}

void MscWriter::writeChordListFile(const QByteArray& data)
{
    addFileData("chordlist.xml", data);
}

void MscWriter::writeThumbnailFile(const QByteArray& data)
{
    addFileData("Thumbnails/thumbnail.png", data);
}

void MscWriter::addImageFile(const QString& fileName, const QByteArray& data)
{
    addFileData("Pictures/" + fileName, data);
}

void MscWriter::writeAudioFile(const QByteArray& data)
{
    addFileData("audio.ogg", data);
}

void MscWriter::writeAudioSettingsJsonFile(const QByteArray& data)
{
    addFileData("audiosettings.json", data);
}

void MscWriter::writeViewSettingsJsonFile(const QByteArray& data)
{
    addFileData("viewsettings.json", data);
}

void MscWriter::writeMeta()
{
    if (m_meta.isWrited) {
        return;
    }

    writeContainer(m_meta.files);

    m_meta.isWrited = true;
}

void MscWriter::writeContainer(const std::vector<QString>& paths)
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

bool MscWriter::Meta::contains(const QString& file) const
{
    if (std::find(files.begin(), files.end(), file) != files.end()) {
        return true;
    }
    return false;
}

void MscWriter::Meta::addFile(const QString& file)
{
    if (!contains(file)) {
        files.push_back(file);
    }
}

// =======================================================================
// Writers
// =======================================================================

MscWriter::ZipWriter::~ZipWriter()
{
    delete m_zip;
    if (m_selfDeviceOwner) {
        delete m_device;
    }
}

bool MscWriter::ZipWriter::open(QIODevice* device, const QString& filePath)
{
    m_device = device;
    if (!m_device) {
        m_device = new QFile(filePath);
        m_selfDeviceOwner = true;
    }

    if (!m_device->isOpen()) {
        if (!m_device->open(QIODevice::WriteOnly)) {
            LOGE() << "failed open file: " << filePath;
            return false;
        }
    }

    m_zip = new MQZipWriter(m_device);

    return true;
}

void MscWriter::ZipWriter::close()
{
    if (m_zip) {
        m_zip->close();
    }

    if (m_device) {
        m_device->close();
    }
}

bool MscWriter::ZipWriter::isOpened() const
{
    return m_device ? m_device->isOpen() : false;
}

bool MscWriter::ZipWriter::addFileData(const QString& fileName, const QByteArray& data)
{
    IF_ASSERT_FAILED(m_zip) {
        return false;
    }

    m_zip->addFile(fileName, data);
    if (m_zip->status() != MQZipWriter::NoError) {
        LOGE() << "failed write files to zip, status: " << m_zip->status();
        return false;
    }
    return true;
}

bool MscWriter::DirWriter::open(QIODevice* device, const QString& filePath)
{
    if (device) {
        NOT_SUPPORTED;
        return false;
    }

    QFileInfo fi(filePath);

    if (fi.absolutePath().isEmpty()) {
        LOGE() << "file path is empty";
        return false;
    }

    m_rootPath = fi.absolutePath() + "/" + fi.completeBaseName();

    QDir dir(m_rootPath);
    if (!dir.removeRecursively()) {
        LOGE() << "failed clear dir: " << dir.absolutePath();
        return false;
    }

    if (!dir.mkpath(dir.absolutePath())) {
        LOGE() << "failed make path: " << dir.absolutePath();
        return false;
    }

    return true;
}

void MscWriter::DirWriter::close()
{
    // noop
}

bool MscWriter::DirWriter::isOpened() const
{
    return QFileInfo::exists(m_rootPath);
}

bool MscWriter::DirWriter::addFileData(const QString& fileName, const QByteArray& data)
{
    QString filePath = m_rootPath + "/" + fileName;

    QDir fileDir(QFileInfo(filePath).absolutePath());
    if (!fileDir.exists()) {
        if (!fileDir.mkpath(fileDir.absolutePath())) {
            LOGE() << "failed make path: " << fileDir.absolutePath();
            return false;
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        LOGE() << "failed open file: " << filePath;
        return false;
    }

    if (file.write(data) != qint64(data.size())) {
        LOGE() << "failed write file: " << filePath;
        return false;
    }

    return true;
}

MscWriter::XmlFileWriter::~XmlFileWriter()
{
    delete m_stream;
    if (m_selfDeviceOwner) {
        delete m_device;
    }
}

bool MscWriter::XmlFileWriter::open(QIODevice* device, const QString& filePath)
{
    m_device = device;
    if (!m_device) {
        m_device = new QFile(filePath);
        m_selfDeviceOwner = true;
    }

    if (!m_device->isOpen()) {
        if (!m_device->open(QIODevice::WriteOnly)) {
            LOGE() << "failed open file: " << filePath;
            return false;
        }
    }

    m_stream = new QTextStream(m_device);

    // Write header
    *m_stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" << Qt::endl;
    *m_stream << "<files>" << Qt::endl;

    return true;
}

void MscWriter::XmlFileWriter::close()
{
    if (m_stream) {
        *m_stream << "</files>" << Qt::endl;
        m_stream->flush();
    }

    if (m_device) {
        m_device->close();
    }
}

bool MscWriter::XmlFileWriter::isOpened() const
{
    return m_device ? m_device->isOpen() : false;
}

bool MscWriter::XmlFileWriter::addFileData(const QString& fileName, const QByteArray& data)
{
    if (!m_stream) {
        return false;
    }

    static QList<QString> supportedExts = { "mscx", "json", "mss" };
    QString ext = QFileInfo(fileName).suffix();
    if (!supportedExts.contains(ext)) {
        NOT_SUPPORTED << fileName;
        return true; // not error
    }

    QTextStream& ts = *m_stream;
    ts << "<file name=\"" << fileName << "\">" << Qt::endl;
    ts << "<![CDATA[";
    ts << data;
    ts << "]]>" << Qt::endl;
    ts << "</file>" << Qt::endl;

    return true;
}
