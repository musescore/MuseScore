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
#include "selectinstrumentscenario.h"

#include "log.h"

using namespace muse;
using namespace mu::instrumentsscene;
using namespace mu::notation;

RetVal<PartInstrumentListScoreOrder> SelectInstrumentsScenario::selectInstruments() const
{
    ValMap params {
        { "canSelectMultipleInstruments", Val(true) }
    };

    return selectInstruments(params);
}

RetVal<InstrumentTemplate> SelectInstrumentsScenario::selectInstrument(const InstrumentKey& currentInstrumentKey) const
{
    ValMap params {
        { "canSelectMultipleInstruments", Val(false) },
        { "currentInstrumentId", Val(currentInstrumentKey.instrumentId.toStdString()) }
    };

    RetVal<PartInstrumentListScoreOrder> selectedInstruments = selectInstruments(params);
    if (!selectedInstruments.ret) {
        return selectedInstruments.ret;
    }

    const InstrumentTemplate& templ = selectedInstruments.val.instruments.first().instrumentTemplate;
    return RetVal<InstrumentTemplate>::make_ok(templ);
}

RetVal<PartInstrumentListScoreOrder> SelectInstrumentsScenario::selectInstruments(const ValMap& params) const
{
    static const Uri SELECT_INSTRUMENT_URI = Uri("musescore://instruments/select");
    if (interactive()->isOpened(Uri(SELECT_INSTRUMENT_URI)).val) {
        return make_ret(Ret::Code::Cancel);
    }

    UriQuery q(SELECT_INSTRUMENT_URI);
    q.set(params);

    RetVal<Val> retVal = interactive()->openSync(q);
    if (!retVal.ret) {
        return retVal.ret;
    }

    ValMap content = retVal.val.toMap();

    ValList instruments = content["instruments"].toList();

    IF_ASSERT_FAILED(!instruments.empty()) {
        return make_ret(Ret::Code::UnknownError);
    }

    PartInstrumentListScoreOrder result;

    for (const Val& obj: instruments) {
        ValMap map = obj.toMap();
        PartInstrument pi;

        pi.partId = ID(map["partId"].toString());
        pi.isExistingPart = map["isExistingPart"].toBool();
        pi.isSoloist = map["isSoloist"].toBool();

        String instrumentId = String::fromStdString(map["instrumentId"].toString());
        pi.instrumentTemplate = instrumentsRepository()->instrumentTemplate(instrumentId);

        result.instruments << pi;
    }

    ValMap order = content["scoreOrder"].toMap();
    result.scoreOrder = instrumentsRepository()->order(String::fromStdString(order["id"].toString()));
    result.scoreOrder.customized = order["customized"].toBool();

    return RetVal<PartInstrumentListScoreOrder>::make_ok(result);
}
