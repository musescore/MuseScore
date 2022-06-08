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
#include "mscreader.h"

#include <QDir>
#include <QDirIterator>

#include "io/file.h"
#include "io/fileinfo.h"
#include "serialization/zipreader.h"
#include "serialization/xmlstreamreader.h"

#include "log.h"

//! NOTE The current implementation resolves files by extension.
//! This will probably be changed in the future.

using namespace mu;
using namespace mu::io;
using namespace mu::engraving;

MscReader::MscReader(const Params& params)
    : m_params(params)
{
}

MscReader::~MscReader()
{
    close();
}

void MscReader::setParams(const Params& params)
{
    IF_ASSERT_FAILED(!isOpened()) {
        return;
    }

    if (m_reader) {
        delete m_reader;
        m_reader = nullptr;
    }

    m_params = params;
}

const MscReader::Params& MscReader::params() const
{
    return m_params;
}

bool MscReader::open()
{
    return reader()->open(m_params.device, m_params.filePath);
}

void MscReader::close()
{
    if (m_reader) {
        m_reader->close();

        delete m_reader;
        m_reader = nullptr;
    }
}

bool MscReader::isOpened() const
{
    return m_reader ? m_reader->isOpened() : false;
}

MscReader::IReader* MscReader::reader() const
{
    if (!m_reader) {
        switch (m_params.mode) {
        case MscIoMode::Zip:
            m_reader = new ZipFileReader();
            break;
        case MscIoMode::Dir:
            m_reader = new DirReader();
            break;
        case MscIoMode::XmlFile:
            m_reader = new XmlFileReader();
            break;
        case MscIoMode::Unknown:
            UNREACHABLE;
            break;
        }
    }

    return m_reader;
}

ByteArray MscReader::fileData(const QString& fileName) const
{
    return reader()->fileData(fileName);
}

ByteArray MscReader::readStyleFile() const
{
    return fileData("score_style.mss");
}

QString MscReader::mainFileName() const
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

ByteArray MscReader::readScoreFile() const
{
    QString mscxFileName = mainFileName();
    ByteArray data = fileData(mscxFileName);
    if (data.empty() && reader()->isContainer()) {
        StringList files = reader()->fileList();
        for (const String& name : files) {
            // mscx file in the root dir
            if (!name.contains('/') && name.endsWith(".mscx", mu::CaseInsensitive)) {
                mscxFileName = name.toQString();
                break;
            }
        }
    }

    return fileData(mscxFileName);
}

std::vector<QString> MscReader::excerptNames() const
{
    if (!reader()->isContainer()) {
        NOT_SUPPORTED << " not container";
        return std::vector<QString>();
    }

    std::vector<QString> names;
    StringList files = reader()->fileList();
    for (const String& filePath : files) {
        if (filePath.startsWith("Excerpts/") && filePath.endsWith(".mscx", mu::CaseInsensitive)) {
            names.push_back(FileInfo(filePath.toQString()).completeBaseName());
        }
    }
    return names;
}

ByteArray MscReader::readExcerptStyleFile(const QString& name) const
{
    QString fileName = name + ".mss";
    return fileData("Excerpts/" + fileName);
}

ByteArray MscReader::readExcerptFile(const QString& name) const
{
    QString fileName = name + ".mscx";
    return fileData("Excerpts/" + fileName);
}

ByteArray MscReader::readChordListFile() const
{
    return fileData("chordlist.xml");
}

ByteArray MscReader::readThumbnailFile() const
{
    return fileData("Thumbnails/thumbnail.png");
}

ByteArray MscReader::readImageFile(const QString& fileName) const
{
    return fileData("Pictures/" + fileName);
}

std::vector<QString> MscReader::imageFileNames() const
{
    if (!reader()->isContainer()) {
        NOT_SUPPORTED << " not container";
        return std::vector<QString>();
    }

    std::vector<QString> names;
    StringList files = reader()->fileList();
    for (const String& filePath : files) {
        if (filePath.startsWith("Pictures/")) {
            names.push_back(FileInfo(filePath.toQString()).fileName());
        }
    }
    return names;
}

ByteArray MscReader::readAudioFile() const
{
    return fileData("audio.ogg");
}

ByteArray MscReader::readAudioSettingsJsonFile() const
{
    return fileData("audiosettings.json");
}

ByteArray MscReader::readViewSettingsJsonFile() const
{
    return fileData("viewsettings.json");
}

// =======================================================================
// Readers
// =======================================================================

MscReader::ZipFileReader::~ZipFileReader()
{
    delete m_zip;
    if (m_selfDeviceOwner) {
        delete m_device;
    }
}

bool MscReader::ZipFileReader::open(IODevice* device, const QString& filePath)
{
    m_device = device;
    if (!m_device) {
        m_device = new File(filePath);
        m_selfDeviceOwner = true;
    }

    if (!m_device->isOpen()) {
        if (!m_device->open(IODevice::ReadOnly)) {
            LOGD() << "failed open file: " << filePath;
            return false;
        }
    }

    m_zip = new ZipReader(m_device);

    return true;
}

