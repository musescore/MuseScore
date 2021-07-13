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
#include "selectinstrumentscenario.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;

mu::RetVal<PartInstrumentListScoreOrder> SelectInstrumentsScenario::selectInstruments(SelectInstrumentsMode mode) const
{
    QStringList params;
    if (mode == SelectInstrumentsMode::ShowCurrentInstruments) {
        params << "initiallySelectedPartIds=" + partsIds().join(",");
        params << "currentScoreOrderId=" + scoreOrder().id;
    }

    return selectInstruments(params);
}

mu::RetVal<Instrument> SelectInstrumentsScenario::selectInstrument(const std::string& currentInstrumentId) const
{
    RetVal<Instrument> result;

    QStringList params {
        "canSelectMultipleInstruments=false",
        "currentInstrumentId=" + QString::fromStdString(currentInstrumentId)
    };

    RetVal<PartInstrumentListScoreOrder> selectedInstruments = selectInstruments(params);
    if (!selectedInstruments.ret) {
        result.ret = selectedInstruments.ret;
        return result;
    }

    result.ret = make_ret(Ret::Code::Ok);

    if (selectedInstruments.val.instruments.empty()) {
        return result;
    }

    result.val = selectedInstruments.val.instruments.first().instrument;
    return result;
}

mu::RetVal<PartInstrumentListScoreOrder> SelectInstrumentsScenario::selectInstruments(const QStringList& params) const
{
    RetVal<PartInstrumentListScoreOrder> result;

    QString uri = QString("musescore://instruments/select?%1").arg(params.join('&'));
    RetVal<Val> instruments = interactive()->open(uri.toStdString());
    if (!instruments.ret) {
        result.ret = instruments.ret;
        return result;
    }

    result.ret = make_ret(Ret::Code::Ok);

    QVariantMap info = instruments.val.toQVariant().toMap();
    result.val.scoreOrder = info["scoreOrder"].value<ScoreOrder>();

    for (const QVariant& obj: info["instrumentList"].toList()) {
        QVariantMap map = obj.toMap();
        PartInstrument pi;

        pi.isExistingPart = map["isExistingPart"].toBool();
        pi.isSoloist = map["isSoloist"].toBool();
        pi.partId = map["id"].toString();
        pi.instrument = map["instrument"].value<Instrument>();

        result.val.instruments << pi;
    }

    return result;
}

INotationPartsPtr SelectInstrumentsScenario::notationParts() const
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->parts();
}

IDList SelectInstrumentsScenario::partsIds() const
{
    auto _notationParts = notationParts();
    if (!_notationParts) {
        return IDList();
    }

    async::NotifyList<const Part*> parts = _notationParts->partList();

    IDList result;
    for (const Part* part: parts) {
        result << part->id();
    }

    return result;
}

ScoreOrder SelectInstrumentsScenario::scoreOrder() const
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return ScoreOrder();
    }

    return notation->scoreOrder();
}
