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
#ifndef MU_ENGRAVING_MSCWRITER_H
#define MU_ENGRAVING_MSCWRITER_H

#include "types/string.h"
#include "types/ret.h"
#include "io/path.h"
#include "io/iodevice.h"
#include "mscio.h"

namespace muse {
class ZipWriter;
class TextStream;
}

namespace mu::engraving {
class MscWriter
{
public:

    struct Params
    {
        muse::io::IODevice* device = nullptr;
        muse::io::path_t filePath;
        muse::String mainFileName;
        MscIoMode mode = MscIoMode::Zip;
    };

    MscWriter() = default;
    MscWriter(const Params& params);
    ~MscWriter();

    void setParams(const Params& params);
    const Params& params() const;

    muse::Ret open();
    void close();
    bool isOpened() const;
    bool hasError() const;

    void writeStyleFile(const muse::ByteArray& data);
    void writeScoreFile(const muse::ByteArray& data);
    void addExcerptStyleFile(const muse::String& excerptFileName, const muse::ByteArray& data);
    void addExcerptFile(const muse::String& excerptFileName, const muse::ByteArray& data);
    void writeChordListFile(const muse::ByteArray& data);
    void writeThumbnailFile(const muse::ByteArray& data);
    void addImageFile(const muse::String& fileName, const muse::ByteArray& data);
    void writeAudioFile(const muse::ByteArray& data);
    void writeAudioSettingsJsonFile(const muse::ByteArray& data, const muse::io::path_t& pathPrefix = "");
    void writeViewSettingsJsonFile(const muse::ByteArray& data, const muse::io::path_t& pathPrefix = "");

private:

    struct IWriter {
        virtual ~IWriter() = default;

        virtual muse::Ret open(muse::io::IODevice* device, const muse::io::path_t& filePath) = 0;
        virtual void close() = 0;
        virtual bool isOpened() const = 0;
        virtual bool hasError() const = 0;
        virtual bool addFileData(const muse::String& fileName, const muse::ByteArray& data) = 0;
    };

    struct ZipFileWriter : public IWriter
    {
        ~ZipFileWriter() override;
        muse::Ret open(muse::io::IODevice* device, const muse::io::path_t& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool hasError() const override;
        bool addFileData(const muse::String& fileName, const muse::ByteArray& data) override;

    private:
        muse::io::IODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
        muse::ZipWriter* m_zip = nullptr;
    };

    struct DirWriter : public IWriter
    {
        muse::Ret open(muse::io::IODevice* device, const muse::io::path_t& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool hasError() const override;
        bool addFileData(const muse::String& fileName, const muse::ByteArray& data) override;
    private:
        muse::io::path_t m_rootPath;
        bool m_hasError = false;
    };

    struct XmlFileWriter : public IWriter
    {
        ~XmlFileWriter() override;
        muse::Ret open(muse::io::IODevice* device, const muse::io::path_t& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool hasError() const override;
        bool addFileData(const muse::String& fileName, const muse::ByteArray& data) override;
    private:
        muse::io::IODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
        muse::TextStream* m_stream = nullptr;
    };

    struct Meta {
        std::vector<muse::String> files;
        bool isWritten = false;

        bool contains(const muse::String& file) const;
        void addFile(const muse::String& file);
    };

    IWriter* writer() const;

    bool addFileData(const muse::String& fileName, const muse::ByteArray& data);

    void writeMeta();
    void writeContainer(const std::vector<muse::String>& paths);

    muse::String mainFileName() const;

    Params m_params;
    mutable IWriter* m_writer = nullptr;
    Meta m_meta;
    bool m_hadError = false;
};
}

#endif // MU_ENGRAVING_MSCWRITER_H
