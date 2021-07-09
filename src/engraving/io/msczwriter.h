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
#ifndef MU_ENGRAVING_MSCZWRITER_H
#define MU_ENGRAVING_MSCZWRITER_H

#include <QString>
#include <QByteArray>
#include <QIODevice>

class MQZipWriter;

namespace mu::engraving {
class MsczWriter
{
public:
    MsczWriter(const QString& filePath = QString());
    MsczWriter(QIODevice* device);
    ~MsczWriter();

    void setDevice(QIODevice* device);
    void setFilePath(const QString& filePath);
    QString filePath() const;

    bool open();
    void close();
    bool isOpened() const;

    void writeScore(const QByteArray& data);
    void writeThumbnail(const QByteArray& data);
    void addImage(const QString& fileName, const QByteArray& data);
    void writeAudio(const QByteArray& data);

private:

    struct Meta {
        QString mscxFileName;
        std::vector<QString> imageFilePaths;
        QString audioFile;

        bool isWrited = false;

        bool isValid() const { return !mscxFileName.isEmpty(); }
    };

    MQZipWriter* writer() const;
    bool addFileData(const QString& fileName, const QByteArray& data);

    void writeMeta();
    void writeContainer(const std::vector<QString>& paths);

    QString m_filePath;
    QIODevice* m_device = nullptr;
    bool m_selfDeviceOwner = false;
    mutable MQZipWriter* m_writer = nullptr;
    Meta m_meta;
};
}

#endif // MU_ENGRAVING_MSCZWRITER_H
