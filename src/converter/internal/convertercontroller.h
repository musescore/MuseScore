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
#ifndef MU_CONVERTER_CONVERTERCONTROLLER_H
#define MU_CONVERTER_CONVERTERCONTROLLER_H

#include <list>

#include "../iconvertercontroller.h"

#include "modularity/ioc.h"
#include "project/iprojectcreator.h"
#include "project/inotationwritersregister.h"
#include "project/iprojectrwregister.h"
#include "context/iglobalcontext.h"

#include "types/retval.h"

namespace mu::converter {
class ConverterController : public IConverterController
{
    INJECT(project::IProjectCreator, notationCreator)
    INJECT(project::INotationWritersRegister, writers)
    INJECT(project::IProjectRWRegister, projectRW)
    INJECT(context::IGlobalContext, globalContext)

public:
    ConverterController() = default;

    Ret fileConvert(const io::path_t& in, const io::path_t& out, const io::path_t& stylePath = io::path_t(),
                    bool forceMode = false) override;
    Ret batchConvert(const io::path_t& batchJobFile, const io::path_t& stylePath = io::path_t(), bool forceMode = false) override;
    Ret convertScoreParts(const io::path_t& in, const io::path_t& out, const io::path_t& stylePath = io::path_t(),
                          bool forceMode = false) override;

    Ret exportScoreMedia(const io::path_t& in, const io::path_t& out,
                         const io::path_t& highlightConfigPath = io::path_t(), const io::path_t& stylePath = io::path_t(),
                         bool forceMode = false) override;
    Ret exportScoreMeta(const io::path_t& in, const io::path_t& out, const io::path_t& stylePath = io::path_t(),
                        bool forceMode = false) override;
    Ret exportScoreParts(const io::path_t& in, const io::path_t& out, const io::path_t& stylePath = io::path_t(),
                         bool forceMode = false) override;
    Ret exportScorePartsPdfs(const io::path_t& in, const io::path_t& out,
                             const io::path_t& stylePath = io::path_t(), bool forceMode = false) override;
    Ret exportScoreTranspose(const io::path_t& in, const io::path_t& out, const std::string& optionsJson,
                             const io::path_t& stylePath = io::path_t(), bool forceMode = false) override;

    Ret exportScoreVideo(const io::path_t& in, const io::path_t& out) override;

    Ret updateSource(const io::path_t& in, const std::string& newSource, bool forceMode = false) override;

private:

    struct Job {
        io::path_t in;
        io::path_t out;
    };

    using BatchJob = std::list<Job>;

    RetVal<BatchJob> parseBatchJob(const io::path_t& batchJobFile) const;

    bool isConvertPageByPage(const std::string& suffix) const;
    Ret convertPageByPage(project::INotationWriterPtr writer, notation::INotationPtr notation, const io::path_t& out) const;
    Ret convertFullNotation(project::INotationWriterPtr writer, notation::INotationPtr notation, const io::path_t& out) const;

    Ret convertScorePartsToPdf(project::INotationWriterPtr writer, notation::IMasterNotationPtr masterNotation,
                               const io::path_t& out) const;
    Ret convertScorePartsToPngs(project::INotationWriterPtr writer, notation::IMasterNotationPtr masterNotation,
                                const io::path_t& out) const;
};
}

#endif // MU_CONVERTER_CONVERTERCONTROLLER_H
