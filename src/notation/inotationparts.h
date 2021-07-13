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
#ifndef MU_NOTATION_INOTATIONPARTS_H
#define MU_NOTATION_INOTATIONPARTS_H

#include "notationtypes.h"
#include "async/notification.h"
#include "async/channel.h"
#include "async/notifylist.h"
#include "retval.h"

namespace mu::notation {
using ID = QString;
using IDList = QList<ID>;

class INotationParts
{
public:
    virtual ~INotationParts() = default;

    virtual async::NotifyList<const Part*> partList() const = 0;
    virtual async::NotifyList<Instrument> instrumentList(const ID& partId) const = 0;
    virtual async::NotifyList<const Staff*> staffList(const ID& partId, const ID& instrumentId) const = 0;

    virtual ValCh<bool> canChangeInstrumentVisibility(const ID& instrumentId, const ID& fromPartId) const = 0;
    virtual bool voiceVisible(int voiceIndex) const = 0;

    virtual void setParts(const PartInstrumentList& instruments) = 0;
    virtual void setScoreOrder(const ScoreOrder& order) = 0;
    virtual void setPartVisible(const ID& partId, bool visible) = 0;
    virtual void setInstrumentVisible(const ID& instrumentId, const ID& fromPartId, bool visible) = 0;
    virtual void setStaffVisible(const ID& staffId, bool visible) = 0;
    virtual void setVoiceVisible(int voiceIndex, bool visible) = 0;
    virtual void setVoiceVisible(const ID& staffId, int voiceIndex, bool visible) = 0;
    virtual void setPartName(const ID& partId, const QString& name) = 0;
    virtual void setPartSharpFlat(const ID& partId, const SharpFlat& sharpFlat) = 0;
    virtual void setPartTransposition(const ID& partId, const Interval& transpose) = 0;
    virtual void setInstrumentName(const ID& instrumentId, const ID& fromPartId, const QString& name) = 0;
    virtual void setInstrumentAbbreviature(const ID& instrumentId, const ID& fromPartId, const QString& abbreviature) = 0;
    virtual void setStaffType(const ID& staffId, StaffType type) = 0;
    virtual void setCutawayEnabled(const ID& staffId, bool enabled) = 0;
    virtual void setSmallStaff(const ID& staffId, bool smallStaff) = 0;

    virtual void setStaffConfig(const ID& staffId, const StaffConfig& config) = 0;

    virtual void removeParts(const IDList& partsIds) = 0;
    virtual void removeInstruments(const IDList& instrumentsIds, const ID& fromPartId) = 0;
    virtual void removeStaves(const IDList& stavesIds) = 0;

    enum class InsertMode {
        Before,
        After
    };

    virtual void moveParts(const IDList& sourcePartsIds, const ID& destinationPartId, InsertMode mode = InsertMode::Before) = 0;
    virtual void moveInstruments(const IDList& sourceInstrumentsIds, const ID& sourcePartId, const ID& destinationPartId,
                                 const ID& destinationInstrumentId, InsertMode mode = InsertMode::Before) = 0;
    virtual void moveStaves(const IDList& sourceStavesIds, const ID& destinationStaffId, InsertMode mode = InsertMode::Before) = 0;

    virtual void appendDoublingInstrument(const Instrument& instrument, const ID& destinationPartId) = 0;
    virtual void appendStaff(Staff* staff, const ID& destinationPartId) = 0;

    virtual void cloneStaff(const ID& sourceStaffId, const ID& destinationStaffId) = 0;

    virtual void replaceInstrument(const ID& instrumentId, const ID& fromPartId, const Instrument& newInstrument) = 0;

    virtual async::Notification partsChanged() const = 0;
};

using INotationPartsPtr = std::shared_ptr<INotationParts>;
}

#endif // MU_NOTATION_INOTATIONPARTS_H
