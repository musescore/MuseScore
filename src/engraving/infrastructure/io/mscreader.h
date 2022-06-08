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

#include <QString>
#include "io/iodevice.h"
#include "mscio.h"
#include "types/string.h"

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
        QString filePath;
        QString mainFileName;
        MscIoMode mode = MscIoMode::Zip;
    };

    MscReader() = default;
    MscReader(const Params& params);
    ~MscReader();

    void setParams(const Params& params);
    const Params& params() const;

    bool open();
    void close();
    bool isOpened() const;

    ByteArray readStyleFile() const;
    ByteArray readScoreFile() const;

    std::vector<QString> excerptNames() const;
    ByteArray readExcerptStyleFile(const QString& name) const;
    ByteArray readExcerptFile(const QString& name) const;

    ByteArray readChordListFile() const;
    ByteArray readThumbnailFile() const;

    std::vector<QString> imageFileNames() const;
    ByteArray readImageFile(const QString& fileName) const;

    ByteArray readAudioFile() const;
    ByteArray readAudioSettingsJsonFile() const;
    ByteArray readViewSettingsJsonFile() const;

private:

    struct IReader {
        virtual ~IReader() = default;

        virtual bool open(io::IODevice* device, const QString& filePath) = 0;
        virtual void close() = 0;
        virtual bool isOpened() const = 0;
        //! NOTE In the case of reading from a directory,
        //! it may happen that we are not reading a container (a directory with a certain structure),
        //! but only one file among others (`.mscx` from MU 3.x)
        virtual bool isContainer() const = 0;
        virtual StringList fileList() const = 0;
        virtual ByteArray fileData(const QString& fileName) const = 0;
    };

    struct ZipFileReader : public IReader
    {
        ~ZipFileReader() override;
        bool open(io::IODevice* device, const QString& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool isContainer() const override;
        StringList fileList() const override;
        ByteArray fileData(const QString& fileName) const override;
    private:
        io::IODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
        ZipReader* m_zip = nullptr;
    };

    struct DirReader : public IReader
    {
        bool open(io::IODevice* device, const QString& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool isContainer() const override;
        StringList fileList() const override;
        ByteArray fileData(const QString& fileName) const override;
    private:
        QString m_rootPath;
    };

    struct XmlFileReader : public IReader
    {
        bool open(io::IODevice* device, const QString& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool isContainer() const override;
        StringList fileList() const override;
        ByteArray fileData(const QString& fileName) const override;
    private:
        io::IODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
    };

    IReader* reader() const;
    ByteArray fileData(const QString& fileName) const;

    QString mainFileName() const;

    Params m_params;
    mutable IReader* m_reader = nullptr;
};
}

#endif // MU_ENGRAVING_MSCREADER_H
