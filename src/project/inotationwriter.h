/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <map>

#include "global/types/ret.h"
#include "global/types/val.h"
#include "global/io/iodevice.h"
#include "global/progress.h"
#include "notation/inotation_fwd.h"

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
        BEATS_COLORS,
        WAIT_FOR_COMPLETION,

        WITH_AUDIO,

        LEADING_SILENCE_SEC,
        TRAILING_SILENCE_SEC,
    };

    using Options = std::map<OptionKey, muse::Val>;

    //! NOTE One destination file for one part/excerpt notation, used by writeParts() below.
    struct PartExportTarget {
        notation::INotationPtr notation;
        muse::io::IODevice* device = nullptr;
    };
    using PartExportTargetList = std::vector<PartExportTarget>;

    virtual std::vector<UnitType> supportedUnitTypes() const = 0;
    virtual bool supportsUnitType(UnitType unitType) const = 0;

    virtual muse::Ret write(notation::INotationPtr notation, muse::io::IODevice& device, const Options& options = Options()) = 0;
    virtual muse::Ret writeList(const notation::INotationPtrList& notations, muse::io::IODevice& device,
                                const Options& options = Options()) = 0;

    //! NOTE Optional capability for writers (currently only audio) that are able to render
    //! several parts/excerpts of masterNotation in a single pass instead of doing a full,
    //! separate write() per notation. When supportsBatchPartExport() is true, writeParts()
    //! must be implemented and produce one file per target, written to target.device, in the
    //! same order as `targets`.
    virtual bool supportsBatchPartExport() const { return false; }
    virtual muse::Ret writeParts(notation::INotationPtr masterNotation, const PartExportTargetList& targets,
                                 const Options& options = Options())
    {
        (void)masterNotation;
        (void)targets;
        (void)options;
        return muse::Ret(muse::Ret::Code::NotSupported);
    }

    virtual muse::Progress* progress() { return nullptr; }
    virtual void abort() {}
};

using INotationWriterPtr = std::shared_ptr<INotationWriter>;
}
