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

class QXmlStreamReader;
class MQZipReader;

namespace mu {
namespace notation {
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

    RetVal<Meta> doReadMeta(QXmlStreamReader& xmlReader) const;
    RawMeta doReadBox(QXmlStreamReader& xmlReader) const;
    RetVal<Meta> loadCompressedMsc(const mu::io::path& filePath) const;
    QString readRootFile(MQZipReader* zipReader) const;
    QPixmap loadThumbnail(MQZipReader* zipReader) const;
    RawMeta doReadRawMeta(QXmlStreamReader& xmlReader) const;
    QString formatFromXml(const QString& xml) const;

    QString format(const QString& str) const;
    QString simplified(const QString& str) const;
    std::string simplified(const std::string& str) const;
    std::string cutXmlTags(const std::string& str) const;
};
}
}

#endif // MU_NOTATION_MSCZMETAREADER_H
