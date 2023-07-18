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

#ifndef MU_PROJECT_INOTATIONWRITER_H
#define MU_PROJECT_INOTATIONWRITER_H

#include "types/ret.h"
#include "types/val.h"

#include "async/channel.h"
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

    using Options = QMap<OptionKey, Val>;

    virtual std::vector<UnitType> supportedUnitTypes() const = 0;
    virtual bool supportsUnitType(UnitType unitType) const = 0;

    virtual Ret write(notation::INotationPtr notation, QIODevice& device, const Options& options = Options()) = 0;
    virtual Ret writeList(const notation::INotationPtrList& notations, QIODevice& device, const Options& options = Options()) = 0;

    virtual framework::Progress* progress() { return nullptr; }
    virtual void abort() {}
};

using INotationWriterPtr = std::shared_ptr<INotationWriter>;
}

#endif // MU_PROJECT_INOTATIONWRITER_H
