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
#ifndef MU_DOMAIN_NOTATION_NOTATIONINSTRUMENTS_H
#define MU_DOMAIN_NOTATION_NOTATIONINSTRUMENTS_H

#include "inotationinstruments.h"

#include "igetscore.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace mu {
namespace domain {
namespace notation {
class NotationInstruments : public INotationInstruments
{
    INJECT(notation, context::IGlobalContext, globalContext)

public:
    NotationInstruments(IGetScore* getScore);

    PartList parts() const override;
    InstrumentList instrumentList(const QString& partId) const override;
    StaffList staffList(const QString &partId, const QString &instrumentId) const override;

    void setPartVisible(const QString &partId, bool visible) override;
    void setInstrumentVisible(const QString& partId, const QString& instrumentId, bool visible) override;
    void setStaffVisible(const QString &partId, const QString &instrumentId, const QString &staffId, bool visible) override;

    async::Notification instrumentsChanged() const override;

private:
    PartList scoreParts() const;
    PartList excerptParts() const;

    Part* part(const QString& id) const;
    Instrument* instrument(const QString &partId, const QString &instrumentId) const;

    IGetScore* m_getScore = nullptr;
    async::Notification m_instrumentsChanged;
};
}
}
}

#endif // MU_DOMAIN_NOTATION_NOTATIONINSTRUMENTS_H
