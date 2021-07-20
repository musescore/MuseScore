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
    enum class Mode {
        Zip,
        Dir
    };

    MsczWriter(const QString& filePath = QString(), Mode mode = Mode::Zip);
    MsczWriter(QIODevice* device);
    ~MsczWriter();

    void setDevice(QIODevice* device);
    void setFilePath(const QString& filePath);
    QString filePath() const;
    void setMode(Mode m);
    Mode mode() const;

    bool open();
    void close();
    bool isOpened() const;

    void writeScoreFile(const QByteArray& data);
    void writeThumbnailFile(const QByteArray& data);
    void addImageFile(const QString& fileName, const QByteArray& data);
    void writeAudioFile(const QByteArray& data);
    void writeAudioSettingsJsonFile(const QByteArray& data);

private:

    struct Meta {
        std::vector<QString> files;
        bool isWrited = false;

        bool contains(const QString& file) const
        {
            if (std::find(files.begin(), files.end(), file) != files.end()) {
                return true;
            }
            return false;
        }

        void addFile(const QString& file)
        {
            if (!contains(file)) {
                files.push_back(file);
            }
        }
    };

    QString rootPath() const;

    MQZipWriter* writer() const;
    bool addFileData(const QString& fileName, const QByteArray& data);

    void writeMeta();
    void writeContainer(const std::vector<QString>& paths);

    QString m_filePath;
    Mode m_mode = Mode::Zip;
    QIODevice* m_device = nullptr;
    bool m_selfDeviceOwner = false;
    mutable MQZipWriter* m_writer = nullptr;
    Meta m_meta;
};
}

#endif // MU_ENGRAVING_MSCZWRITER_H
