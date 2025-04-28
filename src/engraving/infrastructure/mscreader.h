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
#pragma once

#include "types/ret.h"
#include "types/string.h"
#include "io/path.h"
#include "io/iodevice.h"
#include "mscio.h"

namespace muse {
class ZipReader;
}

namespace mu::engraving {
class MscReader
{
public:

    struct Params
    {
        muse::io::IODevice* device = nullptr;
        muse::io::path_t filePath;
        muse::String mainFileName;
        MscIoMode mode = MscIoMode::Zip;
    };

    MscReader() = default;
    MscReader(const Params& params);
    ~MscReader();

    void setParams(const Params& params);
    const Params& params() const;

    muse::Ret open();
    void close();
    bool isOpened() const;

    muse::ByteArray readStyleFile() const;
    muse::ByteArray readScoreFile() const;

    std::vector<muse::String> excerptFileNames() const;
    muse::ByteArray readExcerptStyleFile(const muse::String& excerptFileName) const;
    muse::ByteArray readExcerptFile(const muse::String& excerptFileName) const;

    muse::ByteArray readChordListFile() const;
    muse::ByteArray readThumbnailFile() const;

    std::vector<muse::String> imageFileNames() const;
    muse::ByteArray readImageFile(const muse::String& fileName) const;

    muse::ByteArray readAudioFile() const;
    muse::ByteArray readAudioSettingsJsonFile(const muse::io::path_t& pathPrefix = "") const;
    muse::ByteArray readViewSettingsJsonFile(const muse::io::path_t& pathPrefix = "") const;

private:

    struct IReader {
        virtual ~IReader() = default;

        virtual muse::Ret open(muse::io::IODevice* device, const muse::io::path_t& filePath) = 0;
        virtual void close() = 0;
        virtual bool isOpened() const = 0;
        //! NOTE In the case of reading from a directory,
        //! it may happen that we are not reading a container (a directory with a certain structure),
        //! but only one file among others (`.mscx` from MU 3.x)
        virtual bool isContainer() const = 0;
        virtual muse::StringList fileList() const = 0;
        virtual bool fileExists(const muse::String& fileName) const = 0;
        virtual muse::ByteArray fileData(const muse::String& fileName) const = 0;
    };

    struct ZipFileReader : public IReader
    {
        ~ZipFileReader() override;
        muse::Ret open(muse::io::IODevice* device, const muse::io::path_t& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool isContainer() const override;
        muse::StringList fileList() const override;
        bool fileExists(const muse::String& fileName) const override;
        muse::ByteArray fileData(const muse::String& fileName) const override;
    private:
        muse::io::IODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
        muse::ZipReader* m_zip = nullptr;
    };

    struct DirReader : public IReader
    {
        muse::Ret open(muse::io::IODevice* device, const muse::io::path_t& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool isContainer() const override;
        muse::StringList fileList() const override;
        bool fileExists(const muse::String& fileName) const override;
        muse::ByteArray fileData(const muse::String& fileName) const override;
    private:
        muse::io::path_t m_rootPath;
    };

    struct XmlFileReader : public IReader
    {
        muse::Ret open(muse::io::IODevice* device, const muse::io::path_t& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool isContainer() const override;
        muse::StringList fileList() const override;
        bool fileExists(const muse::String& fileName) const override;
        muse::ByteArray fileData(const muse::String& fileName) const override;
    private:
        muse::io::IODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
    };

    IReader* reader() const;
    bool fileExists(const muse::String& fileName) const;
    muse::ByteArray fileData(const muse::String& fileName) const;

    muse::String mainFileName() const;

    Params m_params;
    mutable IReader* m_reader = nullptr;
};
}
