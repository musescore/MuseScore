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
#include "notation/inotationcreator.h"
#include "notation/inotationwritersregister.h"

#include "retval.h"

namespace mu::converter {
class ConverterController : public IConverterController
{
    INJECT(converter, notation::INotationCreator, notationCreator)
    INJECT(converter, notation::INotationWritersRegister, writers)

public:
    ConverterController() = default;

    Ret fileConvert(const io::path& in, const io::path& out, const io::path& stylePath) override;
    Ret batchConvert(const io::path& batchJobFile, const io::path& stylePath) override;
    Ret exportScoreMedia(const io::path& in, const io::path& out, const io::path& highlightConfigPath, const io::path& stylePath) override;
    Ret exportScoreMeta(const io::path& in, const io::path& out, const io::path& stylePath) override;
    Ret exportScoreParts(const io::path& in, const io::path& out, const io::path& stylePath) override;
    Ret exportScorePartsPdfs(const io::path& in, const io::path& out, const io::path& stylePath) override;
    Ret updateSource(const io::path& in, const QString& newSource) override;

private:

    struct Job {
        io::path in;
        io::path out;
    };

    using BatchJob = std::list<Job>;

    RetVal<BatchJob> parseBatchJob(const io::path& batchJobFile) const;
};
}

#endif // MU_CONVERTER_CONVERTERCONTROLLER_H
