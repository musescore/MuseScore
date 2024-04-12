/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#include "braillewriter.h"

#include <QBuffer>

#include "braille.h"

using namespace mu::project;
using namespace muse;

namespace mu::engraving {
std::vector<INotationWriter::UnitType> BrailleWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool BrailleWriter::supportsUnitType(UnitType unitType) const
{
    std::vector<UnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

muse::Ret BrailleWriter::write(notation::INotationPtr notation, muse::io::IODevice& destinationDevice, const Options&)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    mu::engraving::Score* score = notation->elements()->msScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    QByteArray qdata;
    QBuffer buf(&qdata);
    buf.open(QIODevice::WriteOnly);

    Ret ret = Braille(score).write(buf);
    if (ret) {
        ByteArray data = ByteArray::fromQByteArrayNoCopy(qdata);
        destinationDevice.write(data);
    }
    return ret;
}

muse::Ret BrailleWriter::writeList(const notation::INotationPtrList&, muse::io::IODevice&, const Options&)
{
    NOT_SUPPORTED;
    return muse::Ret(muse::Ret::Code::NotSupported);
}
}
