/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "iinstrumentsrepository.h"
#include "async/asyncable.h"

#include "io/ifilesystem.h"
#include "inotationconfiguration.h"
#include "framework/musesampler/imusesamplerinfo.h"

namespace mu::notation {
class InstrumentsRepository : public IInstrumentsRepository, public muse::async::Asyncable
{
    Inject<muse::io::IFileSystem> fileSystem;
    Inject<INotationConfiguration> configuration;
    Inject<muse::musesampler::IMuseSamplerInfo> museSampler;

public:
    void init();

    const InstrumentTemplateList& instrumentTemplates() const override;
    const InstrumentTemplate& instrumentTemplate(const muse::String& instrumentId) const override;

    const ScoreOrderList& orders() const override;
    const ScoreOrder& order(const muse::String& orderId) const override;

    const InstrumentGenreList& genres() const override;
    const InstrumentGroupList& groups() const override;

    const InstrumentStringTuningsMap& stringTuningsPresets() const override;

private:
    using InstrumentTemplateMap = std::unordered_map<muse::String, const InstrumentTemplate*>;

    void load();
    void clear();

    bool loadStringTuningsPresets(const muse::io::path_t& path);
    void loadMuseInstruments(const InstrumentTemplateMap& standardInstrumentByMusicXmlId);

    InstrumentTemplateList m_instrumentTemplateList;
    InstrumentTemplateMap m_instrumentTemplateMap;
    InstrumentStringTuningsMap m_stringTuningsPresets;
};
}

#endif // MU_NOTATION_INSTRUMENTSREPOSITORY_H
