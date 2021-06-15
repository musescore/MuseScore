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
#ifndef MU_CONVERTER_BACKENDAPI_H
#define MU_CONVERTER_BACKENDAPI_H

#include "retval.h"
#include "io/path.h"

#include "modularity/ioc.h"
#include "system/ifilesystem.h"
#include "notation/inotationcreator.h"
#include "notation/inotationwritersregister.h"

namespace mu::converter {
class BackendJsonWriter;
class BackendApi
{
    INJECT_STATIC(converter, system::IFileSystem, fileSystem)
    INJECT_STATIC(converter, notation::INotationCreator, notationCreator)
    INJECT_STATIC(converter, notation::INotationWritersRegister, writers)

public:
    static Ret exportScoreMedia(const io::path& in, const io::path& out, const io::path& highlightConfigPath);

private:
    static RetVal<notation::IMasterNotationPtr> openScore(const io::path& path);

    static notation::PageList pages(const notation::INotationPtr notation);

    static QVariantMap readNotesColors(const io::path& filePath);

    static Ret exportScorePngs(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter);
    static Ret exportScoreSvgs(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, const io::path& highlightConfigPath);
    static Ret exportScoreElementsPositions(const std::string& elementsPositionsWriterName, const notation::INotationPtr notation,
                                            BackendJsonWriter& jsonWriter);
    static Ret exportScorePdf(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter);
    static Ret exportScoreMidi(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter);
    static Ret exportScoreMusicXML(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter);
    static Ret exportScoreMetaData(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter);

    static Ret processWriter(const std::string& writerName, const notation::INotationPtr notation, BackendJsonWriter& jsonWriter);
};
}

#endif // MU_CONVERTER_BACKENDAPI_H
