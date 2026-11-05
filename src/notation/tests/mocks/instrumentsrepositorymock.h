/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <gmock/gmock.h>

#include "notation/iinstrumentsrepository.h"

namespace mu::notation {
class InstrumentsRepositoryMock : public IInstrumentsRepository
{
public:
    MOCK_METHOD(const InstrumentTemplateList&, instrumentTemplates, (), (const, override));
    MOCK_METHOD(const InstrumentTemplate&, instrumentTemplate, (const muse::String&), (const, override));

    MOCK_METHOD(const ScoreOrderList&, orders, (), (const, override));
    MOCK_METHOD(const ScoreOrder&, order, (const muse::String&), (const, override));

    MOCK_METHOD(const InstrumentGenreList&, genres, (), (const, override));
    MOCK_METHOD(const InstrumentGroupList&, groups, (), (const, override));

    MOCK_METHOD(const InstrumentStringTuningsMap&, stringTuningsPresets, (), (const, override));
};
}
