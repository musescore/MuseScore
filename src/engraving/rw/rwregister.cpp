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
#include "rwregister.h"

#include "read114/read114.h"
#include "read206/read206.h"
#include "read302/read302.h"
#include "read400/read400.h"

#include "write/writer.h"

using namespace mu::engraving;
using namespace mu::engraving::rw;

static const int LATEST_VERSION(400);

static IReaderPtr makeReader(int version, bool testMode)
{
    if (version <= 114) {
        return std::make_shared<read114::Read114>();
    } else if (version <= 207) {
        return std::make_shared<read206::Read206>();
    } else if (version < 400 || testMode) {
        return std::make_shared<read302::Read302>();
    }

    return std::make_shared<read400::Read400>();
}

IReaderPtr RWRegister::reader(int version)
{
    return makeReader(version, MScore::testMode);
}

IReaderPtr RWRegister::latestReader()
{
    return makeReader(LATEST_VERSION, false);
}

IWriterPtr RWRegister::writer()
{
    return std::make_shared<write::Writer>();
}
