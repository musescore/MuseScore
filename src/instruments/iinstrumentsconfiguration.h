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
#ifndef MU_INSTRUMENTS_IINSTRUMENTSCONFIGURATION_H
#define MU_INSTRUMENTS_IINSTRUMENTSCONFIGURATION_H

#include "modularity/imoduleexport.h"

#include <vector>

#include "io/path.h"
#include "async/notification.h"

namespace mu::instruments {
class IInstrumentsConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IInstrumentsConfiguration)

public:
    virtual ~IInstrumentsConfiguration() = default;

    virtual io::paths instrumentListPaths() const = 0;
    virtual async::Notification instrumentListPathsChanged() const = 0;

    virtual io::path firstInstrumentListPath() const = 0;
    virtual void setFirstInstrumentListPath(const io::path& path) = 0;

    virtual io::path secondInstrumentListPath() const = 0;
    virtual void setSecondInstrumentListPath(const io::path& path) = 0;

    virtual io::paths scoreOrderListPaths() const = 0;
    virtual async::Notification scoreOrderListPathsChanged() const = 0;

    virtual io::path firstScoreOrderListPath() const = 0;
    virtual void setFirstScireOrderListPath(const io::path& path) = 0;

    virtual io::path secondScoreOrderListPath() const = 0;
    virtual void setSecondScoreOrderListPath(const io::path& path) = 0;
};
}

#endif // MU_INSTRUMENTS_IINSTRUMENTSSCONFIGURATION_H
