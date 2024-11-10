/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "containers.h"
#include "io/buffer.h"
#include "io/file.h"
#include "io/fileinfo.h"
#include "io/dir.h"
#include "serialization/xmlstreamwriter.h"
#include "serialization/zipwriter.h"
#include "serialization/textstream.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace muse::io;
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
        m_hadError = m_writer->hasError();
        delete m_writer;
        m_writer = nullptr;
    }

    m_params = params;
}

const MscWriter::Params& MscWriter::params() const
{
    return m_params;
}

Ret MscWriter::open()
{
    return writer()->open(m_params.device, m_params.filePath);
}

void MscWriter::close()
{
    if (m_writer) {
        if (m_writer->isOpened()) {
            writeMeta();
            m_writer->close();
        }

        m_hadError = m_writer->hasError();
        delete m_writer;
        m_writer = nullptr;
    }
}

bool MscWriter::isOpened() const
{
    return m_writer ? m_writer->isOpened() : false;
}

bool MscWriter::hasError() const
{
    return m_writer ? m_writer->hasError() : m_hadError;
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

bool MscWriter::addFileData(const String& fileName, const ByteArray& data)
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
    addFileData(u"score_style.mss", data);
}

String MscWriter::mainFileName() const
{
    if (!m_params.mainFileName.isEmpty()) {
        return m_params.mainFileName;
    }

    String name = u"score.mscx";
    if (m_params.filePath.empty()) {
        return name;
    }

    String completeBaseName = FileInfo(m_params.filePath).completeBaseName();
    if (completeBaseName.isEmpty()) {
        return name;
    }

    return completeBaseName + u".mscx";
}

void MscWriter::writeScoreFile(const ByteArray& data)
{
    addFileData(mainFileName(), data);
}

void MscWriter::addExcerptStyleFile(const String& excerptFileName, const ByteArray& data)
{
    String fileName = excerptFileName + u".mss";
    addFileData(u"Excerpts/" + excerptFileName + u"/" + fileName, data);
}

void MscWriter::addExcerptFile(const String& excerptFileName, const ByteArray& data)
{
    String fileName = excerptFileName + u".mscx";
    addFileData(u"Excerpts/" + excerptFileName + u"/" + fileName, data);
}

void MscWriter::writeChordListFile(const ByteArray& data)
{
    addFileData(u"chordlist.xml", data);
}

void MscWriter::writeThumbnailFile(const ByteArray& data)
{
    addFileData(u"Thumbnails/thumbnail.png", data);
}

void MscWriter::addImageFile(const String& fileName, const ByteArray& data)
{
    addFileData(u"Pictures/" + fileName, data);
}

void MscWriter::writeAudioFile(const ByteArray& data)
{
    addFileData(u"audio.ogg", data);
}

void MscWriter::writeAudioSettingsJsonFile(const ByteArray& data, const muse::io::path_t& pathPrefix)
{
    addFileData(pathPrefix.toString() + u"audiosettings.json", data);
}

void MscWriter::writeViewSettingsJsonFile(const ByteArray& data, const muse::io::path_t& pathPrefix)
{
    addFileData(pathPrefix.toString() + u"viewsettings.json", data);
}

void MscWriter::writeMeta()
{
    if (m_meta.isWritten) {
        return;
    }

    writeContainer(m_meta.files);

    m_meta.isWritten = true;
}

void MscWriter::writeContainer(const std::vector<String>& paths)
{
    ByteArray data;
    Buffer buf(&data);
    buf.open(IODevice::WriteOnly);
    XmlStreamWriter xml(&buf);
    xml.startDocument();
    xml.startElement("container");
    xml.startElement("rootfiles");

    for (const String& f : paths) {
        xml.element("rootfile", { { "full-path", f } });
    }

    xml.endElement();
    xml.endElement();
    xml.flush();

    addFileData(u"META-INF/container.xml", data);
}

bool MscWriter::Meta::contains(const String& file) const
{
    if (std::find(files.begin(), files.end(), file) != files.end()) {
        return true;
    }
    return false;
}

void MscWriter::Meta::addFile(const String& file)
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

Ret MscWriter::ZipFileWriter::open(io::IODevice* device, const path_t& filePath)
{
    m_device = device;
    if (!m_device) {
        m_device = new File(filePath);
        m_selfDeviceOwner = true;
    }

    if (!m_device->isOpen()) {
        if (!m_device->open(IODevice::WriteOnly)) {
            LOGE() << "failed open file: " << filePath;
            return make_ret(m_device->error(), m_device->errorString());
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

bool MscWriter::ZipFileWriter::hasError() const
{
    return (m_device ? m_device->hasError() : false) || (m_zip ? m_zip->hasError() : false);
}

bool MscWriter::ZipFileWriter::addFileData(const String& fileName, const ByteArray& data)
{
    IF_ASSERT_FAILED(m_zip) {
        return false;
    }

    m_zip->addFile(fileName.toStdString(), data);
    if (m_zip->hasError()) {
        LOGE() << "failed write files to zip";
        return false;
    }

    return true;
}

Ret MscWriter::DirWriter::open(io::IODevice* device, const muse::io::path_t& filePath)
{
    if (device) {
        NOT_SUPPORTED;
        m_hasError = true;
        return false;
    }

    if (filePath.empty()) {
        LOGE() << "file path is empty";
        m_hasError = true;
        return false;
    }

    m_rootPath = containerPath(filePath);

    Dir dir(m_rootPath);
    Ret ret = dir.removeRecursively();
    if (!ret) {
        LOGE() << "failed clear dir: " << dir.absolutePath();
        m_hasError = true;
        return ret;
    }

    ret = dir.mkpath(dir.absolutePath());
    if (!ret) {
        LOGE() << "failed make path: " << dir.absolutePath();
        m_hasError = true;
        return ret;
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

bool MscWriter::DirWriter::hasError() const
{
    return m_hasError;
}

bool MscWriter::DirWriter::addFileData(const String& fileName, const ByteArray& data)
{
    muse::io::path_t filePath = m_rootPath + "/" + fileName;

    Dir fileDir(FileInfo(filePath).absolutePath());
    if (!fileDir.exists()) {
        if (!fileDir.mkpath(fileDir.absolutePath())) {
            LOGE() << "failed make path: " << fileDir.absolutePath();
            m_hasError = true;
            return false;
        }
    }

    File file(filePath);
    if (!file.open(IODevice::WriteOnly)) {
        LOGE() << "failed open file: " << filePath;
        m_hasError = true;
        return false;
    }

    if (file.write(data) != data.size()) {
        LOGE() << "failed write file: " << filePath;
        m_hasError = true;
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

Ret MscWriter::XmlFileWriter::open(io::IODevice* device, const path_t& filePath)
{
    m_device = device;
    if (!m_device) {
        m_device = new File(filePath);
        m_selfDeviceOwner = true;
    }

    if (!m_device->isOpen()) {
        if (!m_device->open(IODevice::WriteOnly)) {
            LOGE() << "failed open file: " << filePath;
            return make_ret(m_device->error(), m_device->errorString());
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

bool MscWriter::XmlFileWriter::hasError() const
{
    return m_device ? m_device->hasError() : false;
}

bool MscWriter::XmlFileWriter::addFileData(const String& fileName, const ByteArray& data)
{
    if (!m_stream) {
        return false;
    }

    static const std::vector<String> supportedExts = { u"mscx", u"json", u"mss" };
    String ext = FileInfo::suffix(fileName);
    if (!muse::contains(supportedExts, ext)) {
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