void MscReader::ZipFileReader::close()
{
    if (m_zip) {
        m_zip->close();
    }

    if (m_device) {
        m_device->close();
    }
}

bool MscReader::ZipFileReader::isOpened() const
{
    return m_device ? m_device->isOpen() : false;
}

bool MscReader::ZipFileReader::isContainer() const
{
    return true;
}

StringList MscReader::ZipFileReader::fileList() const
{
    IF_ASSERT_FAILED(m_zip) {
        return StringList();
    }

    StringList files;
    std::vector<ZipReader::FileInfo> fileInfoList = m_zip->fileInfoList();
    if (m_zip->status() != ZipReader::NoError) {
        LOGD() << "failed read meta, status: " << m_zip->status();
    }

    for (const ZipReader::FileInfo& fi : fileInfoList) {
        if (fi.isFile) {
            files << String::fromQString(fi.filePath);
        }
    }

    return files;
}

ByteArray MscReader::ZipFileReader::fileData(const QString& fileName) const
{
    IF_ASSERT_FAILED(m_zip) {
        return ByteArray();
    }

    ByteArray data = m_zip->fileData(fileName);
    if (m_zip->status() != ZipReader::NoError) {
        LOGD() << "failed read data, status: " << m_zip->status();
        return ByteArray();
    }
    return data;
}

bool MscReader::DirReader::open(IODevice* device, const QString& filePath)
{
    if (device) {
        NOT_SUPPORTED;
        return false;
    }

    if (!FileInfo::exists(filePath)) {
        LOGD() << "not exists path: " << filePath;
        return false;
    }

    m_rootPath = containerPath(filePath).toQString();

    return true;
}

void MscReader::DirReader::close()
{
    // noop
}

bool MscReader::DirReader::isOpened() const
{
    return FileInfo::exists(m_rootPath);
}

bool MscReader::DirReader::isContainer() const
{
    //! NOTE We will assume that if there is `/META-INF/container.xml` in the root directory,
    //! then we read from the container (a directory with a certain structure)
    return FileInfo::exists(m_rootPath + "/META-INF/container.xml");
}

StringList MscReader::DirReader::fileList() const
{
    StringList files;
    QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;
    QDirIterator it(m_rootPath, QStringList(), QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable | QDir::Files, flags);

    while (it.hasNext()) {
        String filePath = String::fromQString(it.next());
        files << filePath.mid(m_rootPath.length() + 1);
    }

    return files;
}

ByteArray MscReader::DirReader::fileData(const QString& fileName) const
{
    QString filePath = m_rootPath + "/" + fileName;
    File file(filePath);
    if (!file.open(IODevice::ReadOnly)) {
        LOGD() << "failed open file: " << filePath;
        return ByteArray();
    }

    return file.readAll();
}

bool MscReader::XmlFileReader::open(IODevice* device, const QString& filePath)
{
    m_device = device;
    if (!m_device) {
        m_device = new File(filePath);
        m_selfDeviceOwner = true;
    }

    if (!m_device->isOpen()) {
        if (!m_device->open(IODevice::ReadOnly)) {
            LOGD() << "failed open file: " << filePath;
            return false;
        }
    }

    return true;
}

void MscReader::XmlFileReader::close()
{
    if (m_device) {
        m_device->close();
    }
}

bool MscReader::XmlFileReader::isOpened() const
{
    return m_device ? m_device->isOpen() : false;
}

bool MscReader::XmlFileReader::isContainer() const
{
    return true;
}

StringList MscReader::XmlFileReader::fileList() const
{
    if (!m_device) {
        return StringList();
    }

    StringList files;

    m_device->seek(0);
    XmlStreamReader xml(m_device);
    while (xml.readNextStartElement()) {
        if ("files" != xml.name()) {
            xml.skipCurrentElement();
            continue;
        }

        while (xml.readNextStartElement()) {
            if ("file" != xml.name()) {
                xml.skipCurrentElement();
                continue;
            }

            String fileName = xml.attribute("name");
            files << fileName;
            xml.skipCurrentElement();
        }
    }

    return files;
}

ByteArray MscReader::XmlFileReader::fileData(const QString& fileName) const
{
    if (!m_device) {
        return ByteArray();
    }

    m_device->seek(0);
    XmlStreamReader xml(m_device);
    while (xml.readNextStartElement()) {
        if ("files" != xml.name()) {
            xml.skipCurrentElement();
            continue;
        }

        while (xml.readNextStartElement()) {
            if ("file" != xml.name()) {
                xml.skipCurrentElement();
                continue;
            }

            String file = xml.attribute("name");
            if (file.toQString() != fileName) {
                xml.skipCurrentElement();
                continue;
            }

            String cdata = xml.readText();
            ByteArray ba = cdata.trimmed().toUtf8();
            return ba;
        }
    }

    return ByteArray();
}
