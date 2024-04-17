/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "mscmetareader.h"

#include <sstream>

#include "io/buffer.h"

#include "stringutils.h"
#include "global/deprecated/xmlreader.h"
#include "engraving/infrastructure/mscreader.h"

#include "log.h"

using namespace muse;
using namespace muse::io;
using namespace mu::project;
using namespace mu::engraving;

RetVal<ProjectMeta> MscMetaReader::readMeta(const muse::io::path_t& filePath) const
{
    MscReader msczReader;
    Ret ret = prepareReader(filePath, msczReader);
    if (!ret) {
        return ret;
    }

    // Read score meta
    ByteArray scoreData = msczReader.readScoreFile();
    deprecated::XmlReader xmlReader(scoreData.toQByteArray());

    RetVal<ProjectMeta> meta;
    meta.ret = make_ok();
    doReadMeta(xmlReader, meta.val);

    // Read thumbnail
    ByteArray thumbnailData = msczReader.readThumbnailFile();
    if (thumbnailData.empty()) {
        LOGD() << "Can't find thumbnail";
    } else {
        meta.val.thumbnail.loadFromData(thumbnailData.toQByteArray(), "PNG");
    }

    meta.val.filePath = filePath;

    return meta;
}

muse::RetVal<CloudProjectInfo> MscMetaReader::readCloudProjectInfo(const muse::io::path_t& filePath) const
{
    TRACEFUNC;

    MscReader msczReader;
    Ret ret = prepareReader(filePath, msczReader);
    if (!ret) {
        return ret;
    }

    // Read score meta
    ByteArray scoreData = msczReader.readScoreFile();
    deprecated::XmlReader xmlReader(scoreData.toQByteArray());

    ProjectMeta meta;
    doReadMeta(xmlReader, meta);

    RetVal<CloudProjectInfo> info;
    info.ret = make_ok();
    info.val.sourceUrl = meta.source;
    info.val.revisionId = meta.additionalTags[SOURCE_REVISION_ID_TAG].toInt();

    return info;
}

Ret MscMetaReader::prepareReader(const muse::io::path_t& filePath, MscReader& reader) const
{
    Ret ret = fileSystem()->exists(filePath);
    if (!ret) {
        LOGE() << "File not exists: " << filePath;
        return ret;
    }

    MscReader::Params params;
    params.filePath = filePath.toQString();
    params.mode = mscIoModeBySuffix(io::suffix(filePath));
    if (params.mode == MscIoMode::Unknown) {
        return make_ret(Ret::Code::InternalError);
    }

    reader.setParams(params);
    if (!reader.open()) {
        return make_ret(Ret::Code::InternalError);
    }

    return make_ok();
}

MscMetaReader::RawMeta MscMetaReader::doReadBox(deprecated::XmlReader& xmlReader) const
{
    RawMeta meta;

    while (xmlReader.readNextStartElement()) {
        if (xmlReader.tagName() == "Text") {
            bool isTitle = false;
            bool isSubtitle = false;
            bool isComposer = false;
            bool isLyricist = false;
            while (xmlReader.readNextStartElement()) {
                std::string tag(xmlReader.tagName());

                if (tag == "style") {
                    std::string val = strings::toLower(xmlReader.readString());

                    if (val == "title" || val == "2") {
                        isTitle = true;
                    } else if (val == "composer" || val == "4") {
                        isComposer = true;
                    } else if (val == "subtitle") {
                        isSubtitle = true;
                    } else if (val == "lyricist") {
                        isLyricist = true;
                    } else {
                        xmlReader.skipCurrentElement();
                    }
                } else if (tag == "text") {
                    if (isTitle) {
                        meta.titleStyle = readText(xmlReader);
                    } else if (isSubtitle) {
                        meta.subtitleStyle = readText(xmlReader);
                    } else if (isComposer) {
                        meta.composerStyle = readText(xmlReader);
                    } else if (isLyricist) {
                        meta.lyricistStyle = readText(xmlReader);
                    } else {
                        xmlReader.skipCurrentElement();
                    }
                } else if (tag == "html-data") {
                    if (isTitle) {
                        meta.titleStyleHtml = readText(xmlReader);
                    } else if (isSubtitle) {
                        meta.subtitleStyleHtml = readText(xmlReader);
                    } else if (isComposer) {
                        meta.composerStyleHtml = readText(xmlReader);
                    } else if (isLyricist) {
                        meta.lyricistStyleHtml = readText(xmlReader);
                    } else {
                        xmlReader.skipCurrentElement();
                    }
                } else {
                    xmlReader.skipCurrentElement();
                }
            }
        } else {
            xmlReader.skipCurrentElement();
        }
    }

    return meta;
}

