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
#include "instrumentsconfigurationstub.h"

using namespace mu::instruments;

mu::io::paths InstrumentsConfigurationStub::instrumentListPaths() const
{
    return {};
}

mu::async::Notification InstrumentsConfigurationStub::instrumentListPathsChanged() const
{
    return mu::async::Notification();
}

mu::io::paths InstrumentsConfigurationStub::userInstrumentListPaths() const
{
    return {};
}

void InstrumentsConfigurationStub::setUserInstrumentListPaths(const io::paths& /*paths*/)
{
}

mu::io::paths InstrumentsConfigurationStub::scoreOrderListPaths() const
{
    return {};
}

mu::async::Notification InstrumentsConfigurationStub::scoreOrderListPathsChanged() const
{
    return mu::async::Notification();
}

mu::io::paths InstrumentsConfigurationStub::userScoreOrderListPaths() const
{
    return {};
}

void InstrumentsConfigurationStub::setUserScoreOrderListPaths(const io::paths& /*paths*/)
{
}
