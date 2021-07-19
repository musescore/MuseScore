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
#include "msczreader.h"

#include <QXmlStreamReader>
#include <QFile>
#include <QFileInfo>

#include "thirdparty/qzip/qzipreader_p.h"

#include "log.h"

//! NOTE The current implementation resolves files by extension.
//! This will probably be changed in the future.

using namespace mu::engraving;

MsczReader::MsczReader(const QString& filePath)
    : m_filePath(filePath), m_device(new QFile(filePath)), m_selfDeviceOwner(true)
{
}

MsczReader::MsczReader(QIODevice* device)
    : m_device(device), m_selfDeviceOwner(false)
{
}

MsczReader::~MsczReader()
{
    close();

    delete m_reader;

    if (m_selfDeviceOwner) {
        delete m_device;
    }
}

bool MsczReader::open()
{
    if (!m_device->isOpen()) {
        if (!m_device->open(QIODevice::ReadOnly)) {
            LOGE() << "failed open file: " << filePath();
            return false;
        }
    }

    return true;
}

void MsczReader::close()
{
    if (m_reader) {
        m_reader->close();
    }
    m_device->close();
}

bool MsczReader::isOpened() const
{
    return m_device->isOpen();
}

void MsczReader::setDevice(QIODevice* device)
{
    if (m_reader) {
        delete m_reader;
        m_reader = nullptr;
    }

    if (m_device && m_selfDeviceOwner) {
        delete m_device;
    }

    m_device = device;
    m_selfDeviceOwner = false;
}

void MsczReader::setFilePath(const QString& filePath)
{
    m_filePath = filePath;

    if (m_reader) {
        delete m_reader;
        m_reader = nullptr;
    }

    if (!m_device) {
        m_device = new QFile(filePath);
        m_selfDeviceOwner = true;
    }
}

QString MsczReader::filePath() const
{
    return m_filePath;
}

MQZipReader* MsczReader::reader() const
{
    if (!m_reader) {
        m_reader = new MQZipReader(m_device);
    }
    return m_reader;
}

const MsczReader::Meta& MsczReader::meta() const
{
    if (m_meta.isValid()) {
        return m_meta;
    }

    QVector<MQZipReader::FileInfo> files = reader()->fileInfoList();
    for (const MQZipReader::FileInfo& fi : files) {
        if (fi.isFile) {
            if (fi.filePath.endsWith(".mscx")) {
                m_meta.mscxFileName = fi.filePath;
            } else if (fi.filePath.startsWith("Pictures/")) {
                m_meta.imageFilePaths.push_back(fi.filePath);
            }
        }
    }

    if (reader()->status() != MQZipReader::NoError) {
        LOGE() << "failed read meta, status: " << reader()->status();
    }

    return m_meta;
}

QByteArray MsczReader::fileData(const QString& fileName) const
{
    QByteArray data = reader()->fileData(fileName);
    if (reader()->status() != MQZipReader::NoError) {
        LOGE() << "failed read data, status: " << reader()->status();
        return QByteArray();
    }
    return data;
}

QByteArray MsczReader::readScoreFile() const
{
    return fileData(meta().mscxFileName);
}

QByteArray MsczReader::readThumbnailFile() const
{
    return fileData("Thumbnails/thumbnail.png");
}

QByteArray MsczReader::readImageFile(const QString& fileName) const
{
    return fileData("Pictures/" + fileName);
}

std::vector<QString> MsczReader::imageFileNames() const
{
    std::vector<QString> names;
    for (const QString& path : meta().imageFilePaths) {
        names.push_back(QFileInfo(path).fileName());
    }
    return names;
}

QByteArray MsczReader::readAudioFile() const
{
    return fileData("audio.ogg");
}

QByteArray MsczReader::readAudioSettingsJsonFile() const
{
    return fileData("audiosettings.json");
}