MscMetaReader::RawMeta MscMetaReader::doReadRawMeta(deprecated::XmlReader& xmlReader) const
{
    RawMeta meta;

    while (xmlReader.readNextStartElement()) {
        std::string tag(xmlReader.tagName());

        if (tag == "work-title") {
            meta.titleTag = QString::fromStdString(xmlReader.readString());
        } else if (tag == "metaTag") {
            std::string name = xmlReader.attribute("name");

            if (name == "workTitle") {
                meta.titleAttribute = readMetaTagText(xmlReader);
            } else if (name == "composer") {
                meta.composerAttribute = readMetaTagText(xmlReader);
            } else if (name == "arranger") {
                meta.arranger = readMetaTagText(xmlReader);
            } else if (name == "lyricist") {
                meta.lyricistAttribute = readMetaTagText(xmlReader);
            } else if (name == "copyright") {
                meta.copyright = readMetaTagText(xmlReader);
            } else if (name == "translator") {
                meta.translator = readMetaTagText(xmlReader);
            } else if (name == "creationDate") {
                meta.creationDate = readMetaTagText(xmlReader);
            } else {
                meta.additionalTags[QString::fromStdString(name)] = readMetaTagText(xmlReader);
            }
        } else if (tag == "Staff") {
            if (meta.titleStyle.isEmpty()) {
                while (xmlReader.readNextStartElement()) {
                    std::string boxTag(xmlReader.tagName());

                    if (boxTag == "HBox"
                        || boxTag == "VBox"
                        || boxTag == "TBox"
                        || boxTag == "FBox") {
                        RawMeta boxMeta = doReadBox(xmlReader);

                        meta.titleStyle = boxMeta.titleStyle;
                        meta.titleStyleHtml = boxMeta.titleStyleHtml;
                        meta.subtitleStyle = boxMeta.subtitleStyle;
                        meta.subtitleStyleHtml = boxMeta.subtitleStyleHtml;
                        meta.composerStyle = boxMeta.composerStyle;
                        meta.composerStyleHtml = boxMeta.composerStyleHtml;
                        meta.lyricistStyle = boxMeta.lyricistStyle;
                        meta.lyricistStyleHtml = boxMeta.lyricistStyleHtml;
                    } else {
                        xmlReader.skipCurrentElement();
                    }
                }
            } else {
                xmlReader.skipCurrentElement();
            }
        } else if (tag == "Part") {
            meta.partsCount++;
            xmlReader.skipCurrentElement();
        } else {
            xmlReader.skipCurrentElement();
        }
    }

    return meta;
}

