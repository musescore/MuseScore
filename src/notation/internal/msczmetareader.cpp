#include "msczmetareader.h"

#include <sstream>

#include <QFileInfo>
#include <QBuffer>

#include "log.h"
#include "stringutils.h"
#include "notationerrors.h"

#include "thirdparty/qzip/qzipreader_p.h"

#include "framework/global/xmlreader.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::framework;

RetVal<Meta> MsczMetaReader::readMeta(const io::path& filePath) const
{
    RetVal<Meta> meta;

    QFileInfo fileInfo(filePath.toQString());
    if (!fileInfo.exists()) {
        LOGE() << "File not exists: " << filePath;
        meta.ret = make_ret(Err::FileNotFound);
        return meta;
    }

    bool compressed = fileInfo.suffix() == "mscz";

    if (compressed) {
        meta = loadCompressedMsc(filePath);
    } else {
        framework::XmlReader reader(filePath);
        meta = doReadMeta(reader);
    }

    if (meta.val.fileName.isEmpty()) {
        meta.val.fileName = fileInfo.baseName();
    }

    meta.val.filePath = fileInfo.absoluteFilePath();

    return meta;
}

RetVal<Meta> MsczMetaReader::loadCompressedMsc(const io::path& filePath) const
{
    RetVal<Meta> meta;

    QFile file(filePath.toQString());
    if (!file.open(QIODevice::ReadOnly)) {
        LOGE() << "Failed open file: " << filePath;
        meta.ret = make_ret(Err::FileOpenError);
        return meta;
    }

    QByteArray data = file.readAll();

    QBuffer buffer(&data);

    MQZipReader zipReader(&buffer);

    io::path rootFile = readRootFile(&zipReader);
    if (rootFile.empty()) {
        meta.ret = make_ret(Err::FileNoRootFile);
        return meta;
    }

    QByteArray rootBuffer = zipReader.fileData(rootFile.toQString());
    if (rootBuffer.isEmpty()) {
        auto fil = zipReader.fileInfoList();
        for (const MQZipReader::FileInfo& fi : fil) {
            if (mu::strings::endsWith(fi.filePath.toStdString(), ".mscx")) {
                rootBuffer = zipReader.fileData(fi.filePath);
                break;
            }
        }
    }

    framework::XmlReader xmlReader(rootBuffer);
    meta = doReadMeta(xmlReader);

    meta.val.thumbnail = loadThumbnail(&zipReader);

    return meta;
}

