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

    void setParts(const PartInstrumentList& parts, const ScoreOrder& order) override;

    void removeParts(const muse::IDList& partsIds) override;
    void removeStaves(const muse::IDList& stavesIds) override;

    bool appendStaff(Staff* staff, const muse::ID& destinationPartId) override;
    bool appendLinkedStaff(Staff* staff, const muse::ID& sourceStaffId, const muse::ID& destinationPartId) override;

    void replaceInstrument(const InstrumentKey& instrumentKey, const Instrument& newInstrument,
                           const StaffType* newStaffType = nullptr) override;
    void replaceDrumset(const InstrumentKey& instrumentKey, const Drumset& newDrumset, bool undoable = true) override;

    void addSystemObjects(const muse::IDList& stavesIds) override;
    void removeSystemObjects(const muse::IDList& stavesIds) override;
    void moveSystemObjects(const muse::ID& sourceStaffId, const muse::ID& destinationStaffId) override;

private:
    void startGlobalEdit(const muse::TranslatableString& actionName);
    void endGlobalEdit();

    void onPartsRemoved(const std::vector<Part*>& parts) override;

    std::vector<INotationPartsPtr> excerptsParts() const;
    mu::engraving::Excerpt* findExcerpt(const muse::ID& initialPartId) const;

    ExcerptNotationList m_excerpts;
};
}

#endif // MU_NOTATION_MASTERNOTATIONPARTS_H
