#include "msczmetareader.h"

#include <sstream>

#include <QXmlStreamReader>
#include <QFileInfo>
#include <QBuffer>

#include "log.h"
#include "stringutils.h"
#include "notationerrors.h"

#include "thirdparty/qzip/qzipreader_p.h"

using namespace mu;
using namespace mu::notation;

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
        QFile file(fileInfo.filePath());
        if (!file.open(QFile::ReadOnly)) {
            LOGE() << "Failed open file:" << filePath << file.errorString();
            meta.ret = make_ret(Err::FileOpenError);
            return meta;
        }

        QXmlStreamReader reader(file.readAll());
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

    QString rootfile = readRootFile(&zipReader);
    if (rootfile.isEmpty()) {
        meta.ret = make_ret(Err::FileNoRootFile);
        return meta;
    }

    QByteArray rootBuffer = zipReader.fileData(rootfile);
    if (rootBuffer.isEmpty()) {
        auto fil = zipReader.fileInfoList();
        for (const MQZipReader::FileInfo& fi : fil) {
            if (mu::strings::endsWith(fi.filePath.toStdString(), ".mscx")) {
                rootBuffer = zipReader.fileData(fi.filePath);
                break;
            }
        }
    }

    QXmlStreamReader xmlReader(rootBuffer);
    meta = doReadMeta(xmlReader);

    meta.val.thumbnail = loadThumbnail(&zipReader);

    return meta;
}

MsczMetaReader::RawMeta MsczMetaReader::doReadBox(QXmlStreamReader& xmlReader) const
{
    RawMeta meta;

    while (xmlReader.readNextStartElement()) {
        if (xmlReader.name() == "Text") {
            bool isTitle = false;
            bool isSubtitle = false;
            bool isComposer = false;
            bool isLiricist = false;
            while (xmlReader.readNextStartElement()) {
                const QStringRef& tag(xmlReader.name());

                if (tag == "style") {
                    QString val(xmlReader.readElementText().toLower());

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
                        meta.titleStyle = formatFromXml(xmlReader.readElementText(
                                                            QXmlStreamReader::IncludeChildElements));
                    }
                    if (isSubtitle) {
                        meta.subtitleStyle = formatFromXml(xmlReader.readElementText(
                                                               QXmlStreamReader::IncludeChildElements));
                    } else if (isComposer) {
                        meta.composerStyle
                            = formatFromXml(xmlReader.readElementText(QXmlStreamReader::IncludeChildElements));
                    } else if (isLiricist) {
                        meta.lyricistStyle
                            = formatFromXml(xmlReader.readElementText(QXmlStreamReader::IncludeChildElements));
                    } else {
                        xmlReader.skipCurrentElement();
                    }
                } else if (tag == "html-data") {
                    if (isTitle) {
                        meta.titleStyleHtml
                            = formatFromXml(xmlReader.readElementText(QXmlStreamReader::IncludeChildElements));
                    } else if (isSubtitle) {
                        meta.subtitleStyleHtml
                            = formatFromXml(xmlReader.readElementText(QXmlStreamReader::IncludeChildElements));
                    } else if (isComposer) {
                        meta.composerStyleHtml
                            = formatFromXml(xmlReader.readElementText(QXmlStreamReader::IncludeChildElements));
                    } else if (isLiricist) {
                        meta.lyricistStyleHtml
                            = formatFromXml(xmlReader.readElementText(QXmlStreamReader::IncludeChildElements));
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

MsczMetaReader::RawMeta MsczMetaReader::doReadRawMeta(QXmlStreamReader& xmlReader) const
{
    RawMeta meta;

    while (xmlReader.readNextStartElement()) {
        const QStringRef& tag(xmlReader.name());
        if (tag == "work-title") {
            meta.titleTag = xmlReader.readElementText();
        } else if (tag == "metaTag") {
            QString name = xmlReader.attributes().value("name").toString();
            if (name == "workTitle") {
                meta.titleAttribute = xmlReader.readElementText();
            } else if (name == "composer") {
                meta.composerAttribute = xmlReader.readElementText();
            } else if (name == "arranger") {
                meta.arranger = xmlReader.readElementText();
            } else if (name == "lyricist") {
                meta.lyricistAttribute = xmlReader.readElementText();
            } else if (name == "copyright") {
                meta.copyright = xmlReader.readElementText();
            } else if (name == "translator") {
                meta.translator = xmlReader.readElementText();
            } else if (name == "creationDate") {
                meta.creationDate = xmlReader.readElementText();
            } else {
                xmlReader.skipCurrentElement();
            }
        } else if (tag == "Staff") {
            if (meta.titleStyle.isEmpty()) {
                while (xmlReader.readNextStartElement()) {
                    const QStringRef& tag(xmlReader.name());
                    if (tag == "HBox"
                        || tag == "VBox"
                        || tag == "TBox"
                        || tag == "FBox") {
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

RetVal<Meta> MsczMetaReader::doReadMeta(QXmlStreamReader& xmlReader) const
{
    RawMeta rawMeta;

    while (xmlReader.readNextStartElement()) {
        if (xmlReader.name() == "museScore") {
            const QString& version = xmlReader.attributes().value("version").toString();

            if (version.startsWith("1")) {
                rawMeta = doReadRawMeta(xmlReader);
            } else {
                while (xmlReader.readNextStartElement()) {
                    if (xmlReader.name() == "Score") {
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

QString MsczMetaReader::readRootFile(MQZipReader* zipReader) const
{
    QString rootfile;

    QByteArray containerBuffer = zipReader->fileData("META-INF/container.xml");
    if (containerBuffer.isEmpty()) {
        LOGD() << "Can't find container.xml";
        return rootfile;
    }

    QXmlStreamReader reader(containerBuffer);

    while (reader.readNextStartElement()) {
        if (reader.name() != "container") {
            reader.skipCurrentElement();
            continue;
        }

        while (reader.readNextStartElement()) {
            if (reader.name() != "rootfiles") {
                reader.skipCurrentElement();
                continue;
            }

            while (reader.readNextStartElement()) {
                const QStringRef& tag(reader.name());

                if (tag == "rootfile") {
                    if (rootfile.isEmpty()) {
                        rootfile = reader.attributes().value("full-path").toString();
                        reader.skipCurrentElement();
                    }
                } else {
                    reader.skipCurrentElement();
                }
            }
        }
    }

    return rootfile;
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

QString MsczMetaReader::formatFromXml(const QString& xml) const
{
    std::string c_xml = xml.toStdString();
    size_t bbegin = c_xml.find("<body");

    if (bbegin != std::string::npos) {
        size_t bend = c_xml.find("</body>", bbegin);
        std::string body = c_xml.substr(bbegin, (bend - bbegin));
        return QString::fromStdString(MsczMetaReader::cutXmlTags(body));
    }
    return QString::fromStdString(MsczMetaReader::cutXmlTags(c_xml));
}

QString MsczMetaReader::format(const QString& str) const
{
    std::string fin = MsczMetaReader::cutXmlTags(str.toStdString());
    return QString::fromStdString(MsczMetaReader::simplified(fin));
}

QString MsczMetaReader::simplified(const QString& str) const
{
    return QString::fromStdString(simplified(str.toStdString()));
}

std::string MsczMetaReader::simplified(const std::string& str) const
{
    std::string fin;
    for (size_t index = 0; str[index]; ++index) {
        if (str[index] == '\"' || str[index] == '\n') {
            fin += '\\';
        }
        fin += str[index];
    }

    return fin;
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