void MscMetaReader::doReadMeta(deprecated::XmlReader& xmlReader, ProjectMeta& meta) const
{
    RawMeta rawMeta;

    while (xmlReader.readNextStartElement()) {
        if (xmlReader.tagName() == "museScore") {
            std::string version = xmlReader.attribute("version");
            bool suitedVersion = version.rfind("1", 0) == 0;

            if (suitedVersion) {
                rawMeta = doReadRawMeta(xmlReader);
            } else {
                while (xmlReader.readNextStartElement()) {
                    if (xmlReader.tagName() == "Score") {
                        rawMeta = doReadRawMeta(xmlReader);
                    } else {
                        xmlReader.skipCurrentElement();
                    }
                }
            }
        } else {
            xmlReader.skipCurrentElement();
        }
    }

    if (!rawMeta.titleStyle.isEmpty()) {
        meta.title = simplified(rawMeta.titleStyle);
    } else if (!rawMeta.titleStyleHtml.isEmpty()) {
        meta.title = simplified(rawMeta.titleStyleHtml);
    } else if (!rawMeta.titleAttribute.isEmpty()) {
        meta.title = simplified(rawMeta.titleAttribute);
    } else {
        meta.title = simplified(rawMeta.titleTag);
    }

    if (!rawMeta.subtitleStyle.isEmpty()) {
        meta.subtitle = simplified(rawMeta.subtitleStyle);
    } else if (!rawMeta.subtitleStyleHtml.isEmpty()) {
        meta.subtitle = simplified(rawMeta.subtitleStyleHtml);
    }

    if (!rawMeta.composerStyle.isEmpty()) {
        meta.composer = simplified(rawMeta.composerStyle);
    } else if (!rawMeta.composerStyleHtml.isEmpty()) {
        meta.composer = simplified(rawMeta.composerStyleHtml);
    } else {
        meta.composer = simplified(rawMeta.composerAttribute);
    }

    if (!rawMeta.lyricistStyle.isEmpty()) {
        meta.lyricist = simplified(rawMeta.lyricistStyle);
    } else if (!rawMeta.lyricistStyleHtml.isEmpty()) {
        meta.lyricist = simplified(rawMeta.lyricistStyleHtml);
    } else {
        meta.lyricist = simplified(rawMeta.lyricistAttribute);
    }

    meta.copyright = simplified(rawMeta.copyright);
    meta.translator = simplified(rawMeta.translator);
    meta.arranger = simplified(rawMeta.arranger);
    meta.partsCount = rawMeta.partsCount;
    meta.creationDate = QDate::fromString(rawMeta.creationDate, "yyyy-MM-dd");
    meta.additionalTags = std::move(rawMeta.additionalTags);
}

QString MscMetaReader::formatFromXml(const std::string& xml) const
{
    size_t bbegin = xml.find("<body");

    if (bbegin != std::string::npos) {
        size_t bend = xml.find("</body>", bbegin);
        std::string body = xml.substr(bbegin, (bend - bbegin));
        return QString::fromStdString(MscMetaReader::cutXmlTags(body));
    }
    return QString::fromStdString(MscMetaReader::cutXmlTags(xml));
}

QString MscMetaReader::format(const std::string& str) const
{
    std::string fin = MscMetaReader::cutXmlTags(str);
    return simplified(fin);
}

QString MscMetaReader::simplified(const QString& str) const
{
    return simplified(str.toStdString());
}

QString MscMetaReader::simplified(const std::string& str) const
{
    std::string fin;
    for (size_t index = 0; str[index]; ++index) {
        if (str[index] == '\"' || str[index] == '\n') {
            fin += '\\';
        }
        fin += str[index];
    }

    return QString::fromStdString(fin);
}

std::string MscMetaReader::cutXmlTags(const std::string& str) const
{
    std::string fin;

    for (size_t index = 0; str[index]; ++index) {
        if (str[index] == '<') {
            for (; str[index + 1] && str[index] != '>'; ++index) {
            }
            continue;
        }
        fin += str[index];
    }

    return fin;
}

QString MscMetaReader::readText(deprecated::XmlReader& xmlReader) const
{
    std::string str = xmlReader.readString(deprecated::XmlReader::IncludeChildElements);
    return formatFromXml(str);
}

QString MscMetaReader::readMetaTagText(deprecated::XmlReader& xmlReader) const
{
    return QString::fromStdString(xmlReader.readString());
}
