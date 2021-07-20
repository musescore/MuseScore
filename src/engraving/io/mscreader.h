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

class MQZipReader;

namespace mu::engraving {
class MscReader
{
public:
    enum class Mode {
        Zip,
        Dir
    };

    MscReader(const QString& filePath = QString(), Mode mode = Mode::Zip);
    MscReader(QIODevice* device);
    ~MscReader();

    void setDevice(QIODevice* device);
    void setFilePath(const QString& filePath);
    QString filePath() const;
    void setMode(Mode m);
    Mode mode() const;

    bool open();
    void close();
    bool isOpened() const;

    QByteArray readScoreFile() const;
    QByteArray readThumbnailFile() const;
    QByteArray readImageFile(const QString& fileName) const;
    std::vector<QString> imageFileNames() const;
    QByteArray readAudioFile() const;
    QByteArray readAudioSettingsJsonFile() const;

private:

    struct Meta {
        QString mscxFileName;
        std::vector<QString> imageFilePaths;

        bool isValid() const { return !mscxFileName.isEmpty(); }
    };

    QString rootPath() const;
    MQZipReader* reader() const;
    const Meta& meta() const;
    QByteArray fileData(const QString& fileName) const;

    QString m_filePath;
    Mode m_mode = Mode::Zip;
    QIODevice* m_device = nullptr;
    bool m_selfDeviceOwner = false;
    mutable MQZipReader* m_reader = nullptr;
    mutable Meta m_meta;
};
}

#endif // MU_ENGRAVING_MSCREADER_H
