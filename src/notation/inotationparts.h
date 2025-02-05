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
#ifndef MU_NOTATION_INOTATIONPARTS_H
#define MU_NOTATION_INOTATIONPARTS_H

#include "notationtypes.h"
#include "async/notification.h"
#include "async/channel.h"
#include "async/notifylist.h"
#include "types/retval.h"

namespace mu::notation {
class INotationParts
{
public:
    virtual ~INotationParts() = default;

    virtual muse::async::NotifyList<const Part*> partList() const = 0;
    virtual muse::async::NotifyList<const Staff*> staffList(const muse::ID& partId) const = 0;

    virtual bool hasParts() const = 0;

    virtual const Part* part(const muse::ID& partId) const = 0;
    virtual bool partExists(const muse::ID& partId) const = 0;

    virtual const Staff* staff(const muse::ID& staffId) const = 0;
    virtual bool staffExists(const muse::ID& staffId) const = 0;

    virtual StaffConfig staffConfig(const muse::ID& staffId) const = 0;
    virtual ScoreOrder scoreOrder() const = 0;

    virtual void setParts(const PartInstrumentList& instruments, const ScoreOrder& order) = 0;
    virtual void setScoreOrder(const ScoreOrder& order) = 0;
    virtual void setPartVisible(const muse::ID& partId, bool visible) = 0;
    virtual bool setVoiceVisible(const muse::ID& staffId, int voiceIndex, bool visible) = 0;
    virtual void setStaffVisible(const muse::ID& staffId, bool visible) = 0;
    virtual void setPartSharpFlat(const muse::ID& partId, const SharpFlat& sharpFlat) = 0;
    virtual void setInstrumentName(const InstrumentKey& instrumentKey, const QString& name) = 0;
    virtual void setInstrumentAbbreviature(const InstrumentKey& instrumentKey, const QString& abbreviature) = 0;
    virtual void setStaffType(const muse::ID& staffId, StaffTypeId type) = 0;
    virtual void setStaffConfig(const muse::ID& staffId, const StaffConfig& config) = 0;

    virtual void removeParts(const muse::IDList& partsIds) = 0;
    virtual void removeStaves(const muse::IDList& stavesIds) = 0;

    enum class InsertMode {
        Before,
        After
    };

    virtual void moveParts(const muse::IDList& sourcePartsIds, const muse::ID& destinationPartId, InsertMode mode = InsertMode::Before) = 0;
    virtual void moveStaves(const muse::IDList& sourceStavesIds, const muse::ID& destinationStaffId,
                            InsertMode mode = InsertMode::Before) = 0;

    virtual bool appendStaff(Staff* staff, const muse::ID& destinationPartId) = 0;
    virtual bool appendLinkedStaff(Staff* staff, const muse::ID& sourceStaffId, const muse::ID& destinationPartId) = 0;

    virtual void insertPart(Part* part, size_t index) = 0;

    virtual void replacePart(const muse::ID& partId, Part* newPart) = 0;
    virtual void replaceInstrument(const InstrumentKey& instrumentKey, const Instrument& newInstrument,
                                   const StaffType* newStaffType = nullptr) = 0;
    virtual void replaceDrumset(const InstrumentKey& instrumentKey, const Drumset& newDrumset, bool undoable = true) = 0;

    virtual const std::vector<Staff*>& systemObjectStaves() const = 0;
    virtual muse::async::Notification systemObjectStavesChanged() const = 0;

    virtual void addSystemObjects(const muse::IDList& stavesIds) = 0;
    virtual void removeSystemObjects(const muse::IDList& stavesIds) = 0;
    virtual void moveSystemObjects(const muse::ID& sourceStaffId, const muse::ID& destinationStaffId) = 0;

    virtual muse::async::Notification partsChanged() const = 0;
    virtual muse::async::Notification scoreOrderChanged() const = 0;
};

using INotationPartsPtr = std::shared_ptr<INotationParts>;
}

#endif // MU_NOTATION_INOTATIONPARTS_H
