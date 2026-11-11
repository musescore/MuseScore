/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include <gmock/gmock.h>

#include "async/notifylist.h"

#include "notation/inotationparts.h"

namespace mu::notation {
class NotationPartsMock : public INotationParts
{
public:
    MOCK_METHOD(muse::async::NotifyList<const Part*>, partList, (), (const, override));
    MOCK_METHOD(muse::async::NotifyList<const Staff*>, staffList, (const muse::ID& partId), (const, override));

    MOCK_METHOD(bool, hasParts, (), (const, override));

    MOCK_METHOD(const Part*, part, (const muse::ID& partId), (const, override));
    MOCK_METHOD(bool, partExists, (const muse::ID& partId), (const, override));

    MOCK_METHOD(const Staff*, staff, (const muse::ID& staffId), (const, override));
    MOCK_METHOD(bool, staffExists, (const muse::ID& staffId), (const, override));

    MOCK_METHOD(StaffConfig, staffConfig, (const muse::ID& staffId, engraving::Fraction tick), (const, override));
    MOCK_METHOD(ScoreOrder, scoreOrder, (), (const, override));

    MOCK_METHOD(void, setParts, (const PartInstrumentList& instruments, const ScoreOrder& order), (override));
    MOCK_METHOD(void, setScoreOrder, (const ScoreOrder& order), (override));
    MOCK_METHOD(void, setPartVisible, (const muse::ID& partId, bool visible), (override));
    MOCK_METHOD(bool, setVoiceVisible, (const muse::ID& staffId, int voiceIndex, bool visible), (override));
    MOCK_METHOD(void, setStaffVisible, (const muse::ID& staffId, bool visible), (override));
    MOCK_METHOD(void, setPartSharpFlat, (const muse::ID& partId, const SharpFlat& sharpFlat), (override));
    MOCK_METHOD(void, setInstrumentName, (const InstrumentKey& instrumentKey, const QString& name), (override));
    MOCK_METHOD(void, setInstrumentAbbreviature, (const InstrumentKey& instrumentKey, const QString& abbreviature), (override));
    MOCK_METHOD(void, setInstrumentGroupNameOptions, (const std::vector<InstrumentKey>& instruments, bool useCustom,
                                                      const QString& longName, const QString& shortName), (override));
    MOCK_METHOD(void, setInstrumentNumber, (const InstrumentKey& instrumentKey, int v), (override));
    MOCK_METHOD(void, setStaffType, (const muse::ID& staffId, StaffTypeId type), (override));
    MOCK_METHOD(void, setStaffConfig, (const muse::ID& staffId, const StaffConfig& config, engraving::Fraction tick), (override));
    MOCK_METHOD(void, setSharedPartEnabled, (const muse::ID& partId, bool enable), (override));

    MOCK_METHOD(void, removeParts, (const muse::IDList& partsIds), (override));
    MOCK_METHOD(void, removeStaves, (const muse::IDList& stavesIds), (override));

    MOCK_METHOD(void, reorderParts, (const muse::IDList& partIds, const std::vector<SystemObjectsMove>& systemObjectsMoves), (override));
    MOCK_METHOD(void, moveStaves, (const muse::IDList& sourceStavesIds, const muse::ID& destinationStaffId, InsertMode mode),
                (override));

    MOCK_METHOD(bool, appendStaff, (Staff * staff, const muse::ID& destinationPartId), (override));
    MOCK_METHOD(bool, appendStaffLinkedToMaster, (Staff * staff, Staff * masterSourceStaff, const muse::ID& destinationPartId),
                (override));
    MOCK_METHOD(bool, appendLinkedStaff, (Staff * staff, const muse::ID& sourceStaffId, const muse::ID& destinationPartId), (override));

    MOCK_METHOD(void, insertPart, (Part * part, size_t index), (override));

    MOCK_METHOD(void, replacePart, (const muse::ID& partId, Part * newPart), (override));
    MOCK_METHOD(void, replaceInstrument, (const InstrumentKey& instrumentKey, const Instrument& newInstrument,
                                          const StaffType * newStaffType), (override));
    MOCK_METHOD(void, replaceDrumset, (const InstrumentKey& instrumentKey, const Drumset& newDrumset, bool undoable), (override));

    MOCK_METHOD(const std::vector<Staff*>&, systemObjectStaves, (), (const, override));
    MOCK_METHOD(muse::async::Notification, systemObjectStavesChanged, (), (const, override));

    MOCK_METHOD(void, addSystemObjects, (const muse::IDList& stavesIds), (override));
    MOCK_METHOD(void, removeSystemObjects, (const muse::IDList& stavesIds), (override));

    MOCK_METHOD(void, toggleStaveSharing, (bool on), (override));

    MOCK_METHOD(muse::async::Notification, partsChanged, (), (const, override));
    MOCK_METHOD(muse::async::Notification, scoreOrderChanged, (), (const, override));

    MOCK_METHOD(muse::async::Notification, sharedPartsChanged, (), (const, override));
};
}