MsczMetaReader::RawMeta MsczMetaReader::doReadBox(framework::XmlReader& xmlReader) const
{
    RawMeta meta;

    while (xmlReader.readNextStartElement()) {
        if (xmlReader.tagName() == "Text") {
            bool isTitle = false;
            bool isSubtitle = false;
            bool isComposer = false;
            bool isLiricist = false;
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
                        isLiricist = true;
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
                    } else if (isLiricist) {
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
                    } else if (isLiricist) {
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

MsczMetaReader::RawMeta MsczMetaReader::doReadRawMeta(framework::XmlReader& xmlReader) const
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
                xmlReader.skipCurrentElement();
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

RetVal<Meta> MsczMetaReader::doReadMeta(framework::XmlReader& xmlReader) const
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

    RetVal<Meta> meta(make_ret(Err::NoError));
    if (!rawMeta.titleStyle.isEmpty()) {
        meta.val.title = simplified(rawMeta.titleStyle);
    } else if (!rawMeta.titleStyleHtml.isEmpty()) {
        meta.val.title = simplified(rawMeta.titleStyleHtml);
    } else if (!rawMeta.titleAttribute.isEmpty()) {
        meta.val.title = simplified(rawMeta.titleAttribute);
    } else {
        meta.val.title = simplified(rawMeta.titleTag);
    }

    if (!rawMeta.subtitleStyle.isEmpty()) {
        meta.val.subtitle = simplified(rawMeta.subtitleStyle);
    } else if (!rawMeta.subtitleStyleHtml.isEmpty()) {
        meta.val.subtitle = simplified(rawMeta.subtitleStyleHtml);
    }

    if (!rawMeta.composerStyle.isEmpty()) {
        meta.val.composer = simplified(rawMeta.composerStyle);
    } else if (!rawMeta.composerStyleHtml.isEmpty()) {
        meta.val.composer = simplified(rawMeta.composerStyleHtml);
    } else {
        meta.val.composer = simplified(rawMeta.composerAttribute);
    }

    if (!rawMeta.lyricistStyle.isEmpty()) {
        meta.val.lyricist = simplified(rawMeta.lyricistStyle);
    } else if (!rawMeta.lyricistStyleHtml.isEmpty()) {
        meta.val.lyricist = simplified(rawMeta.lyricistStyleHtml);
    } else {
        meta.val.lyricist = simplified(rawMeta.lyricistAttribute);
    }

    meta.val.copyright = simplified(rawMeta.copyright);
    meta.val.translator = simplified(rawMeta.translator);
    meta.val.arranger = simplified(rawMeta.arranger);
    meta.val.partsCount = rawMeta.partsCount;
    meta.val.creationDate = QDate::fromString(rawMeta.creationDate, "yyyy-MM-dd");

    return meta;
}

io::path MsczMetaReader::readRootFile(MQZipReader* zipReader) const
{
    io::path rootFile;

    QByteArray containerBuffer = zipReader->fileData("META-INF/container.xml");
    if (containerBuffer.isEmpty()) {
        LOGD() << "Can't find container.xml";
        return rootFile;
    }

    framework::XmlReader reader(containerBuffer);

    while (reader.readNextStartElement()) {
        if (reader.tagName() != "container") {
            reader.skipCurrentElement();
            continue;
        }

        while (reader.readNextStartElement()) {
            if (reader.tagName() != "rootfiles") {
                reader.skipCurrentElement();
                continue;
            }

            while (reader.readNextStartElement()) {
                if (reader.tagName() == "rootfile") {
                    if (rootFile.empty()) {
                        rootFile = reader.attribute("full-path");
                        reader.skipCurrentElement();
                    }
                } else {
                    reader.skipCurrentElement();
                }
            }
        }
    }

    return rootFile;
}

QPixmap MsczMetaReader::loadThumbnail(MQZipReader* zipReader) const
{
    QByteArray thumbnailBuffer = zipReader->fileData("Thumbnails/thumbnail.png");

    if (thumbnailBuffer.isEmpty()) {
        LOGD() << "Can't find thumbnail";
        return QPixmap();
    }

    QPixmap thumbnail;
    thumbnail.loadFromData(thumbnailBuffer, "PNG");

    return thumbnail;
}

QString MsczMetaReader::formatFromXml(const std::string& xml) const
{
    size_t bbegin = xml.find("<body");

    if (bbegin != std::string::npos) {
        size_t bend = xml.find("</body>", bbegin);
        std::string body = xml.substr(bbegin, (bend - bbegin));
        return QString::fromStdString(MsczMetaReader::cutXmlTags(body));
    }
    return QString::fromStdString(MsczMetaReader::cutXmlTags(xml));
}

QString MsczMetaReader::format(const std::string& str) const
{
    std::string fin = MsczMetaReader::cutXmlTags(str);
    return simplified(fin);
}

QString MsczMetaReader::simplified(const QString& str) const
{
    return simplified(str.toStdString());
}

QString MsczMetaReader::simplified(const std::string& str) const
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

std::string MsczMetaReader::cutXmlTags(const std::string& str) const
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

QString MsczMetaReader::readText(XmlReader& xmlReader) const
{
    std::string str = xmlReader.readString(framework::XmlReader::IncludeChildElements);
    return formatFromXml(str);
}

QString MsczMetaReader::readMetaTagText(XmlReader& xmlReader) const
{
    return QString::fromStdString(xmlReader.readString());
}
