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

#pragma once

#include "imscmetareader.h"

#include "io/ifilesystem.h"
#include "modularity/ioc.h"

namespace muse {
class XmlStreamReader;
}

namespace mu::project {
class MscMetaReader : public IMscMetaReader
{
    INJECT(muse::io::IFileSystem, fileSystem)

public:
    muse::RetVal<ProjectMeta> readMeta(const muse::io::path_t& filePath) const override;
    muse::RetVal<CloudProjectInfo> readCloudProjectInfo(const muse::io::path_t& filePath) const override;

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

        QVariantMap additionalTags;

        size_t partsCount = 0;
    };

    muse::Ret prepareReader(const muse::io::path_t& filePath, mu::engraving::MscReader& reader) const;

    void doReadMeta(muse::XmlStreamReader& xmlReader, ProjectMeta& meta) const;
    RawMeta doReadBox(muse::XmlStreamReader& xmlReader) const;
    RawMeta doReadRawMeta(muse::XmlStreamReader& xmlReader) const;
    QString formatFromXml(const std::string& xml) const;

    QString format(const std::string& str) const;
    QString simplified(const QString& str) const;
    QString simplified(const std::string& str) const;
    std::string cutXmlTags(const std::string& str) const;

    QString readText(muse::XmlStreamReader& xmlReader) const;
    QString readMetaTagText(muse::XmlStreamReader& xmlReader) const;
};
}
