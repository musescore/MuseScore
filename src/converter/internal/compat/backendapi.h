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
#include "io/device.h"

#include "modularity/ioc.h"
#include "system/ifilesystem.h"
#include "project/iprojectcreator.h"
#include "project/inotationwritersregister.h"

namespace Ms {
class Score;
}

namespace mu::converter {
class BackendJsonWriter;
class BackendApi
{
    INJECT_STATIC(converter, system::IFileSystem, fileSystem)
    INJECT_STATIC(converter, project::IProjectCreator, notationCreator)
    INJECT_STATIC(converter, project::INotationWritersRegister, writers)

public:
    static Ret exportScoreMedia(const io::path& in, const io::path& out, const io::path& highlightConfigPath,
                                const io::path& stylePath = "", bool forceMode = false);
    static Ret exportScoreMeta(const io::path& in, const io::path& out, const io::path& stylePath, bool forceMode = false);
    static Ret exportScoreParts(const io::path& in, const io::path& out, const io::path& stylePath, bool forceMode = false);
    static Ret exportScorePartsPdfs(const io::path& in, const io::path& out, const io::path& stylePath, bool forceMode = false);
    static Ret exportScoreTranspose(const io::path& in, const io::path& out, const std::string& optionsJson, const io::path& stylePath,
                                    bool forceMode = false);

    static Ret updateSource(const io::path& in, const std::string& newSource, bool forceMode = false);

private:
    static Ret openOutputFile(QFile& file, const io::path& out);

    static RetVal<project::INotationProjectPtr> openProject(const io::path& path,
                                                            const io::path& stylePath = io::path(), bool forceMode = false);

    static notation::PageList pages(const notation::INotationPtr notation);

    static QVariantMap readNotesColors(const io::path& filePath);

    static Ret exportScorePngs(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static Ret exportScoreSvgs(const notation::INotationPtr notation, const io::path& highlightConfigPath, BackendJsonWriter& jsonWriter,
                               bool addSeparator = false);
    static Ret exportScoreElementsPositions(const std::string& elementsPositionsWriterName, const notation::INotationPtr notation,
                                            BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static Ret exportScorePdf(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static Ret exportScorePdf(const notation::INotationPtr notation, mu::io::Device& destinationDevice);
    static Ret exportScoreMidi(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static Ret exportScoreMusicXML(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static Ret exportScoreMetaData(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);

    static mu::RetVal<QByteArray> processWriter(const std::string& writerName, const notation::INotationPtr notation);
    static mu::RetVal<QByteArray> processWriter(const std::string& writerName, const notation::INotationPtrList notations,
                                                const project::INotationWriter::Options& options);

    static Ret doExportScoreParts(const notation::INotationPtr notation, mu::io::Device& destinationDevice);
    static Ret doExportScorePartsPdfs(const notation::IMasterNotationPtr notation, mu::io::Device& destinationDevice,
                                      const std::string& scoreFileName);
    static Ret doExportScoreTranspose(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);

    static RetVal<QByteArray> scorePartJson(Ms::Score* score, const std::string& fileName);

    static RetVal<notation::TransposeOptions> parseTransposeOptions(const std::string& optionsJson);
    static Ret applyTranspose(const notation::INotationPtr notation, const std::string& optionsJson);
};
}

#endif // MU_CONVERTER_BACKENDAPI_H
