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
using namespace muse::async;
using namespace mu::instrumentsscene;
using namespace mu::notation;

muse::async::Promise<PartInstrumentListScoreOrder> SelectInstrumentsScenario::selectInstruments() const
{
    ValMap params {
        { "canSelectMultipleInstruments", Val(true) }
    };

    return selectInstruments(params);
}

muse::async::Promise<InstrumentTemplate> SelectInstrumentsScenario::selectInstrument(const InstrumentKey& currentInstrumentKey) const
{
    ValMap params {
        { "canSelectMultipleInstruments", Val(false) },
        { "currentInstrumentId", Val(currentInstrumentKey.instrumentId.toStdString()) }
    };

    return async::make_promise<InstrumentTemplate>([this, params](auto resolve, auto reject) {
        Promise<PartInstrumentListScoreOrder> selectedInstruments = selectInstruments(params);
        selectedInstruments.onResolve(this, [resolve](const PartInstrumentListScoreOrder& selectedInstruments) {
            const InstrumentTemplate& tpl = selectedInstruments.instruments.first().instrumentTemplate;
            (void)resolve(tpl);
        });
        selectedInstruments.onReject(this, [reject](int code, const std::string& msg) {
            (void)reject(code, msg);
        });

        return Promise<InstrumentTemplate>::dummy_result();
    });
}

muse::async::Promise<PartInstrumentListScoreOrder> SelectInstrumentsScenario::selectInstruments(const ValMap& params) const
{
    return async::make_promise<PartInstrumentListScoreOrder>([this, params](auto resolve, auto reject) {
        static const Uri SELECT_INSTRUMENT_URI = Uri("musescore://instruments/select");
        if (interactive()->isOpened(SELECT_INSTRUMENT_URI).val) {
            Ret ret = muse::make_ret(Ret::Code::Cancel);
            return reject(ret.code(), ret.text());
        }

        UriQuery q(SELECT_INSTRUMENT_URI);
        q.set(params);

        auto promise = interactive()->open(q);
        promise.onResolve(this, [this, resolve, reject](const Val& val) {
            ValMap content = val.toMap();
            ValList instruments = content["instruments"].toList();

            IF_ASSERT_FAILED(!instruments.empty()) {
                Ret ret = muse::make_ret(Ret::Code::UnknownError);
                (void)reject(ret.code(), ret.text());
                return;
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

            (void)resolve(result);
        });

        return Promise<PartInstrumentListScoreOrder>::dummy_result();
    }, PromiseType::AsyncByBody);
}
