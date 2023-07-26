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
#ifndef MU_PROJECT_MSCMETAREADER_H
#define MU_PROJECT_MSCMETAREADER_H

#include "imscmetareader.h"

#include "io/ifilesystem.h"
#include "modularity/ioc.h"

namespace mu::framework {
class XmlReader;
}

namespace mu::project {
class MscMetaReader : public IMscMetaReader
{
    INJECT(io::IFileSystem, fileSystem)

public:
    RetVal<ProjectMeta> readMeta(const io::path_t& filePath) const;

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

    void doReadMeta(framework::XmlReader& xmlReader, ProjectMeta& meta) const;
    RawMeta doReadBox(framework::XmlReader& xmlReader) const;
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

#endif // MU_PROJECT_MSCMETAREADER_H
