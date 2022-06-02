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

#include <vector>

#include <QDir>

#include "containers.h"
#include "io/buffer.h"
#include "io/file.h"
#include "io/fileinfo.h"
#include "serialization/xmlstreamwriter.h"
#include "serialization/zipwriter.h"
#include "serialization/textstream.h"

#include "log.h"

using namespace mu::io;
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
            m_writer = new ZipFileWriter();
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

bool MscWriter::addFileData(const QString& fileName, const ByteArray& data)
{
    if (!writer()->addFileData(fileName, data)) {
        LOGE() << "failed write file: " << fileName;
        return false;
    }

    m_meta.addFile(fileName);

    return true;
}

void MscWriter::writeStyleFile(const ByteArray& data)
{
    addFileData("score_style.mss", data);
}

QString MscWriter::mainFileName() const
{
    if (!m_params.mainFileName.isEmpty()) {
        return m_params.mainFileName;
    }

    QString name = "score.mscx";
    if (m_params.filePath.isEmpty()) {
        return name;
    }

    QString completeBaseName = FileInfo(m_params.filePath).completeBaseName();
    if (completeBaseName.isEmpty()) {
        return name;
    }

    return completeBaseName + ".mscx";
}

void MscWriter::writeScoreFile(const ByteArray& data)
{
    addFileData(mainFileName(), data);
}

void MscWriter::addExcerptStyleFile(const QString& name, const ByteArray& data)
{
    QString fileName = name + ".mss";
    addFileData("Excerpts/" + fileName, data);
}

void MscWriter::addExcerptFile(const QString& name, const ByteArray& data)
{
    QString fileName = name + ".mscx";
    addFileData("Excerpts/" + fileName, data);
}

void MscWriter::writeChordListFile(const ByteArray& data)
{
    addFileData("chordlist.xml", data);
}

void MscWriter::writeThumbnailFile(const ByteArray& data)
{
    addFileData("Thumbnails/thumbnail.png", data);
}

void MscWriter::addImageFile(const QString& fileName, const ByteArray& data)
{
    addFileData("Pictures/" + fileName, data);
}

void MscWriter::writeAudioFile(const ByteArray& data)
{
    addFileData("audio.ogg", data);
}

void MscWriter::writeAudioSettingsJsonFile(const ByteArray& data)
{
    addFileData("audiosettings.json", data);
}

void MscWriter::writeViewSettingsJsonFile(const ByteArray& data)
{
    addFileData("viewsettings.json", data);
}

void MscWriter::writeMeta()
{
    if (m_meta.isWritten) {
        return;
    }

    writeContainer(m_meta.files);

    m_meta.isWritten = true;
}

void MscWriter::writeContainer(const std::vector<QString>& paths)
{
    ByteArray data;
    Buffer buf(&data);
    buf.open(IODevice::WriteOnly);
    XmlStreamWriter xml(&buf);
    xml.startDocument();
    xml.writeStartElement("container");
    xml.writeStartElement("rootfiles");

    for (const QString& f : paths) {
        xml.writeElement(QString("rootfile full-path=\"%1\"").arg(f));
    }

    xml.writeEndElement();
    xml.writeEndElement();
    xml.flush();

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

MscWriter::ZipFileWriter::~ZipFileWriter()
{
    delete m_zip;
    if (m_selfDeviceOwner) {
        delete m_device;
    }
}

bool MscWriter::ZipFileWriter::open(io::IODevice* device, const QString& filePath)
{
    m_device = device;
    if (!m_device) {
        m_device = new File(filePath);
        m_selfDeviceOwner = true;
    }

    if (!m_device->isOpen()) {
        if (!m_device->open(IODevice::WriteOnly)) {
            LOGE() << "failed open file: " << filePath;
            return false;
        }
    }

    m_zip = new ZipWriter(m_device);

    return true;
}

void MscWriter::ZipFileWriter::close()
{
    if (m_zip) {
        m_zip->close();
    }

    if (m_device) {
        m_device->close();
    }
}

bool MscWriter::ZipFileWriter::isOpened() const
{
    return m_device ? m_device->isOpen() : false;
}

bool MscWriter::ZipFileWriter::addFileData(const QString& fileName, const ByteArray& data)
{
    IF_ASSERT_FAILED(m_zip) {
        return false;
    }

    m_zip->addFile(fileName, data);
    if (m_zip->status() != ZipWriter::NoError) {
        LOGE() << "failed write files to zip, status: " << m_zip->status();
        return false;
    }
    return true;
}

bool MscWriter::DirWriter::open(io::IODevice* device, const QString& filePath)
{
    if (device) {
        NOT_SUPPORTED;
        return false;
    }

    if (filePath.isEmpty()) {
        LOGE() << "file path is empty";
        return false;
    }

    m_rootPath = containerPath(filePath).toQString();

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
    return FileInfo::exists(m_rootPath);
}

bool MscWriter::DirWriter::addFileData(const QString& fileName, const ByteArray& data)
{
    QString filePath = m_rootPath + "/" + fileName;

    QDir fileDir(FileInfo(filePath).absolutePath());
    if (!fileDir.exists()) {
        if (!fileDir.mkpath(fileDir.absolutePath())) {
            LOGE() << "failed make path: " << fileDir.absolutePath();
            return false;
        }
    }

    File file(filePath);
    if (!file.open(IODevice::WriteOnly)) {
        LOGE() << "failed open file: " << filePath;
        return false;
    }

    if (file.write(data) != data.size()) {
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

bool MscWriter::XmlFileWriter::open(io::IODevice* device, const QString& filePath)
{
    m_device = device;
    if (!m_device) {
        m_device = new File(filePath);
        m_selfDeviceOwner = true;
    }

    if (!m_device->isOpen()) {
        if (!m_device->open(IODevice::WriteOnly)) {
            LOGE() << "failed open file: " << filePath;
            return false;
        }
    }

    m_stream = new TextStream(m_device);

    // Write header
    *m_stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    *m_stream << "<files>\n";

    return true;
}

void MscWriter::XmlFileWriter::close()
{
    if (m_stream) {
        *m_stream << "</files>\n";
        m_stream->flush();
        m_device->close();
    }
}

bool MscWriter::XmlFileWriter::isOpened() const
{
    return m_device ? m_device->isOpen() : false;
}

bool MscWriter::XmlFileWriter::addFileData(const QString& fileName, const ByteArray& data)
{
    if (!m_stream) {
        return false;
    }

    static const std::vector<QString> supportedExts = { "mscx", "json", "mss" };
    QString ext = FileInfo::suffix(fileName);
    if (!mu::contains(supportedExts, ext)) {
        NOT_SUPPORTED << fileName;
        return true; // not error
    }

    TextStream& ts = *m_stream;
    ts << "<file name=\"" << fileName << "\">\n";
    ts << "<![CDATA[";
    ts << data;
    ts << "]]>\n";
    ts << "</file>\n";

    return true;
}
