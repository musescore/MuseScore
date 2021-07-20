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
    return m_reader->isOpened();
}

MscReader::IReader* MscReader::reader() const
{
    if (!m_reader) {
        switch (m_params.mode) {
        case Mode::Zip:
            m_reader = new ZipReader();
            break;
        case Mode::Dir:
            m_reader = new DirReader();
            break;
        case Mode::XmlFile:
            m_reader = new XmlFileReader();
            break;
        }
    }

    return m_reader;
}

const MscReader::Meta& MscReader::meta() const
{
    if (m_meta.isValid()) {
        return m_meta;
    }

    QStringList files = reader()->fileList();
    for (const QString& filePath : files) {
        if (filePath.endsWith(".mscx")) {
            m_meta.mscxFileName = filePath;
        } else if (filePath.startsWith("Pictures/")) {
            m_meta.imageFilePaths.push_back(filePath);
        }
    }

    return m_meta;
}

QByteArray MscReader::fileData(const QString& fileName) const
{
    return reader()->fileData(fileName);
}

QByteArray MscReader::readScoreFile() const
{
    return fileData(meta().mscxFileName);
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
    std::vector<QString> names;
    for (const QString& path : meta().imageFilePaths) {
        names.push_back(QFileInfo(path).fileName());
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
            LOGE() << "failed open file: " << filePath;
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

QStringList MscReader::ZipReader::fileList() const
{
    IF_ASSERT_FAILED(m_zip) {
        return QStringList();
    }

    QStringList files;
    QVector<MQZipReader::FileInfo> fileInfoList = m_zip->fileInfoList();
    if (m_zip->status() != MQZipReader::NoError) {
        LOGE() << "failed read meta, status: " << m_zip->status();
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
        LOGE() << "failed read data, status: " << m_zip->status();
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
        LOGE() << "not exists path: " << m_rootPath;
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

QStringList MscReader::DirReader::fileList() const
{
    QStringList files;
    QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;
    QDirIterator it(m_rootPath, QStringList(), QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable | QDir::Files, flags);

    while (it.hasNext()) {
        QString filePath = it.next();
        files << filePath.mid(m_rootPath.length());
    }

    return files;
}

QByteArray MscReader::DirReader::fileData(const QString& fileName) const
{
    QString filePath = m_rootPath + "/" + fileName;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOGE() << "failed open file: " << filePath;
        return QByteArray();
    }

    QByteArray data = file.readAll();
    return data;
}

bool MscReader::XmlFileReader::open(QIODevice* device, const QString& filePath)
{
}

void MscReader::XmlFileReader::close()
{
}

bool MscReader::XmlFileReader::isOpened() const
{
}

QStringList MscReader::XmlFileReader::fileList() const
{
}

QByteArray MscReader::XmlFileReader::fileData(const QString& fileName) const
{
}
