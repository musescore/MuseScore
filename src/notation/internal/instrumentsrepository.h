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

#include "io/ifilesystem.h"
#include "iinstrumentsrepository.h"
#include "inotationconfiguration.h"

namespace mu::notation {
class InstrumentsRepository : public IInstrumentsRepository, public async::Asyncable
{
    INJECT(io::IFileSystem, fileSystem)
    INJECT(INotationConfiguration, configuration)

public:
    void init();

    const InstrumentTemplateList& instrumentTemplates() const override;
    const InstrumentTemplate& instrumentTemplate(const std::string& instrumentId) const override;

    const ScoreOrderList& orders() const override;
    const ScoreOrder& order(const std::string& orderId) const override;

    const InstrumentGenreList& genres() const override;
    const InstrumentGroupList& groups() const override;

    const InstrumentStringTuningsMap& stringTuningsPresets() const override;

private:
    void load();
    void clear();

    bool loadStringTuningsPresets(const io::path_t& path);

    InstrumentTemplateList m_instrumentTemplates;
    InstrumentGroupList m_groups;
    InstrumentGenreList m_genres;
    InstrumentStringTuningsMap m_stringTuningsPresets;
};
}

#endif // MU_NOTATION_INSTRUMENTSREPOSITORY_H
