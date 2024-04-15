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

#ifndef MU_PROJECT_INOTATIONWRITER_H
#define MU_PROJECT_INOTATIONWRITER_H

#include <map>

#include "global/types/ret.h"
#include "global/types/val.h"
#include "global/io/iodevice.h"
#include "global/async/channel.h"
#include "global/progress.h"
#include "notation/inotation.h"

namespace mu::project {
class INotationWriter
{
public:

    virtual ~INotationWriter() = default;

    enum class UnitType {
        PER_PAGE,
        PER_PART,
        MULTI_PART
    };

    enum class OptionKey {
        UNIT_TYPE,
        PAGE_NUMBER,
        TRANSPARENT_BACKGROUND,
        BEATS_COLORS
    };

    using Options = std::map<OptionKey, muse::Val>;

    virtual std::vector<UnitType> supportedUnitTypes() const = 0;
    virtual bool supportsUnitType(UnitType unitType) const = 0;

    virtual muse::Ret write(notation::INotationPtr notation, muse::io::IODevice& device, const Options& options = Options()) = 0;
    virtual muse::Ret writeList(const notation::INotationPtrList& notations, muse::io::IODevice& device,
                                const Options& options = Options()) = 0;

    virtual muse::Progress* progress() { return nullptr; }
    virtual void abort() {}
};

using INotationWriterPtr = std::shared_ptr<INotationWriter>;
}

#endif // MU_PROJECT_INOTATIONWRITER_H
