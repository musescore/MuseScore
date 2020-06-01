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

#include "msmrfile.h"

#include <QFileInfo>
#include <QBuffer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"

#include "avslog.h"

using namespace Ms::Avs;

MsmrFile::MsmrFile(const QByteArray& data, const QString& name)
    : _data(data), _name(QFileInfo(name).baseName())
{
}

MsmrFile::~MsmrFile()
{
}

//---------------------------------------------------------
//   readMxl
//---------------------------------------------------------

QByteArray MsmrFile::readMxl()
{
    QBuffer buf(&_data);
    MQZipReader zip(&buf);

    MetaInf meta;
    meta.read(zip);

    QString scoreMxlPath = meta.fileByExt(".mxl");
    QByteArray scoreMxlData = zip.fileData(scoreMxlPath);
    if (scoreMxlData.isEmpty()) {
        LOGE() << "failed read mxl: " << scoreMxlPath;
        return QByteArray();
    }

    return scoreMxlData;
}

//---------------------------------------------------------
//   readMuzicXml
//---------------------------------------------------------

QByteArray MsmrFile::readMuzicXml()
{
    // compressed
    QByteArray scoreMxlData = readMxl();
    QBuffer scoreMxlBuf(&scoreMxlData);
    scoreMxlBuf.open(QIODevice::ReadOnly);

    // xml
    MQZipReader mxlZip(&scoreMxlBuf);

    MetaInf meta;
    meta.read(mxlZip);

    QString scoreXmlPath = meta.fileByExt(".xml");
    QByteArray scoreXmlData = mxlZip.fileData(scoreXmlPath);
    if (scoreXmlData.isEmpty()) {
        LOGE() << "failed read xml: " << scoreXmlPath;
        return QByteArray();
    }

    return scoreXmlData;
}

//---------------------------------------------------------
//   readOmr
//---------------------------------------------------------

QByteArray MsmrFile::readOmr()
{
    QBuffer buf(&_data);
    MQZipReader zip(&buf);

    MetaInf meta;
    meta.read(zip);

    QString ormPath = meta.fileByExt(".omr");
    QByteArray ormData = zip.fileData(ormPath);
    if (ormData.isEmpty()) {
        LOGE() << "failed read omr: " << ormPath;
        return QByteArray();
    }

    return ormData;
}

//---------------------------------------------------------
//   readMscz
//---------------------------------------------------------

QByteArray MsmrFile::readMscz()
{
    QBuffer buf(&_data);
    MQZipReader zip(&buf);

    MetaInf meta;
    meta.read(zip);

    QString msczPath = meta.fileByExt(".mscz");
    if (msczPath.isEmpty()) {   //! NOTE No error, maybe not
        LOGI() << "no mscz data";
        return QByteArray();
    }

    QByteArray msczData = zip.fileData(msczPath);
    if (msczData.isEmpty()) {
        LOGE() << "failed read mscz: " << msczPath;
        return QByteArray();
    }

    return msczData;
}

//---------------------------------------------------------
//   writeMscz
//---------------------------------------------------------

bool MsmrFile::writeMscz(const QByteArray& mscz)
{
    QByteArray mxl = readMxl();
    QByteArray omr = readOmr();

    //! NOTE MQZipWriter will overwrite the data,
    //! so we’ll clear it explicitly so as not to be misleading
    _data.clear();

    QBuffer buf(&_data);
    MQZipWriter zip(&buf);

    //! NOTE all items already compressed
    zip.setCompressionPolicy(MQZipWriter::NeverCompress);

    MetaInf meta;

    auto addFile = [&zip, &meta](const QString& path, const QByteArray& data) {
                       meta.addFile(path);
                       zip.addFile(path, data);
                   };

    addFile(_name + ".mxl", mxl);
    addFile(_name + ".omr", omr);
    addFile(_name + ".mscz", mscz);

    meta.write(zip);

    if (zip.status() != MQZipWriter::NoError) {
        LOGE() << "failed add mscz, zip status: " << zip.status();
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   writeTo
//---------------------------------------------------------

bool MsmrFile::writeTo(QIODevice* d)
{
    IF_ASSERT(d) {
        return false;
    }

    if (!d->isOpen()) {
        d->open(QIODevice::WriteOnly);
    }

    IF_ASSERT(d->isOpen()) {
        return false;
    }

    d->seek(0);
    qint64 size = d->write(_data);
    if (size != _data.size()) {
        LOGE() << "failed write data";
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   addFile
//---------------------------------------------------------

void MsmrFile::MetaInf::addFile(const QString& path)
{
    IF_ASSERT(!_containerFiles.contains(path)) {
        return;
    }

    _containerFiles << path;

    IF_ASSERT(!_storedFiles.contains(path)) {
        return;
    }

    _storedFiles << path;
}

//---------------------------------------------------------
//   fileByExt
//---------------------------------------------------------

QString MsmrFile::MetaInf::fileByExt(const QString& ext) const
{
    auto findByExt = [](const QStringList& list, const QString& ext) {
                         for (const QString& f : list) {
                             if (f.endsWith(ext)) {
                                 return f;
                             }
                         }
                         return QString();
                     };

    QString file = findByExt(_containerFiles, ext);

    //! NOTE If the file is not found in the container,
    //! then we will look among the stored files for backward compatibility
    //! and eliminating any errors
    if (file.isEmpty()) {
        file = findByExt(_storedFiles, ext);
    } else {
        if (!_storedFiles.contains(file)) {
            LOGE() << "not found file: " << file;
            file = findByExt(_storedFiles, ext);
        }
    }

    return file;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void MsmrFile::MetaInf::write(MQZipWriter& zip)
{
    QByteArray data;
    writeContainer(&data);
    zip.addFile("META-INF/container.xml", data);
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void MsmrFile::MetaInf::read(const MQZipReader& zip)
{
    // read container
    QByteArray container = zip.fileData("META-INF/container.xml");
    if (container.isEmpty()) {
        LOGW() << "not found META-INF/container.xml";
    } else {
        readContainer(container);
    }

    // read files list
    QVector<MQZipReader::FileInfo> fis = zip.fileInfoList();
    for (const MQZipReader::FileInfo& fi : fis) {
        _storedFiles << fi.filePath;
    }
}

//---------------------------------------------------------
//   readContainer
//---------------------------------------------------------

void MsmrFile::MetaInf::readContainer(const QByteArray& data)
{
    _containerFiles.clear();

    QXmlStreamReader xml(data);
    while (xml.readNextStartElement()) {
        if ("container" != xml.name()) {
            xml.skipCurrentElement();
            continue;
        }

        while (xml.readNextStartElement()) {
            if ("rootfiles" != xml.name()) {
                xml.skipCurrentElement();
                continue;
            }

            while (xml.readNextStartElement()) {
                if ("rootfile" != xml.name()) {
                    xml.skipCurrentElement();
                    continue;
                }

                QString path = xml.attributes().value("full-path").toString();
                _containerFiles << path;
            }
        }
    }
}

//---------------------------------------------------------
//   writeContainer
//---------------------------------------------------------

void MsmrFile::MetaInf::writeContainer(QByteArray* data) const
{
    IF_ASSERT(data) {
        return;
    }

    QXmlStreamWriter xml(data);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("container");
    xml.writeStartElement("rootfiles");

    for (const QString& f : _containerFiles) {
        xml.writeStartElement("rootfile");
        xml.writeAttribute("full-path", f);
        xml.writeEndElement();
    }

    xml.writeEndElement();
    xml.writeEndElement();
    xml.writeEndDocument();
}
