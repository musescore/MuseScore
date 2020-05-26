//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef AVS_MSMRFILE_H
#define AVS_MSMRFILE_H

#include <QIODevice>
#include <QByteArray>
#include <QBuffer>

class MQZipReader;
class MQZipWriter;

namespace Ms {
namespace Avs {
class MsmrFile
{
public:
    MsmrFile(const QByteArray& data, const QString& name);
    ~MsmrFile();

    QByteArray readMxl();   // compressed MuzicXml
    QByteArray readMuzicXml();
    QByteArray readOmr();

    QByteArray readMscz();
    bool writeMscz(const QByteArray& mscz);   // add or replace

    bool writeTo(QIODevice* d);

private:

    struct MetaInf {
        void addFile(const QString& path);
        QString fileByExt(const QString& ext) const;

        void write(MQZipWriter& zip);
        void read(const MQZipReader& zip);

    private:

        void readContainer(const QByteArray& data);
        void writeContainer(QByteArray* data) const;

        QStringList _containerFiles;
        QStringList _storedFiles;
    };

    QByteArray _data;
    QString _name;
};
} // Avs
} // Ms

#endif // AVS_MSMRFILE_H
