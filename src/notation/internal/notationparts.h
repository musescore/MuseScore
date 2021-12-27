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
#ifndef MU_NOTATION_NOTATIONPARTS_H
#define MU_NOTATION_NOTATIONPARTS_H

#include "inotationparts.h"
#include "async/asyncable.h"
#include "inotationundostack.h"
#include "inotationinteraction.h"

namespace mu::notation {
class IGetScore;
class NotationParts : public INotationParts, public async::Asyncable
{
public:
    NotationParts(IGetScore* getScore, INotationInteractionPtr interaction, INotationUndoStackPtr undoStack);

    async::NotifyList<const Part*> partList() const override;
    async::NotifyList<const Staff*> staffList(const ID& partId) const override;

    const Part* part(const ID& partId) const override;
    bool partExists(const ID& partId) const override;

    const Staff* staff(const ID& staffId) const override;
    bool staffExists(const ID& staffId) const override;

    StaffConfig staffConfig(const ID& staffId) const override;
    ScoreOrder scoreOrder() const override;

    void setParts(const PartInstrumentList& parts, const ScoreOrder& order) override;
    void setScoreOrder(const ScoreOrder& order) override;
    void setPartVisible(const ID& partId, bool visible) override;
    bool setVoiceVisible(const ID& staffId, int voiceIndex, bool visible) override;
    void setStaffVisible(const ID& staffId, bool visible) override;
    void setPartName(const ID& partId, const QString& name) override;
    void setPartSharpFlat(const ID& partId, const SharpFlat& sharpFlat) override;
    void setPartTransposition(const ID& partId, const Interval& transpose) override;
    void setInstrumentName(const InstrumentKey& instrumentKey, const QString& name) override;
    void setInstrumentAbbreviature(const InstrumentKey& instrumentKey, const QString& abbreviature) override;
    void setStaffType(const ID& staffId, StaffType type) override;
    void setStaffConfig(const ID& staffId, const StaffConfig& config) override;

    void removeParts(const IDList& partsIds) override;
    void removeStaves(const IDList& stavesIds) override;

    void moveParts(const IDList& sourcePartsIds, const ID& destinationPartId, InsertMode mode = InsertMode::Before) override;
    void moveStaves(const IDList& sourceStavesIds, const ID& destinationStaffId, InsertMode mode = InsertMode::Before) override;

    void appendStaff(Staff* staff, const ID& destinationPartId) override;
    void appendLinkedStaff(Staff* staff, const ID& sourceStaffId, const ID& destinationPartId) override;

    void insertPart(Part* part, size_t index) override;

    void replacePart(const ID& partId, Part* newPart) override;
    void replaceInstrument(const InstrumentKey& instrumentKey, const Instrument& newInstrument) override;
    void replaceDrumset(const InstrumentKey& instrumentKey, const Drumset& newDrumset) override;

    async::Notification partsChanged() const override;
    async::Notification scoreOrderChanged() const override;

protected:
    Ms::Score* score() const;
    INotationUndoStackPtr undoStack() const;

    void startEdit();
    void apply();

private:
    void updatePartTitles();

    void doSetScoreOrder(const ScoreOrder& order);
    void doMoveStaves(const std::vector<Staff*>& staves, int destinationStaffIndex, Part* destinationPart = nullptr);
    void doRemoveParts(const std::vector<Part*>& parts);
    void doAppendStaff(Staff* staff, Part* destinationPart);
    void doSetStaffConfig(Staff* staff, const StaffConfig& config);
    void doInsertPart(Part* part, int index);

    Part* partModifiable(const ID& partId) const;
    Staff* staffModifiable(const ID& staffId) const;

    std::vector<Staff*> staves(const IDList& stavesIds) const;
    std::vector<Part*> parts(const IDList& partsIds) const;

    void appendStaves(Part* part, const InstrumentTemplate& templ);
    void insertStaff(Staff* staff, int destinationStaffIndex);
    void initStaff(Staff* staff, const InstrumentTemplate& templ, const Ms::StaffType* staffType, int cleffIndex);

    void removeMissingParts(const PartInstrumentList& newParts);
    void appendNewParts(const PartInstrumentList& parts);
    void updateSoloist(const PartInstrumentList& parts);
    void sortParts(const PartInstrumentList& parts, const QList<Ms::Staff*>& originalStaves);

    void updateTracks();

    int resolveNewInstrumentNumber(const InstrumentTemplate& instrument, const PartInstrumentList& allNewInstruments) const;

    void setBracketsAndBarlines();

    void endInteractionWithScore();

    void notifyAboutPartChanged(const Part* part) const;
    void notifyAboutPartAdded(const Part* part) const;
    void notifyAboutPartRemoved(const Part* part) const;
    void notifyAboutPartReplaced(const Part* oldPart, const Part* newPart) const;
    void notifyAboutStaffChanged(const Staff* staff) const;
    void notifyAboutStaffAdded(const Staff* staff, const ID& partId) const;
    void notifyAboutStaffRemoved(const Staff* staff) const;

    IGetScore* m_getScore = nullptr;
    INotationUndoStackPtr m_undoStack;
    INotationInteractionPtr m_interaction;
    async::Notification m_partsChanged;
    async::Notification m_scoreOrderChanged;

    mutable async::ChangedNotifier<const Part*> m_partChangedNotifier;
    mutable std::map<ID, async::ChangedNotifier<const Staff*> > m_staffChangedNotifierMap;
};
}

#endif // MU_NOTATION_NOTATIONPARTS_H
