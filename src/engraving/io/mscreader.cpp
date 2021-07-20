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

MscReader::MscReader(const QString& filePath, Mode mode)
    : m_filePath(filePath), m_mode(mode)
{
}

MscReader::MscReader(QIODevice* device)
    : m_mode(Mode::Zip), m_device(device), m_selfDeviceOwner(false)
{
}

MscReader::~MscReader()
{
    close();

    delete m_reader;

    if (m_selfDeviceOwner) {
        delete m_device;
    }
}

QString MscReader::rootPath() const
{
    switch (m_mode) {
    case Mode::Zip: {
        return "/";
    } break;
    case Mode::Dir: {
        QFileInfo fi(m_filePath);
        return fi.absolutePath();
    } break;
    }
    return QString();
}

bool MscReader::open()
{
    switch (m_mode) {
    case Mode::Zip: {
        if (!m_device) {
            m_device = new QFile(m_filePath);
            m_selfDeviceOwner = true;
        }

        if (!m_device->isOpen()) {
            if (!m_device->open(QIODevice::ReadOnly)) {
                LOGE() << "failed open file: " << filePath();
                return false;
            }
        }
    } break;
    case Mode::Dir: {
        QString root = rootPath();
        if (!QFileInfo::exists(root)) {
            LOGE() << "not exists path: " << root;
            return false;
        }
    } break;
    }

    return true;
}

void MscReader::close()
{
    switch (m_mode) {
    case Mode::Zip: {
        if (m_reader) {
            m_reader->close();
        }
        m_device->close();
    } break;
    case Mode::Dir: {
        // noop
    } break;
    }
}

bool MscReader::isOpened() const
{
    switch (m_mode) {
    case Mode::Zip: {
        return m_device->isOpen();
    } break;
    case Mode::Dir: {
        return QFileInfo::exists(rootPath());
    } break;
    }
    return false;
}

void MscReader::setDevice(QIODevice* device)
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

    if (m_mode == Mode::Dir) {
        LOGW() << "The mode changed to ZIP";
        m_mode = Mode::Zip;
    }
}

void MscReader::setFilePath(const QString& filePath)
{
    m_filePath = filePath;

    if (m_reader) {
        delete m_reader;
        m_reader = nullptr;
    }
}

QString MscReader::filePath() const
{
    return m_filePath;
}

void MscReader::setMode(Mode m)
{
    m_mode = m;
}

MscReader::Mode MscReader::mode() const
{
    return m_mode;
}

MQZipReader* MscReader::reader() const
{
    if (!m_reader) {
        m_reader = new MQZipReader(m_device);
    }
    return m_reader;
}

const MscReader::Meta& MscReader::meta() const
{
    if (m_meta.isValid()) {
        return m_meta;
    }

    auto fileList = [this]() {
        QStringList files;
        switch (m_mode) {
        case Mode::Zip: {
            QVector<MQZipReader::FileInfo> fileInfoList = reader()->fileInfoList();

            if (reader()->status() != MQZipReader::NoError) {
                LOGE() << "failed read meta, status: " << reader()->status();
            }

            for (const MQZipReader::FileInfo& fi : fileInfoList) {
                if (fi.isFile) {
                    files << fi.filePath;
                }
            }
        } break;
        case Mode::Dir: {
            QString root = rootPath();
            QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;
            QDirIterator it(root, QStringList(), QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable | QDir::Files, flags);

            while (it.hasNext()) {
                QString filePath = it.next();
                files << filePath.mid(root.length());
            }
        } break;
        }

        return files;
    };

    QStringList files = fileList();
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
    switch (m_mode) {
    case Mode::Zip: {
        QByteArray data = reader()->fileData(fileName);
        if (reader()->status() != MQZipReader::NoError) {
            LOGE() << "failed read data, status: " << reader()->status();
            return QByteArray();
        }
        return data;
    } break;
    case Mode::Dir: {
        QString filePath = rootPath() + "/" + fileName;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            LOGE() << "failed open file: " << filePath;
            return QByteArray();
        }

        QByteArray data = file.readAll();
        return data;
    } break;
    }
    return QByteArray();
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
