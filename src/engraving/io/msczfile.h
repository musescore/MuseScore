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
#ifndef MU_ENGRAVING_MUMSCZFILE_H
#define MU_ENGRAVING_MUMSCZFILE_H

#include <QByteArray>
#include <QString>
#include <QFile>

class MQZipReader;
class MQZipWriter;

namespace mu::engraving {
class MsczFile
{
public:
    MsczFile(const QString& filePath = QString());
    MsczFile(QIODevice* device);
    ~MsczFile();

    struct Meta {
        QString mscxFileName;
    };

    void setFilePath(const QString& filePath);
    QString filePath() const;

    bool exists() const;
    bool isWritable() const;

    bool open();
    bool flush();
    void close();
    bool isOpened() const;

    const Meta& meta() const;

    QByteArray readMscx() const;
    void writeMscx(const QByteArray& data);

    std::vector<QByteArray> readImages() const;
    void addImage(const QString& name, const QByteArray& data);

    QByteArray thumbnail() const;
    void writeThumbnail(const QByteArray& data);

private:

    // meta
    bool readMeta(Meta& info) const;
    void writeMetaData(MQZipWriter& zip, const std::map<QString, QString>& meta);
    void readMetaData(MQZipReader& zip, std::map<QString, QString>& meta);
    void writeContainer(MQZipWriter& zip, const std::vector<QString>& paths);

    // raw access
    QByteArray fileData(const QString& fileName) const;
    bool addFileData(const QString& fileName, const QByteArray& data);

    mutable QFile m_file;
    QIODevice* m_device = nullptr;
    bool m_selfDeviceOwner = false;
    QString m_filePath;
    Meta m_meta;
};
}

#endif // MU_ENGRAVING_MUMSCZFILE_H
