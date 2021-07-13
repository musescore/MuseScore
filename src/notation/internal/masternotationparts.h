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

#ifndef MU_NOTATION_MASTERNOTATIONPARTS_H
#define MU_NOTATION_MASTERNOTATIONPARTS_H

#include "notationparts.h"
#include "imasternotation.h"

namespace mu::notation {
class MasterNotationParts : public NotationParts
{
public:
    MasterNotationParts(IGetScore* getScore, INotationInteractionPtr interaction, INotationUndoStackPtr undoStack);

    void setExcerpts(ExcerptNotationList excerpts);

    void setParts(const PartInstrumentList& instruments) override;
    void setScoreOrder(const ScoreOrder& order) override;
    void setInstrumentName(const ID& instrumentId, const ID& fromPartId, const QString& name) override;
    void setPartName(const ID& partId, const QString& name) override;
    void setPartSharpFlat(const ID& partId, const SharpFlat& sharpFlat) override;
    void setPartTransposition(const ID& partId, const Interval& transpose) override;
    void setInstrumentAbbreviature(const ID& instrumentId, const ID& fromPartId, const QString& abbreviature) override;
    void setStaffType(const ID& staffId, StaffType type) override;
    void setCutawayEnabled(const ID& staffId, bool enabled) override;
    void setSmallStaff(const ID& staffId, bool smallStaff) override;

    void setStaffConfig(const ID& staffId, const StaffConfig& config) override;

    void removeParts(const IDList& partsIds) override;
    void removeInstruments(const IDList& instrumentsIds, const ID& fromPartId) override;
    void removeStaves(const IDList& stavesIds) override;

    void moveParts(const IDList& sourcePartsIds, const ID& destinationPartId, InsertMode mode = InsertMode::Before) override;
    void moveInstruments(const IDList& sourceInstrumentIds, const ID& sourcePartId, const ID& destinationPartId,
                         const ID& destinationInstrumentId, InsertMode mode = InsertMode::Before) override;
    void moveStaves(const IDList& sourceStavesIds, const ID& destinationStaffId, InsertMode mode = InsertMode::Before) override;

    void appendDoublingInstrument(const Instrument& instrument, const ID& destinationPartId) override;
    void appendStaff(Staff* staff, const ID& destinationPartId) override;

    void cloneStaff(const ID& sourceStaffId, const ID& destinationStaffId) override;

    void replaceInstrument(const ID& instrumentId, const ID& fromPartId, const Instrument& newInstrument) override;

private:
    void startEdit();
    void apply();

    std::vector<INotationPartsPtr> excerptsParts() const;

    ExcerptNotationList m_excerpts;
};
}

#endif // MU_NOTATION_MASTERNOTATIONPARTS_H
