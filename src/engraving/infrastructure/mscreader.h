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
#ifndef MU_ENGRAVING_MSCREADER_H
#define MU_ENGRAVING_MSCREADER_H

#include "types/ret.h"
#include "types/string.h"
#include "io/path.h"
#include "io/iodevice.h"
#include "mscio.h"

namespace mu {
class ZipReader;
}

namespace mu::engraving {
class MscReader
{
public:

    struct Params
    {
        io::IODevice* device = nullptr;
        io::path_t filePath;
        String mainFileName;
        MscIoMode mode = MscIoMode::Zip;
    };

    MscReader() = default;
    MscReader(const Params& params);
    ~MscReader();

    void setParams(const Params& params);
    const Params& params() const;

    Ret open();
    void close();
    bool isOpened() const;

    ByteArray readStyleFile() const;
    ByteArray readScoreFile() const;

    std::vector<String> excerptNames() const;
    ByteArray readExcerptStyleFile(const String& name) const;
    ByteArray readExcerptFile(const String& name) const;

    ByteArray readChordListFile() const;
    ByteArray readThumbnailFile() const;

    std::vector<String> imageFileNames() const;
    ByteArray readImageFile(const String& fileName) const;

    ByteArray readAudioFile() const;
    ByteArray readAudioSettingsJsonFile() const;
    ByteArray readViewSettingsJsonFile(const io::path_t& pathPrefix) const;

private:

    struct IReader {
        virtual ~IReader() = default;

        virtual Ret open(io::IODevice* device, const io::path_t& filePath) = 0;
        virtual void close() = 0;
        virtual bool isOpened() const = 0;
        //! NOTE In the case of reading from a directory,
        //! it may happen that we are not reading a container (a directory with a certain structure),
        //! but only one file among others (`.mscx` from MU 3.x)
        virtual bool isContainer() const = 0;
        virtual StringList fileList() const = 0;
        virtual bool fileExists(const String& fileName) const = 0;
        virtual ByteArray fileData(const String& fileName) const = 0;
    };

    struct ZipFileReader : public IReader
    {
        ~ZipFileReader() override;
        Ret open(io::IODevice* device, const io::path_t& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool isContainer() const override;
        StringList fileList() const override;
        bool fileExists(const String& fileName) const override;
        ByteArray fileData(const String& fileName) const override;
    private:
        io::IODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
        ZipReader* m_zip = nullptr;
    };

    struct DirReader : public IReader
    {
        Ret open(io::IODevice* device, const io::path_t& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool isContainer() const override;
        StringList fileList() const override;
        bool fileExists(const String& fileName) const override;
        ByteArray fileData(const String& fileName) const override;
    private:
        io::path_t m_rootPath;
    };

    struct XmlFileReader : public IReader
    {
        Ret open(io::IODevice* device, const io::path_t& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool isContainer() const override;
        StringList fileList() const override;
        bool fileExists(const String& fileName) const override;
        ByteArray fileData(const String& fileName) const override;
    private:
        io::IODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
    };

    IReader* reader() const;
    bool fileExists(const String& fileName) const;
    ByteArray fileData(const String& fileName) const;

    String mainFileName() const;

    Params m_params;
    mutable IReader* m_reader = nullptr;
};
}

#endif // MU_ENGRAVING_MSCREADER_H
