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

    virtual io::paths userInstrumentListPaths() const = 0;
    virtual void setUserInstrumentListPaths(const io::paths& paths) = 0;

    virtual io::paths scoreOrderListPaths() const = 0;
    virtual async::Notification scoreOrderListPathsChanged() const = 0;

    virtual io::paths userScoreOrderListPaths() const = 0;
    virtual void setUserScoreOrderListPaths(const io::paths& paths) = 0;
};
}

#endif // MU_INSTRUMENTS_IINSTRUMENTSSCONFIGURATION_H
