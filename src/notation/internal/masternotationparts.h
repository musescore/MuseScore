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

    void removeParts(const IDList& partsIds) override;
    void removeStaves(const IDList& stavesIds) override;

    void appendStaff(Staff* staff, const ID& destinationPartId) override;
    void appendLinkedStaff(Staff* staff, const ID& sourceStaffId, const ID& destinationPartId) override;

    void replaceInstrument(const InstrumentKey& instrumentKey, const Instrument& newInstrument) override;
    void replaceDrumset(const InstrumentKey& instrumentKey, const Drumset& newDrumset) override;

private:
    void startGlobalEdit();
    void endGlobalEdit();

    std::vector<INotationPartsPtr> excerptsParts() const;

    ExcerptNotationList m_excerpts;
};
}

#endif // MU_NOTATION_MASTERNOTATIONPARTS_H
