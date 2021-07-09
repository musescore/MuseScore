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
#ifndef MU_NOTATION_MSCZMETAREADER_H
#define MU_NOTATION_MSCZMETAREADER_H

#include "imsczmetareader.h"

#include "system/ifilesystem.h"
#include "modularity/ioc.h"

namespace mu::framework {
class XmlReader;
}

class MQZipReader;

namespace mu::notation {
class MsczMetaReader : public IMsczMetaReader
{
    INJECT(notation, system::IFileSystem, fileSystem)

public:
    MetaList readMetaList(const io::paths& filePaths) const override;

private:
    RetVal<Meta> readMeta(const io::path& filePath) const;

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
    RetVal<Meta> readMetaFromMscx(const io::path& filePath) const;
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
