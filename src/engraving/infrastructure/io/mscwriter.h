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

#include <QString>
#include <QByteArray>

#include "io/iodevice.h"
#include "mscio.h"

class QTextStream;

namespace mu {
class ZipWriter;
}

namespace mu::engraving {
class MscWriter
{
public:

    struct Params
    {
        io::IODevice* device = nullptr;
        QString filePath;
        QString mainFileName;
        MscIoMode mode = MscIoMode::Zip;
    };

    MscWriter() = default;
    MscWriter(const Params& params);
    ~MscWriter();

    void setParams(const Params& params);
    const Params& params() const;

    bool open();
    void close();
    bool isOpened() const;

    void writeStyleFile(const io::ByteArray& data);
    void writeScoreFile(const io::ByteArray& data);
    void addExcerptStyleFile(const QString& name, const io::ByteArray& data);
    void addExcerptFile(const QString& name, const io::ByteArray& data);
    void writeChordListFile(const io::ByteArray& data);
    void writeThumbnailFile(const io::ByteArray& data);
    void addImageFile(const QString& fileName, const io::ByteArray& data);
    void writeAudioFile(const io::ByteArray& data);
    void writeAudioSettingsJsonFile(const io::ByteArray& data);
    void writeViewSettingsJsonFile(const io::ByteArray& data);

private:

    struct IWriter {
        virtual ~IWriter() = default;

        virtual bool open(io::IODevice* device, const QString& filePath) = 0;
        virtual void close() = 0;
        virtual bool isOpened() const = 0;
        virtual bool addFileData(const QString& fileName, const QByteArray& data) = 0;
    };

    struct ZipFileWriter : public IWriter
    {
        ~ZipFileWriter() override;
        bool open(io::IODevice* device, const QString& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool addFileData(const QString& fileName, const QByteArray& data) override;

    private:
        io::IODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
        ZipWriter* m_zip = nullptr;
    };

    struct DirWriter : public IWriter
    {
        bool open(io::IODevice* device, const QString& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool addFileData(const QString& fileName, const QByteArray& data) override;
    private:
        QString m_rootPath;
    };

    struct XmlFileWriter : public IWriter
    {
        ~XmlFileWriter() override;
        bool open(io::IODevice* device, const QString& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool addFileData(const QString& fileName, const QByteArray& data) override;
    private:
        io::IODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
        QTextStream* m_stream = nullptr;
        QString m_data;
    };

    struct Meta {
        std::vector<QString> files;
        bool isWritten = false;

        bool contains(const QString& file) const;
        void addFile(const QString& file);
    };

    IWriter* writer() const;
    bool addFileData(const QString& fileName, const QByteArray& data);
    bool addFileData(const QString& fileName, const io::ByteArray& data);

    void writeMeta();
    void writeContainer(const std::vector<QString>& paths);

    QString mainFileName() const;

    Params m_params;
    mutable IWriter* m_writer = nullptr;
    Meta m_meta;
};
}

#endif // MU_ENGRAVING_MSCWRITER_H
