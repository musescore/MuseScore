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

#include <vector>

#include "../iconvertercontroller.h"

#include "modularity/ioc.h"
#include "project/iprojectcreator.h"
#include "project/inotationwritersregister.h"
#include "project/iprojectrwregister.h"
#include "context/iglobalcontext.h"
#include "extensions/iextensionsprovider.h"

#include "types/retval.h"

namespace mu::converter {
class ConverterController : public IConverterController, public muse::Contextable
{
    muse::GlobalInject<project::IProjectCreator> notationCreator;
    muse::GlobalInject<muse::extensions::IExtensionsProvider> extensionsProvider;
    muse::GlobalInject<project::INotationWritersRegister> writers;
    muse::GlobalInject<project::IProjectRWRegister> projectRW;
    muse::ContextInject<context::IGlobalContext> globalContext = { this };

public:
    ConverterController(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    muse::Ret fileConvert(const muse::io::path_t& in, const muse::io::path_t& out, const OpenParams& openParams = {},
                          const muse::String& soundProfile = {}, const muse::io::path_t& tracksDiffPath = {},
                          const muse::UriQuery& extensionUri = {}, const std::string& transposeOptionsJson = {},
                          const std::optional<ConvertTarget>& target = std::nullopt) override;

    muse::Ret batchConvert(const muse::io::path_t& batchJobFile, const OpenParams& openParams = {}, const muse::String& soundProfile = {},
                           const muse::UriQuery& extensionUri = {}, muse::ProgressPtr progress = nullptr) override;

    muse::Ret convertScoreParts(const muse::io::path_t& in, const muse::io::path_t& out, const OpenParams& openParams = {}) override;

    muse::Ret exportScoreMedia(const muse::io::path_t& in, const muse::io::path_t& out,  const OpenParams& openParams = {},
                               const muse::io::path_t& highlightConfigPath = muse::io::path_t()) override;
    muse::Ret exportScoreMeta(const muse::io::path_t& in, const muse::io::path_t& out, const OpenParams& openParams = {}) override;
    muse::Ret exportScoreParts(const muse::io::path_t& in, const muse::io::path_t& out, const OpenParams& openParams = {}) override;
    muse::Ret exportScorePartsPdfs(const muse::io::path_t& in, const muse::io::path_t& out, const OpenParams& openParams = {}) override;
    muse::Ret exportScoreTranspose(const muse::io::path_t& in, const muse::io::path_t& out, const std::string& optionsJson,
                                   const OpenParams& openParams = {}) override;

    muse::Ret exportScoreElements(const muse::io::path_t& in, const muse::io::path_t& out, const OpenParams& openParams = {}) override;

    muse::Ret exportScoreVideo(const muse::io::path_t& in, const muse::io::path_t& out, const OpenParams& openParams = {}) override;

    muse::Ret updateSource(const muse::io::path_t& in, const std::string& newSource, bool forceMode = false) override;

private:
    struct CopyrightInfo {
        CopyrightInfo() {}

        muse::String text;
        bool showOnAllPages = false;
    };

    struct Job {
        muse::io::path_t in;
        muse::io::path_t out;
        muse::io::path_t tracksDiffPath;
        std::optional<notation::TransposeOptions> transposeOptions;
        std::optional<size_t> pageNum;
        std::vector<size_t> visibleParts;
        CopyrightInfo copyright;
    };

    using BatchJob = std::vector<Job>;
    using TransposeOpts = std::optional<notation::TransposeOptions>;

    muse::RetVal<BatchJob> parseBatchJob(const muse::io::path_t& batchJobFile) const;

    muse::Ret convertFile(const muse::io::path_t& in, const muse::io::path_t& out, const OpenParams& openParams = {},
                          const muse::String& soundProfile = {}, const muse::io::path_t& tracksDiffPath = {},
                          const muse::UriQuery& extensionUri = {}, const TransposeOpts& transposeOptions = std::nullopt,
                          const std::optional<ConvertTarget>& target = std::nullopt, const std::vector<size_t>& visibleParts = {},
                          const CopyrightInfo& copyright = {});

    muse::Ret convertScoreParts(project::INotationWriterPtr writer, notation::IMasterNotationPtr masterNotation,
                                const muse::io::path_t& out);

    muse::Ret convertByExtension(project::INotationWriterPtr writer, notation::INotationPtr notation, const muse::io::path_t& out,
                                 const muse::UriQuery& extensionUri);
    bool isConvertPageByPage(const std::string& suffix) const;
    muse::Ret convertPageByPage(project::INotationWriterPtr writer, notation::INotationPtr notation, const muse::io::path_t& out) const;
    muse::Ret convertPage(project::INotationWriterPtr writer, notation::INotationPtr notation, const size_t pageNum,
                          const muse::io::path_t& filePath, const muse::io::path_t& dirPath = {}) const;
    muse::Ret convertFullNotation(project::INotationWriterPtr writer, notation::INotationPtr notation, const muse::io::path_t& out) const;

    muse::Ret convertScorePartsToPdf(project::INotationWriterPtr writer, notation::IMasterNotationPtr masterNotation,
                                     const muse::io::path_t& out) const;
    muse::Ret convertScorePartsToPngs(project::INotationWriterPtr writer, notation::IMasterNotationPtr masterNotation,
                                      const muse::io::path_t& out) const;
    muse::Ret convertScorePartsToMp3(project::INotationWriterPtr writer, notation::IMasterNotationPtr masterNotation,
                                     const muse::io::path_t& out) const;

    muse::Ret saveRegion(project::INotationProjectPtr project, const ConvertRegionJson& regionJson, const muse::io::path_t& out) const;

    muse::Ret writeTracksDiff(project::INotationProjectPtr project, const QJsonArray& oldTracks, const muse::io::path_t& path) const;
};
}
