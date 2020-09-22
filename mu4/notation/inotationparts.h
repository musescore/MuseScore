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
#ifndef MU_NOTATION_INOTATIONPARTS_H
#define MU_NOTATION_INOTATIONPARTS_H

#include "notationtypes.h"
#include "instruments/instrumentstypes.h"
#include "async/notification.h"
#include "async/channel.h"
#include "async/notifylist.h"
#include "retval.h"

namespace mu {
namespace notation {
using ID = QString;
using IDList = QList<ID>;

class INotationParts
{
public:
    virtual ~INotationParts() = default;

    virtual async::NotifyList<const Part*> partList() const = 0;
    virtual async::NotifyList<instruments::Instrument> instrumentList(const ID& partId) const = 0;
    virtual async::NotifyList<const Staff*> staffList(const ID& partId, const ID& instrumentId) const = 0;

    virtual ValCh<bool> canChangeInstrumentVisibility(const ID& instrumentId, const ID& fromPartId) const = 0;

    virtual void setInstruments(const instruments::InstrumentList& instruments) = 0;
    virtual void setPartVisible(const ID& partId, bool visible) = 0;
    virtual void setInstrumentVisible(const ID& instrumentId, const ID& fromPartId, bool visible) = 0;
    virtual void setStaffVisible(const ID& staffId, bool visible) = 0;
    virtual void setVoiceVisible(const ID& staffId, int voiceIndex, bool visible) = 0;
    virtual void setPartName(const ID& partId, const QString& name) = 0;
    virtual void setInstrumentName(const ID& instrumentId, const ID& fromPartId, const QString& name) = 0;
    virtual void setInstrumentAbbreviature(const ID& instrumentId, const ID& fromPartId, const QString& abbreviature) = 0;
    virtual void setStaffType(const ID& staffId, StaffType type) = 0;
    virtual void setCutawayEnabled(const ID& staffId, bool enabled) = 0;
    virtual void setSmallStaff(const ID& staffId, bool smallStaff) = 0;

    virtual void removeParts(const IDList& partsIds) = 0;
    virtual void removeInstruments(const IDList& instrumentsIds, const ID& fromPartId) = 0;
    virtual void removeStaves(const IDList& stavesIds) = 0;

    enum InsertMode {
        Before,
        After
    };

    virtual void moveParts(const IDList& sourcePartsIds, const ID& destinationPartId, InsertMode mode = Before) = 0;
    virtual void moveInstruments(const IDList& sourceInstrumentsIds, const ID& sourcePartId, const ID& destinationPartId,
                                 const ID& destinationInstrumentId, InsertMode mode = Before) = 0;
    virtual void moveStaves(const IDList& sourceStavesIds, const ID& destinationStaffId, InsertMode mode = Before) = 0;

    virtual void appendDoublingInstrument(const instruments::Instrument& instrument, const ID& destinationPartId) = 0;
    virtual void appendStaff(const ID& destinationPartId) = 0;
    virtual void appendLinkedStaff(const ID& originStaffId) = 0;

    virtual void replaceInstrument(const ID& instrumentId, const ID& fromPartId, const instruments::Instrument& newInstrument) = 0;

    virtual async::Notification partsChanged() const = 0;
};

using INotationPartsPtr = std::shared_ptr<INotationParts>;
}
}

#endif // MU_NOTATION_INOTATIONPARTS_H
