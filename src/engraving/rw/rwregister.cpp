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
#include "rwregister.h"

#include "types/constants.h"

#include "read114/read114.h"
#include "read206/read206.h"
#include "read302/read302.h"
#include "read400/read400.h"
#include "read410/read410.h"
#include "read460/read460.h"

#include "write/writer.h"

using namespace mu::engraving;
using namespace mu::engraving::rw;

IReaderPtr RWRegister::reader(int version)
{
    if (version < 0) {
        version = Constants::MSC_VERSION;
    }

    if (version <= 114) {
        return std::make_shared<read114::Read114>();
    } else if (version <= 207) {
        return std::make_shared<read206::Read206>();
    } else if (version < 400) {
        return std::make_shared<read302::Read302>();
    } else if (version < 410) {
        return std::make_shared<read400::Read400>();
    } else if (version < 460) {
        return std::make_shared<read410::Read410>();
    }

    return std::make_shared<read460::Read460>();
}

IWriterPtr RWRegister::writer(const muse::modularity::ContextPtr& iocCtx)
{
    return std::make_shared<write::Writer>(iocCtx);
}
