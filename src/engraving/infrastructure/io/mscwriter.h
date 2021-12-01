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
#include <QIODevice>

#include "mscio.h"

class MQZipWriter;
class QTextStream;

namespace mu::engraving {
class MscWriter
{
public:

    struct Params
    {
        QIODevice* device = nullptr;
        QString filePath;
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

    void writeStyleFile(const QByteArray& data);
    void writeScoreFile(const QByteArray& data);
    void addExcerptStyleFile(const QString& name, const QByteArray& data);
    void addExcerptFile(const QString& name, const QByteArray& data);
    void writeChordListFile(const QByteArray& data);
    void writeThumbnailFile(const QByteArray& data);
    void addImageFile(const QString& fileName, const QByteArray& data);
    void writeAudioFile(const QByteArray& data);
    void writeAudioSettingsJsonFile(const QByteArray& data);
    void writeViewSettingsJsonFile(const QByteArray& data);

private:

    struct IWriter {
        virtual ~IWriter() = default;

        virtual bool open(QIODevice* device, const QString& filePath) = 0;
        virtual void close() = 0;
        virtual bool isOpened() const = 0;
        virtual bool addFileData(const QString& fileName, const QByteArray& data) = 0;
    };

    struct ZipWriter : public IWriter
    {
        ~ZipWriter() override;
        bool open(QIODevice* device, const QString& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool addFileData(const QString& fileName, const QByteArray& data) override;

    private:
        QIODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
        MQZipWriter* m_zip = nullptr;
    };

    struct DirWriter : public IWriter
    {
        bool open(QIODevice* device, const QString& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool addFileData(const QString& fileName, const QByteArray& data) override;
    private:
        QString m_rootPath;
    };

    struct XmlFileWriter : public IWriter
    {
        ~XmlFileWriter() override;
        bool open(QIODevice* device, const QString& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool addFileData(const QString& fileName, const QByteArray& data) override;
    private:
        QIODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
        QTextStream* m_stream = nullptr;
    };

    struct Meta {
        std::vector<QString> files;
        bool isWrited = false;

        bool contains(const QString& file) const;
        void addFile(const QString& file);
    };

    IWriter* writer() const;
    bool addFileData(const QString& fileName, const QByteArray& data);

    void writeMeta();
    void writeContainer(const std::vector<QString>& paths);

    Params m_params;
    mutable IWriter* m_writer = nullptr;
    Meta m_meta;
};
}

#endif // MU_ENGRAVING_MSCWRITER_H
