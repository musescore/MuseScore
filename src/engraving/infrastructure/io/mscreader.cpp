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

#include <QXmlStreamReader>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>

#include "thirdparty/qzip/qzipreader_p.h"

#include "log.h"

//! NOTE The current implementation resolves files by extension.
//! This will probably be changed in the future.

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
            m_reader = new ZipReader();
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

QByteArray MscReader::fileData(const QString& fileName) const
{
    return reader()->fileData(fileName);
}

QByteArray MscReader::readStyleFile() const
{
    return fileData("score_style.mss");
}

QByteArray MscReader::readScoreFile() const
{
    QString mscxFileName = QFileInfo(m_params.filePath).completeBaseName() + ".mscx";
    QByteArray data = fileData(mscxFileName);
    if (data.isEmpty() && reader()->isContainer()) {
        QStringList files = reader()->fileList();
        for (const QString& name : files) {
            // mscx file in the root dir
            if (!name.contains("/") && name.endsWith(".mscx", Qt::CaseInsensitive)) {
                mscxFileName = name;
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
    QStringList files = reader()->fileList();
    for (const QString& filePath : files) {
        if (filePath.startsWith("Excerpts/") && filePath.endsWith(".mscx", Qt::CaseInsensitive)) {
            names.push_back(QFileInfo(filePath).completeBaseName());
        }
    }
    return names;
}

QByteArray MscReader::readExcerptStyleFile(const QString& name) const
{
    QString fileName = name + ".mss";
    return fileData("Excerpts/" + fileName);
}

QByteArray MscReader::readExcerptFile(const QString& name) const
{
    QString fileName = name + ".mscx";
    return fileData("Excerpts/" + fileName);
}

QByteArray MscReader::readChordListFile() const
{
    return fileData("chordlist.xml");
}

QByteArray MscReader::readThumbnailFile() const
{
    return fileData("Thumbnails/thumbnail.png");
}

QByteArray MscReader::readImageFile(const QString& fileName) const
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
    QStringList files = reader()->fileList();
    for (const QString& filePath : files) {
        if (filePath.startsWith("Pictures/")) {
            names.push_back(QFileInfo(filePath).fileName());
        }
    }
    return names;
}

QByteArray MscReader::readAudioFile() const
{
    return fileData("audio.ogg");
}

QByteArray MscReader::readAudioSettingsJsonFile() const
{
    return fileData("audiosettings.json");
}

QByteArray MscReader::readViewSettingsJsonFile() const
{
    return fileData("viewsettings.json");
}

// =======================================================================
// Readers
// =======================================================================

MscReader::ZipReader::~ZipReader()
{
    delete m_zip;
    if (m_selfDeviceOwner) {
        delete m_device;
    }
}

bool MscReader::ZipReader::open(QIODevice* device, const QString& filePath)
{
    m_device = device;
    if (!m_device) {
        m_device = new QFile(filePath);
        m_selfDeviceOwner = true;
    }

    if (!m_device->isOpen()) {
        if (!m_device->open(QIODevice::ReadOnly)) {
            LOGD() << QString("failed open %1: %2").arg(filePath).arg(m_device->errorString());
            return false;
        }
    }

    m_zip = new MQZipReader(m_device);

    return true;
}

void MscReader::ZipReader::close()
{
    if (m_zip) {
        m_zip->close();
    }

    if (m_device) {
        m_device->close();
    }
}

bool MscReader::ZipReader::isOpened() const
{
    return m_device ? m_device->isOpen() : false;
}

bool MscReader::ZipReader::isContainer() const
{
    return true;
}

QStringList MscReader::ZipReader::fileList() const
{
    IF_ASSERT_FAILED(m_zip) {
        return QStringList();
    }

    QStringList files;
    QVector<MQZipReader::FileInfo> fileInfoList = m_zip->fileInfoList();
    if (m_zip->status() != MQZipReader::NoError) {
        LOGD() << "failed read meta, status: " << m_zip->status();
    }

    for (const MQZipReader::FileInfo& fi : fileInfoList) {
        if (fi.isFile) {
            files << fi.filePath;
        }
    }

    return files;
}

QByteArray MscReader::ZipReader::fileData(const QString& fileName) const
{
    IF_ASSERT_FAILED(m_zip) {
        return QByteArray();
    }

    QByteArray data = m_zip->fileData(fileName);
    if (m_zip->status() != MQZipReader::NoError) {
        LOGD() << "failed read data, status: " << m_zip->status();
        return QByteArray();
    }
    return data;
}

bool MscReader::DirReader::open(QIODevice* device, const QString& filePath)
{
    if (device) {
        NOT_SUPPORTED;
        return false;
    }

    m_rootPath = QFileInfo(filePath).absolutePath();

    if (!QFileInfo::exists(m_rootPath)) {
        LOGD() << "not exists path: " << m_rootPath;
        return false;
    }

    return true;
}

void MscReader::DirReader::close()
{
    // noop
}

bool MscReader::DirReader::isOpened() const
{
    return QFileInfo::exists(m_rootPath);
}

bool MscReader::DirReader::isContainer() const
{
    //! NOTE We will assume that if there is `/META-INF/container.xml` in the root directory,
    //! then we read from the container (a directory with a certain structure)
    return QFileInfo::exists(m_rootPath + "/META-INF/container.xml");
}

QStringList MscReader::DirReader::fileList() const
{
    QStringList files;
    QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;
    QDirIterator it(m_rootPath, QStringList(), QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable | QDir::Files, flags);

    while (it.hasNext()) {
        QString filePath = it.next();
        files << filePath.mid(m_rootPath.length() + 1);
    }

    return files;
}

QByteArray MscReader::DirReader::fileData(const QString& fileName) const
{
    QString filePath = m_rootPath + "/" + fileName;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOGD() << "failed open file: " << filePath;
        return QByteArray();
    }

    QByteArray data = file.readAll();
    return data;
}

bool MscReader::XmlFileReader::open(QIODevice* device, const QString& filePath)
{
    m_device = device;
    if (!m_device) {
        m_device = new QFile(filePath);
        m_selfDeviceOwner = true;
    }

    if (!m_device->isOpen()) {
        if (!m_device->open(QIODevice::ReadOnly)) {
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

QStringList MscReader::XmlFileReader::fileList() const
{
    if (!m_device) {
        return QStringList();
    }

    QStringList files;

    m_device->seek(0);
    QXmlStreamReader xml(m_device);
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

            QStringRef fileName = xml.attributes().value("name");
            files << fileName.toString();
            xml.skipCurrentElement();
        }
    }

    return files;
}

QByteArray MscReader::XmlFileReader::fileData(const QString& fileName) const
{
    if (!m_device) {
        return QByteArray();
    }

    m_device->seek(0);
    QXmlStreamReader xml(m_device);
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

            QStringRef file = xml.attributes().value("name");
            if (file != fileName) {
                continue;
            }

            QString cdata = xml.readElementText();
            QByteArray data = cdata.trimmed().toUtf8();
            return data;
        }
    }

    return QByteArray();
}
