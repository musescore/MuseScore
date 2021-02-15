//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "selectinstrumentscenario.h"

using namespace mu::instruments;
using namespace mu::notation;

mu::RetVal<InstrumentList> SelectInstrumentsScenario::selectInstruments(SelectInstrumentsMode mode) const
{
    QStringList params;
    if (mode == SelectInstrumentsMode::ShowCurrentInstruments) {
        params << "initiallySelectedInstrumentIds=" + partsInstrumentIds().join(",");
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

    RetVal<InstrumentList> selectedInstruments = selectInstruments(params);
    if (!selectedInstruments.ret) {
        result.ret = selectedInstruments.ret;
        return result;
    }

    result.ret = make_ret(Ret::Code::Ok);

    if (selectedInstruments.val.empty()) {
        return result;
    }

    result.val = selectedInstruments.val.first();
    return result;
}

mu::RetVal<InstrumentList> SelectInstrumentsScenario::selectInstruments(const QStringList& params) const
{
    RetVal<InstrumentList> result;

    QString uri = QString("musescore://instruments/select?%1").arg(params.join('&'));
    RetVal<Val> instruments = interactive()->open(uri.toStdString());
    if (!instruments.ret) {
        result.ret = instruments.ret;
        return result;
    }

    result.ret = make_ret(Ret::Code::Ok);

    QVariantList objList = instruments.val.toQVariant().toList();
    for (const QVariant& obj: objList) {
        result.val << obj.value<Instrument>();
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

IDList SelectInstrumentsScenario::partsInstrumentIds() const
{
    auto _notationParts = notationParts();
    if (!_notationParts) {
        return IDList();
    }

    async::NotifyList<const Part*> parts = _notationParts->partList();

    IDList result;
    for (const Part* part: parts) {
        async::NotifyList<Instrument> selectedInstruments = _notationParts->instrumentList(part->id());

        for (const Instrument& instrument: selectedInstruments) {
            if (part->isDoublingInstrument(instrument.id)) {
                continue;
            }

            result << instrument.id;
        }
    }

    return result;
}
