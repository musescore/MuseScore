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
    ~MsczFile();

    struct Info {
        QString mscxFileName;
    };

    void setFilePath(const QString& filePath);
    QString filePath() const;

    bool open();
    bool flush();
    void close();

    const Info& info() const;

    QByteArray mscx() const;
    void setMscx(const QString& fileName, const QByteArray& data);

    QByteArray thumbnail() const;
    void setThumbnail(const QByteArray& data);

private:

    // meta
    bool readInfo(Info& info) const;
    void writeMeta(MQZipWriter& zip, const std::map<QString, QString>& meta);
    void readMeta(MQZipReader& zip, std::map<QString, QString>& meta);
    void writeContainer(MQZipWriter& zip, const std::vector<QString>& paths);

    // raw access
    QByteArray fileData(const QString& fileName) const;
    bool addFileData(const QString& fileName, const QByteArray& data);

    mutable QFile m_file;
    Info m_info;
};
}

#endif // MU_ENGRAVING_MUMSCZFILE_H
