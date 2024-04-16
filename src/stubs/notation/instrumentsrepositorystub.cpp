/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "instrumentsrepositorystub.h"

using namespace mu::notation;

const InstrumentTemplateList& InstrumentsRepositoryStub::instrumentTemplates() const
{
    static InstrumentTemplateList l;
    return l;
}

const InstrumentTemplate& InstrumentsRepositoryStub::instrumentTemplate(const muse::String&) const
{
    static InstrumentTemplate t;
    return t;
}

const ScoreOrderList& InstrumentsRepositoryStub::orders() const
{
    static ScoreOrderList l;
    return l;
}

const ScoreOrder& InstrumentsRepositoryStub::order(const muse::String&) const
{
    static ScoreOrder o;
    return o;
}

const InstrumentGenreList& InstrumentsRepositoryStub::genres() const
{
    static InstrumentGenreList l;
    return l;
}

const InstrumentGroupList& InstrumentsRepositoryStub::groups() const
{
    static InstrumentGroupList l;
    return l;
}
