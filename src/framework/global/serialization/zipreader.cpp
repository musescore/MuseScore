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
#include "zipreader.h"

#include "global/io/file.h"
#include "global/io/dir.h"
#include "internal/zipcontainer.h"

using namespace muse;
using namespace muse::io;

struct ZipReader::Impl
{
    ZipContainer* zip = nullptr;
    IODevice* device = nullptr;
    bool isSelfDevice = false;
};

ZipReader::ZipReader(const io::path_t& filePath)
    : m_filePath(filePath)
{
    m_impl = new Impl();
    m_impl->device = new File(filePath);
    m_impl->isSelfDevice = true;
    if (m_impl->device->open(IODevice::ReadOnly)) {
    }
    m_impl->zip = new ZipContainer(m_impl->device);
}

ZipReader::ZipReader(IODevice* device)
{
    m_impl = new Impl();
    m_impl->device = device;
    m_impl->zip = new ZipContainer(m_impl->device);
}

ZipReader::~ZipReader()
{
    close();
    delete m_impl->zip;
    if (m_impl->isSelfDevice) {
        delete m_impl->device;
    }
    delete m_impl;
}

bool ZipReader::exists() const
{
    return File::exists(m_filePath);
}

void ZipReader::close()
{
    m_impl->zip->close();
}

bool ZipReader::hasError() const
{
    return m_impl->zip->status() != ZipContainer::NoError;
}

std::vector<ZipReader::FileInfo> ZipReader::fileInfoList() const
{
    std::vector<FileInfo> ret;
    std::vector<ZipContainer::FileInfo> fis = m_impl->zip->fileInfoList();
    ret.reserve(fis.size());
    for (const ZipContainer::FileInfo& qfi : fis) {
        FileInfo fi;
        fi.filePath = qfi.filePath;
        fi.isDir = qfi.isDir;
        fi.isFile = qfi.isFile;
        fi.isSymLink = qfi.isSymLink;
        fi.size = qfi.size;

        ret.push_back(std::move(fi));
    }

    return ret;
}

bool ZipReader::fileExists(const std::string& fileName) const
{
    return m_impl->zip->fileExists(fileName);
}

ByteArray ZipReader::fileData(const std::string& fileName) const
{
    return m_impl->zip->fileData(fileName);
}

// ===========================
// ZipUnpack
// ===========================

Ret ZipUnpack::unpack(const io::path_t& zipPath, const io::path_t& dirPath)
{
    Ret ret = io::Dir::mkpath(dirPath);
    if (!ret) {
        return ret;
    }

    ZipReader zip(zipPath);
    for (const ZipReader::FileInfo& fi : zip.fileInfoList()) {
        if (fi.isDir) {
            ret = io::Dir::mkpath(dirPath + "/" + fi.filePath);
        } else if (fi.isFile) {
            ByteArray data = zip.fileData(fi.filePath.toStdString());
            ret = io::File::writeFile(dirPath + "/" + fi.filePath, data);
        }

        if (!ret) {
            break;
        }
    }

    return ret;
}
