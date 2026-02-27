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

#ifndef MU_PROJECT_IPROJECTWRITER_H
#define MU_PROJECT_IPROJECTWRITER_H

#include "types/ret.h"

#include "async/channel.h"
#include "global/io/iodevice.h"
#include "global/progress.h"
#include "inotationproject.h"
#include "types/projecttypes.h"

namespace mu::project {
class IProjectWriter
{
public:

    virtual ~IProjectWriter() = default;

    virtual std::vector<WriteUnitType> supportedUnitTypes() const = 0;
    virtual bool supportsUnitType(WriteUnitType unitType) const = 0;

    virtual muse::Ret write(project::INotationProjectPtr project, muse::io::IODevice& device,
                            const WriteOptions& options = WriteOptions()) = 0;
    virtual muse::Ret write(project::INotationProjectPtr project, const muse::io::path_t& filePath,
                            const WriteOptions& options = WriteOptions()) = 0;

    virtual muse::Progress* progress() { return nullptr; }
    virtual void abort() {}
};

using IProjectWriterPtr = std::shared_ptr<IProjectWriter>;
}

#endif // MU_PROJECT_IPROJECTWRITER_H
