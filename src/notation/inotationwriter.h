//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

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

    virtual Ret write(const INotationPtr notation, framework::IODevice& destinationDevice, const Options& options = Options()) = 0;
    virtual void abort() = 0;
    virtual framework::ProgressChannel progress() const = 0;
};

using INotationWriterPtr = std::shared_ptr<INotationWriter>;
}

#endif // MU_NOTATION_INOTATIONWRITER_H
