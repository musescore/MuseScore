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
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_NOTATION_MSCZMETAREADER_H
#define MU_NOTATION_MSCZMETAREADER_H

#include "imsczmetareader.h"

namespace mu::framework {
class XmlReader;
}

class MQZipReader;

namespace mu::notation {
class MsczMetaReader : public IMsczMetaReader
{
public:
    RetVal<Meta> readMeta(const io::path& filePath) const override;

private:
    struct RawMeta {
        QString titleTag;
        QString titleAttribute;
        QString titleStyle;
        QString titleStyleHtml;

        QString subtitleAttribute;
        QString subtitleStyle;
        QString subtitleStyleHtml;

        QString composerAttribute;
        QString composerStyle;
        QString composerStyleHtml;

        QString lyricistAttribute;
        QString lyricistStyle;
        QString lyricistStyleHtml;

        QString copyright;
        QString translator;
        QString arranger;
        QString creationDate;

        size_t partsCount = 0;
    };

    RetVal<Meta> doReadMeta(framework::XmlReader& xmlReader) const;
    RawMeta doReadBox(framework::XmlReader& xmlReader) const;
    RetVal<Meta> loadCompressedMsc(const io::path& filePath) const;
    io::path readRootFile(MQZipReader* zipReader) const;
    QPixmap loadThumbnail(MQZipReader* zipReader) const;
    RawMeta doReadRawMeta(framework::XmlReader& xmlReader) const;
    QString formatFromXml(const std::string& xml) const;

    QString format(const std::string& str) const;
    QString simplified(const QString& str) const;
    QString simplified(const std::string& str) const;
    std::string cutXmlTags(const std::string& str) const;

    QString readText(framework::XmlReader& xmlReader) const;
    QString readMetaTagText(framework::XmlReader& xmlReader) const;
};
}

#endif // MU_NOTATION_MSCZMETAREADER_H
