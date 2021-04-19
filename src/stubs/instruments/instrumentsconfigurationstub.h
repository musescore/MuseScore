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
#ifndef MU_INSTRUMENTS_INSTRUMENTSCONFIGURATIONSTUB_H
#define MU_INSTRUMENTS_INSTRUMENTSCONFIGURATIONSTUB_H

#include "instruments/iinstrumentsconfiguration.h"

namespace mu::instruments {
class InstrumentsConfigurationStub : public IInstrumentsConfiguration
{
public:
    io::paths instrumentListPaths() const override;
    async::Notification instrumentListPathsChanged() const override;

    io::paths userInstrumentListPaths() const override;
    void setUserInstrumentListPaths(const io::paths& paths) override;

    io::paths scoreOrderListPaths() const override;
    async::Notification scoreOrderListPathsChanged() const override;

    io::paths userScoreOrderListPaths() const override;
    void setUserScoreOrderListPaths(const io::paths& paths) override;
};
}

#endif // MU_INSTRUMENTS_INSTRUMENTSSCONFIGURATIONSTUB_H
