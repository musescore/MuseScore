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

#include "types/retval.h"

#include "io/path.h"

#include "modularity/ioc.h"
#include "io/ifilesystem.h"
#include "project/iprojectcreator.h"
#include "project/inotationwritersregister.h"

namespace mu::engraving {
class Score;
}

namespace mu::converter {
class BackendJsonWriter;
class BackendApi
{
    INJECT_STATIC(io::IFileSystem, fileSystem)
    INJECT_STATIC(project::IProjectCreator, notationCreator)
    INJECT_STATIC(project::INotationWritersRegister, writers)

public:
    static Ret exportScoreMedia(const io::path_t& in, const io::path_t& out, const io::path_t& highlightConfigPath,
                                const io::path_t& stylePath = "", bool forceMode = false);
    static Ret exportScoreMeta(const io::path_t& in, const io::path_t& out, const io::path_t& stylePath, bool forceMode = false);
    static Ret exportScoreParts(const io::path_t& in, const io::path_t& out, const io::path_t& stylePath, bool forceMode = false);
    static Ret exportScorePartsPdfs(const io::path_t& in, const io::path_t& out, const io::path_t& stylePath, bool forceMode = false);
    static Ret exportScoreTranspose(const io::path_t& in, const io::path_t& out, const std::string& optionsJson,
                                    const io::path_t& stylePath, bool forceMode = false);

    static Ret updateSource(const io::path_t& in, const std::string& newSource, bool forceMode = false);

private:
    static Ret openOutputFile(QFile& file, const io::path_t& out);

    static RetVal<project::INotationProjectPtr> openProject(const io::path_t& path,
                                                            const io::path_t& stylePath = io::path_t(), bool forceMode = false);

    static notation::PageList pages(const notation::INotationPtr notation);

    static QVariantMap readBeatsColors(const io::path_t& filePath);

    static Ret exportScorePngs(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static Ret exportScoreSvgs(const notation::INotationPtr notation, const io::path_t& highlightConfigPath, BackendJsonWriter& jsonWriter,
                               bool addSeparator = false);
    static Ret exportScoreElementsPositions(const std::string& elementsPositionsWriterName, const std::string& elementsPositionsTagName,
                                            const notation::INotationPtr notation, BackendJsonWriter& jsonWriter,
                                            bool addSeparator = false);
    static Ret exportScorePdf(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static Ret exportScorePdf(const notation::INotationPtr notation, QIODevice& destinationDevice);
    static Ret exportScoreMidi(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static Ret exportScoreMusicXML(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static Ret exportScoreMetaData(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static Ret devInfo(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);

    static mu::RetVal<QByteArray> processWriter(const std::string& writerName, const notation::INotationPtr notation);
    static mu::RetVal<QByteArray> processWriter(const std::string& writerName, const notation::INotationPtrList notations,
                                                const project::INotationWriter::Options& options);

    static Ret doExportScoreParts(const notation::INotationPtr notation, QIODevice& destinationDevice);
    static Ret doExportScorePartsPdfs(const notation::IMasterNotationPtr notation, QIODevice& destinationDevice,
                                      const std::string& scoreFileName);
    static Ret doExportScoreTranspose(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);

    static RetVal<QByteArray> scorePartJson(mu::engraving::Score* score, const std::string& fileName);

    static RetVal<notation::TransposeOptions> parseTransposeOptions(const std::string& optionsJson);
    static Ret applyTranspose(const notation::INotationPtr notation, const std::string& optionsJson);
};
}

#endif // MU_CONVERTER_BACKENDAPI_H
