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

#include "retval.h"

namespace mu::converter {
class ConverterController : public IConverterController
{
    INJECT(converter, project::IProjectCreator, notationCreator)
    INJECT(converter, project::INotationWritersRegister, writers)

public:
    ConverterController() = default;

    Ret fileConvert(const io::path& in, const io::path& out, const io::path& stylePath = io::path(), bool forceMode = false) override;
    Ret batchConvert(const io::path& batchJobFile, const io::path& stylePath = io::path(), bool forceMode = false) override;
    Ret convertScoreParts(const io::path& in, const io::path& out, const io::path& stylePath = io::path(), bool forceMode = false) override;

    Ret exportScoreMedia(const io::path& in, const io::path& out,
                         const io::path& highlightConfigPath = io::path(), const io::path& stylePath = io::path(),
                         bool forceMode = false) override;
    Ret exportScoreMeta(const io::path& in, const io::path& out, const io::path& stylePath = io::path(), bool forceMode = false) override;
    Ret exportScoreParts(const io::path& in, const io::path& out, const io::path& stylePath = io::path(), bool forceMode = false) override;
    Ret exportScorePartsPdfs(const io::path& in, const io::path& out, const io::path& stylePath = io::path(),
                             bool forceMode = false) override;
    Ret exportScoreTranspose(const io::path& in, const io::path& out, const std::string& optionsJson,
                             const io::path& stylePath = io::path(), bool forceMode = false) override;

    Ret updateSource(const io::path& in, const std::string& newSource, bool forceMode = false) override;

private:

    struct Job {
        io::path in;
        io::path out;
    };

    using BatchJob = std::list<Job>;

    RetVal<BatchJob> parseBatchJob(const io::path& batchJobFile) const;

    bool isConvertPageByPage(const std::string& suffix) const;
    Ret convertPageByPage(project::INotationWriterPtr writer, notation::INotationPtr notation, const io::path& out) const;
    Ret convertFullNotation(project::INotationWriterPtr writer, notation::INotationPtr notation, const io::path& out) const;

    Ret convertScorePartsToPdf(project::INotationWriterPtr writer, notation::IMasterNotationPtr masterNotation, const io::path& out) const;
    Ret convertScorePartsToPngs(project::INotationWriterPtr writer, notation::IMasterNotationPtr masterNotation, const io::path& out) const;
};
}

#endif // MU_CONVERTER_CONVERTERCONTROLLER_H
