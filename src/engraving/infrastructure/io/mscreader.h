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
#include <QByteArray>
#include <QIODevice>

#include "mscio.h"

class MQZipReader;
class QXmlStreamReader;

namespace mu::engraving {
class MscReader
{
public:

    struct Params
    {
        QIODevice* device = nullptr;
        QString filePath;
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

    QByteArray readStyleFile() const;
    QByteArray readScoreFile() const;

    std::vector<QString> excerptNames() const;
    QByteArray readExcerptStyleFile(const QString& name) const;
    QByteArray readExcerptFile(const QString& name) const;

    QByteArray readChordListFile() const;
    QByteArray readThumbnailFile() const;

    std::vector<QString> imageFileNames() const;
    QByteArray readImageFile(const QString& fileName) const;

    QByteArray readAudioFile() const;
    QByteArray readAudioSettingsJsonFile() const;
    QByteArray readViewSettingsJsonFile() const;

private:

    struct IReader {
        virtual ~IReader() = default;

        virtual bool open(QIODevice* device, const QString& filePath) = 0;
        virtual void close() = 0;
        virtual bool isOpened() const = 0;
        //! NOTE In the case of reading from a directory,
        //! it may happen that we are not reading a container (a directory with a certain structure),
        //! but only one file among others (`.mscx` from MU 3.x)
        virtual bool isContainer() const = 0;
        virtual QStringList fileList() const = 0;
        virtual QByteArray fileData(const QString& fileName) const = 0;
    };

    struct ZipReader : public IReader
    {
        ~ZipReader() override;
        bool open(QIODevice* device, const QString& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool isContainer() const override;
        QStringList fileList() const override;
        QByteArray fileData(const QString& fileName) const override;
    private:
        QIODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
        MQZipReader* m_zip = nullptr;
    };

    struct DirReader : public IReader
    {
        bool open(QIODevice* device, const QString& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool isContainer() const override;
        QStringList fileList() const override;
        QByteArray fileData(const QString& fileName) const override;
    private:
        QString m_rootPath;
    };

    struct XmlFileReader : public IReader
    {
        bool open(QIODevice* device, const QString& filePath) override;
        void close() override;
        bool isOpened() const override;
        bool isContainer() const override;
        QStringList fileList() const override;
        QByteArray fileData(const QString& fileName) const override;
    private:
        QIODevice* m_device = nullptr;
        bool m_selfDeviceOwner = false;
    };

    IReader* reader() const;
    QByteArray fileData(const QString& fileName) const;

    Params m_params;
    mutable IReader* m_reader = nullptr;
};
}

#endif // MU_ENGRAVING_MSCREADER_H
