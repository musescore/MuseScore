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
#include "workspacefile.h"

#include <QBuffer>

#include "log.h"
#include "stringutils.h"

#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"

#include "global/xmlreader.h"
#include "global/xmlwriter.h"

using namespace mu::workspace;
using namespace mu::framework;

static const QString WS_CONTAINTER_XML_PATH("META-INF/container.xml");

static constexpr std::string_view WS_CONTAINER_TAG("container");
static constexpr std::string_view WS_ROOTFILES_TAG("rootfiles");
static constexpr std::string_view WS_ROOTFILE_TAG("rootfile");
static constexpr std::string_view WS_FULLPATH_ATTRIBUTE("full-path");

WorkspaceFile::WorkspaceFile(const io::path& filePath)
    : m_filePath(filePath)
{}

QByteArray WorkspaceFile::readRootFile()
{
    RetVal<QByteArray> data = fileSystem()->readFile(m_filePath);
    if (!data.ret) {
        LOGE() << data.ret.toString();
        return QByteArray();
    }

    QBuffer buf(&data.val);
    MQZipReader zip(&buf);

    std::string rootFile;
    MetaInfo meta;

    if (meta.read(zip)) {
        rootFile = meta.rootFile();
    } else {
        QVector<MQZipReader::FileInfo> fis = zip.fileInfoList();
        if (!fis.isEmpty()) {
            rootFile = fis.first().filePath.toStdString();
        }
    }

    if (rootFile.empty()) {
        LOGE() << "not found root file: " << m_filePath;
        return QByteArray();
    }

    QByteArray fileData = zip.fileData(QString::fromStdString(rootFile));
    if (fileData.isEmpty()) {
        LOGE() << "failed read root file: " << m_filePath;
        return QByteArray();
    }

    return fileData;
}

bool WorkspaceFile::writeRootFile(const std::string& name, const QByteArray& data)
{
    MQZipWriter zip(m_filePath.toQString());

    MetaInfo meta;
    meta.setRootFile(name);
    meta.write(zip);

    zip.addFile(QString::fromStdString(name), data);

    bool ret = zip.status() == MQZipWriter::NoError;
    if (!ret) {
        LOGE() << "Error while writing workspace, zip status: " << zip.status();
    }

    return ret;
}

void WorkspaceFile::MetaInfo::setRootFile(const std::string& name)
{
    m_rootFile = name;
}

std::string WorkspaceFile::MetaInfo::rootFile() const
{
    return m_rootFile;
}

void WorkspaceFile::MetaInfo::write(MQZipWriter& zip)
{
    QByteArray data;
    writeContainer(&data);
    zip.addFile(WS_CONTAINTER_XML_PATH, data);
}

bool WorkspaceFile::MetaInfo::read(const MQZipReader& zip)
{
    QByteArray container = zip.fileData(WS_CONTAINTER_XML_PATH);
    if (container.isEmpty()) {
        LOGE() << "not found" << WS_CONTAINTER_XML_PATH;
        return false;
    }

    readContainer(container);
    if (m_rootFile.empty()) {
        return false;
    }

    return true;
}

void WorkspaceFile::MetaInfo::readContainer(const QByteArray& data)
{
    XmlReader reader(data);

    while (reader.readNextStartElement()) {
        if (WS_CONTAINER_TAG != reader.tagName()) {
            reader.skipCurrentElement();
            continue;
        }

        while (reader.readNextStartElement()) {
            if (WS_ROOTFILES_TAG != reader.tagName()) {
                reader.skipCurrentElement();
                continue;
            }

            while (reader.readNextStartElement()) {
                if (WS_ROOTFILE_TAG != reader.tagName()) {
                    reader.skipCurrentElement();
                    continue;
                }

                m_rootFile = reader.attribute(WS_FULLPATH_ATTRIBUTE);
                return;
            }
        }
    }
}

void WorkspaceFile::MetaInfo::writeContainer(QByteArray* data) const
{
    IF_ASSERT_FAILED(data) {
        return;
    }

    QBuffer buffer(data);
    buffer.open(IODevice::WriteOnly);

    XmlWriter writer(&buffer);
    writer.writeStartDocument();
    writer.writeStartElement(WS_CONTAINER_TAG);
    writer.writeStartElement(WS_ROOTFILES_TAG);

    writer.writeStartElement(WS_ROOTFILE_TAG);
    writer.writeAttribute(WS_FULLPATH_ATTRIBUTE, m_rootFile);
    writer.writeEndElement();

    writer.writeEndElement();
    writer.writeEndElement();
    writer.writeEndDocument();

    buffer.close();
}
