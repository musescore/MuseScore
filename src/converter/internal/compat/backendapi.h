/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include <QFile>

#include "types/retval.h"

#include "io/path.h"

#include "modularity/ioc.h"
#include "global/iapplication.h"
#include "io/ifilesystem.h"
#include "project/iprojectcreator.h"
#include "project/iprojectrwregister.h"

#include "converter/convertertypes.h"

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
    inline static muse::ContextInject<project::IProjectRWRegister> projectRW = { nullptr };

public:
    static muse::Ret exportScoreMedia(const muse::io::path_t& in, const muse::io::path_t& out, const muse::io::path_t& highlightConfigPath,
                                      const OpenParams& openParams = {});
    static muse::Ret exportScoreMeta(const muse::io::path_t& in, const muse::io::path_t& out, const OpenParams& openParams = {});
    static muse::Ret exportScoreParts(const muse::io::path_t& in, const muse::io::path_t& out, const OpenParams& openParams = {});
    static muse::Ret exportScorePartsPdfs(const muse::io::path_t& in, const muse::io::path_t& out, const OpenParams& openParams = {});
    static muse::Ret exportScoreTranspose(const muse::io::path_t& in, const muse::io::path_t& out, const std::string& optionsJson,
                                          const OpenParams& openParams = {});

    static muse::Ret exportScoreElements(const muse::io::path_t& in, const muse::io::path_t& out, const OpenParams& openParams = {});

    static muse::Ret updateSource(const muse::io::path_t& in, const std::string& newSource, bool forceMode = false);

private:
    static muse::Ret openOutputFile(QFile& file, const muse::io::path_t& out);

    static muse::RetVal<project::INotationProjectPtr> openProject(const muse::io::path_t& path, const OpenParams& params = {});

    static QVariantMap readBeatsColors(const muse::io::path_t& filePath);

    static muse::Ret exportScorePngs(const project::INotationProjectPtr project, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static muse::Ret exportScoreSvgs(const project::INotationProjectPtr project, const muse::io::path_t& highlightConfigPath,
                                     BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static muse::Ret exportScoreElementsPositions(const project::INotationProjectPtr project,
                                                  const std::string& elementsPositionsWriterName,
                                                  const std::string& elementsPositionsTagName, BackendJsonWriter& jsonWriter,
                                                  bool addSeparator = false);
    static muse::Ret exportScorePdf(const project::INotationProjectPtr project, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static muse::Ret exportScorePdf(const project::INotationProjectPtr project, QIODevice& destinationDevice);
    static muse::Ret exportScoreMidi(const project::INotationProjectPtr project, BackendJsonWriter& jsonWriter, bool addSeparator = false);
    static muse::Ret exportScoreMusicXML(const project::INotationProjectPtr project, BackendJsonWriter& jsonWriter,
                                         bool addSeparator = false);
    static muse::Ret exportScoreMetaData(const project::INotationProjectPtr project, BackendJsonWriter& jsonWriter,
                                         bool addSeparator = false);
    static muse::Ret devInfo(const project::INotationProjectPtr project, BackendJsonWriter& jsonWriter, bool addSeparator = false);

    static muse::RetVal<QByteArray> processWriter(const project::INotationProjectPtr project, const std::string& writerName,
                                                  const muse::IDList& notationsIds = {});

    static muse::Ret doExportScoreParts(const notation::IMasterNotationPtr notation, QIODevice& destinationDevice);
    static muse::Ret doExportScorePartsPdfs(const project::INotationProjectPtr project, const notation::IMasterNotationPtr notation,
                                            QIODevice& destinationDevice, const std::string& scoreFileName);
    static muse::Ret doExportScoreTranspose(const project::INotationProjectPtr project, BackendJsonWriter& jsonWriter,
                                            bool addSeparator = false);

    static muse::Ret doExportScoreElements(const notation::INotationPtr notation, QIODevice& out);

    static muse::RetVal<QByteArray> scorePartJson(mu::engraving::Score* score, const std::string& fileName);

    static void switchToPageView(notation::IMasterNotationPtr masterNotation);
    static void renderExcerptsContents(notation::IMasterNotationPtr masterNotation);

    static notation::ExcerptNotationList allExcerpts(notation::IMasterNotationPtr masterNotation);
    static void initPotentialExcerpts(notation::IMasterNotationPtr masterNotation);
};
}
