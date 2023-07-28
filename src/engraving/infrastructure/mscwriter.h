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
#ifndef MU_ENGRAVING_MSCWRITER_H
#define MU_ENGRAVING_MSCWRITER_H

#include "types/string.h"
#include "types/ret.h"
#include "io/path.h"
#include "io/iodevice.h"
#include "mscio.h"

namespace mu {
class ZipWriter;
class TextStream;
}

namespace mu::engraving {
class MscWriter
{
public:

    struct Params
    {
        io::IODevice* device = nullptr;
        io::path_t filePath;
        String mainFileName;
        MscIoMode mode = MscIoMode::Zip;
    };

    MscWriter() = default;
    MscWriter(const Params& params);
    ~MscWriter();

    void setParams(const Params& params);
    const Params& params() const;

    Ret open();
    void close();
    bool isOpened() const;
    bool hasError() const;

    void writeStyleFile(const ByteArray& data);
    void writeScoreFile(const ByteArray& data);
    void addExcerptStyleFile(const String& name, const ByteArray& data);
    void addExcerptFile(const String& name, const ByteArray& data);
    void writeChordListFile(const ByteArray& data);
    void writeThumbnailFile(const ByteArray& data);
    void addImageFile(const String& fileName, const ByteArray& data);
    void writeAudioFile(const ByteArray& data);
    void writeAudioSettingsJsonFile(const ByteArray& data);
    void writeViewSettingsJsonFile(const ByteArray& data, const io::path_t& pathPrefix = "");

private:

    struct IWriter {
        virtual ~IWriter() = default;

        virtual Ret open(io::IODevice* device, const io::path_t& filePath) = 0;
        virtual void close() = 0;
        virtual bool isOpened() const = 0;
        virtual bool hasError() const = 0;
        virtual bool addFileData(const String& fileName, const ByteArray& data) = 0;
    };

    struct ZipFileWriter : public IWriter
    {
        ~ZipFileWriter() override;
        Ret open(io::IODevice* device, const io::path_t& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool hasError() const override;
        bool addFileData(const String& fileName, const ByteArray& data) override;

    private:
        io::IODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
        ZipWriter* m_zip = nullptr;
    };

    struct DirWriter : public IWriter
    {
        Ret open(io::IODevice* device, const io::path_t& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool hasError() const override;
        bool addFileData(const String& fileName, const ByteArray& data) override;
    private:
        io::path_t m_rootPath;
        bool m_hasError = false;
    };

    struct XmlFileWriter : public IWriter
    {
        ~XmlFileWriter() override;
        Ret open(io::IODevice* device, const io::path_t& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool hasError() const override;
        bool addFileData(const String& fileName, const ByteArray& data) override;
    private:
        io::IODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
        TextStream* m_stream = nullptr;
    };

    struct Meta {
        std::vector<String> files;
        bool isWritten = false;

        bool contains(const String& file) const;
        void addFile(const String& file);
    };

    IWriter* writer() const;

    bool addFileData(const String& fileName, const ByteArray& data);

    void writeMeta();
    void writeContainer(const std::vector<String>& paths);

    String mainFileName() const;

    Params m_params;
    mutable IWriter* m_writer = nullptr;
    Meta m_meta;
    bool m_hadError = false;
};
}

#endif // MU_ENGRAVING_MSCWRITER_H
