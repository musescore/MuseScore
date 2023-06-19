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
#ifndef MU_NOTATION_INSTRUMENTSREPOSITORY_H
#define MU_NOTATION_INSTRUMENTSREPOSITORY_H

#include "modularity/ioc.h"

#include "async/channel.h"
#include "async/asyncable.h"

#include "iinstrumentsrepository.h"
#include "inotationconfiguration.h"

namespace mu::notation {
class InstrumentsRepository : public IInstrumentsRepository, public async::Asyncable
{
    INJECT(INotationConfiguration, configuration)

public:
    void init();

    const InstrumentTemplateList& instrumentTemplates() const override;
    const InstrumentTemplate& instrumentTemplate(const std::string& instrumentId) const override;

    const ScoreOrderList& orders() const override;
    const ScoreOrder& order(const std::string& orderId) const override;

    const InstrumentGenreList& genres() const override;
    const InstrumentGroupList& groups() const override;

private:
    void load();
    void clear();

    InstrumentTemplateList m_instrumentTemplates;
    InstrumentGroupList m_groups;
    InstrumentGenreList m_genres;
};
}

#endif // MU_NOTATION_INSTRUMENTSREPOSITORY_H
