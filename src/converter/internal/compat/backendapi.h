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
#ifndef MU_CONVERTER_BACKENDAPI_H
#define MU_CONVERTER_BACKENDAPI_H

#include <QFile>

#include "types/retval.h"

#include "io/path.h"

#include "modularity/ioc.h"
#include "global/iapplication.h"
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
    inline static muse::GlobalInject<muse::io::IFileSystem> fileSystem;
    inline static muse::GlobalInject<muse::IApplication> application;
    inline static muse::GlobalInject<project::IProjectCreator> notationCreator;
    inline static muse::GlobalInject<project::INotationWritersRegister> writers;

public:
    static muse::Ret exportScoreMedia(const muse::io::path_t& in, const muse::io::path_t& out, const muse::io::path_t& highlightConfigPath,
                                      const muse::io::path_t& stylePath = "", bool forceMode = false);
    static muse::Ret exportScoreMeta(const muse::io::path_t& in, const muse::io::path_t& out, const muse::io::path_t& stylePath,
                                     bool forceMode = false);
    static muse::Ret exportScoreParts(const muse::io::path_t& in, const muse::io::path_t& out, const muse::io::path_t& stylePath,
                                      bool forceMode = false);
    static muse::Ret exportScorePartsPdfs(const muse::io::path_t& in, const muse::io::path_t& out, const muse::io::path_t& stylePath,
                                          bool forceMode = false);
    static muse::Ret exportScoreTranspose(const muse::io::path_t& in, const muse::io::path_t& out, const std::string& optionsJson,
                                          const muse::io::path_t& stylePath, bool forceMode = false);

    static muse::Ret updateSource(const muse::io::path_t& in, const std::string& newSource, bool forceMode = false);

private:
    static muse::Ret openOutputFile(QFile& file, const muse::io::path_t& out);

    static muse::RetVal<project::INotationProjectPtr> openProject(const muse::io::path_t& path,
                                                                  const muse::io::path_t& stylePath = muse::io::path_t(), bool forceMode = false);

    static notation::PageList pages(const notation::INotationPtr notation);

    static QVariantMap readBeatsColors(const muse::io::path_t& filePath);

    static muse::Ret exportScorePngs(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static muse::Ret exportScoreSvgs(const notation::INotationPtr notation, const muse::io::path_t& highlightConfigPath,
                                     BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static muse::Ret exportScoreElementsPositions(const std::string& elementsPositionsWriterName,
                                                  const std::string& elementsPositionsTagName, const notation::INotationPtr notation,
                                                  BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static muse::Ret exportScorePdf(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static muse::Ret exportScorePdf(const notation::INotationPtr notation, QIODevice& destinationDevice);
    static muse::Ret exportScoreMidi(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static muse::Ret exportScoreMusicXML(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static muse::Ret exportScoreMetaData(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static muse::Ret devInfo(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator = false);

    static muse::RetVal<QByteArray> processWriter(const std::string& writerName, const notation::INotationPtr notation);
    static muse::RetVal<QByteArray> processWriter(const std::string& writerName, const notation::INotationPtrList notations,
                                                  const project::INotationWriter::Options& options);

    static muse::Ret doExportScoreParts(const notation::IMasterNotationPtr notation, QIODevice& destinationDevice);
    static muse::Ret doExportScorePartsPdfs(const notation::IMasterNotationPtr notation, QIODevice& destinationDevice,
                                            const std::string& scoreFileName);
    static muse::Ret doExportScoreTranspose(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter,
                                            bool addSeparator = false);

    static muse::RetVal<QByteArray> scorePartJson(mu::engraving::Score* score, const std::string& fileName);

    static void switchToPageView(notation::IMasterNotationPtr masterNotation);
    static void renderExcerptsContents(notation::IMasterNotationPtr masterNotation);

    static notation::ExcerptNotationList allExcerpts(notation::IMasterNotationPtr masterNotation);
    static void initPotentialExcerpts(notation::IMasterNotationPtr masterNotation);
};
}

#endif // MU_CONVERTER_BACKENDAPI_H
