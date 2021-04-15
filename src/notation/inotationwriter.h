/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/

#ifndef MU_NOTATION_INOTATIONWRITER_H
#define MU_NOTATION_INOTATIONWRITER_H

#include "ret.h"
#include "val.h"

#include "async/channel.h"
#include "global/progress.h"
#include "system/iodevice.h"
#include "inotation.h"

namespace mu::notation {
class INotationWriter
{
public:
    enum class OptionKey {
        PAGE_NUMBER,
        TRANSPARENT_BACKGROUND,
        TRIM_MARGINS_SIZE,
        NOTES_COLORS
    };

    using Options = QMap<OptionKey, Val>;

    virtual ~INotationWriter() = default;

    virtual Ret write(const INotationPtr notation, system::IODevice& destinationDevice, const Options& options = Options()) = 0;
    virtual void abort() = 0;
    virtual framework::ProgressChannel progress() const = 0;
};

using INotationWriterPtr = std::shared_ptr<INotationWriter>;
}

#endif // MU_NOTATION_INOTATIONWRITER_H
