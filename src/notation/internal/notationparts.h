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
#ifndef MU_NOTATION_NOTATIONPARTS_H
#define MU_NOTATION_NOTATIONPARTS_H

#include "inotationparts.h"
#include "async/asyncable.h"
#include "inotationundostack.h"
#include "inotationinteraction.h"

namespace mu::notation {
class IGetScore;
class NotationParts : public INotationParts, public muse::async::Asyncable
{
public:
    NotationParts(IGetScore* getScore, INotationInteractionPtr interaction, INotationUndoStackPtr undoStack);

    muse::async::NotifyList<const Part*> partList() const override;
    muse::async::NotifyList<const Staff*> staffList(const muse::ID& partId) const override;

    bool hasParts() const override;

    const Part* part(const muse::ID& partId) const override;
    bool partExists(const muse::ID& partId) const override;

    const Staff* staff(const muse::ID& staffId) const override;
    bool staffExists(const muse::ID& staffId) const override;

    StaffConfig staffConfig(const muse::ID& staffId) const override;
    ScoreOrder scoreOrder() const override;

    void setParts(const PartInstrumentList& parts, const ScoreOrder& order) override;
    void setScoreOrder(const ScoreOrder& order) override;
    void setPartVisible(const muse::ID& partId, bool visible) override;
    bool setVoiceVisible(const muse::ID& staffId, int voiceIndex, bool visible) override;
    void setStaffVisible(const muse::ID& staffId, bool visible) override;
    void setPartSharpFlat(const muse::ID& partId, const SharpFlat& sharpFlat) override;
    void setInstrumentName(const InstrumentKey& instrumentKey, const QString& name) override;
    void setInstrumentAbbreviature(const InstrumentKey& instrumentKey, const QString& abbreviature) override;
    void setStaffType(const muse::ID& staffId, StaffTypeId type) override;
    void setStaffConfig(const muse::ID& staffId, const StaffConfig& config) override;

    void removeParts(const muse::IDList& partsIds) override;
    void removeStaves(const muse::IDList& stavesIds) override;

    void moveParts(const muse::IDList& sourcePartsIds, const muse::ID& destinationPartId, InsertMode mode = InsertMode::Before) override;
    void moveStaves(const muse::IDList& sourceStavesIds, const muse::ID& destinationStaffId, InsertMode mode = InsertMode::Before) override;

    bool appendStaff(Staff* staff, const muse::ID& destinationPartId) override;
    bool appendLinkedStaff(Staff* staff, const muse::ID& sourceStaffId, const muse::ID& destinationPartId) override;

    void insertPart(Part* part, size_t index) override;

    void replacePart(const muse::ID& partId, Part* newPart) override;
    void replaceInstrument(const InstrumentKey& instrumentKey, const Instrument& newInstrument,
                           const StaffType* newStaffType = nullptr) override;
    void replaceDrumset(const InstrumentKey& instrumentKey, const Drumset& newDrumset, bool undoable = true) override;

    muse::async::Notification partsChanged() const override;
    muse::async::Notification scoreOrderChanged() const override;

protected:
    mu::engraving::Score* score() const;
    INotationUndoStackPtr undoStack() const;

    Part* partModifiable(const muse::ID& partId) const;

    void startEdit(const muse::TranslatableString& actionName);
    void apply();
    void rollback();

    virtual void onPartsRemoved(const std::vector<Part*>& parts);

private:
    friend class MasterNotationParts;

    void doSetScoreOrder(const ScoreOrder& order);
    void doRemoveParts(const std::vector<Part*>& parts);
    void doAppendStaff(Staff* staff, Part* destinationPart);
    void doSetStaffConfig(Staff* staff, const StaffConfig& config);
    void doInsertPart(Part* part, size_t index);

    Staff* staffModifiable(const muse::ID& staffId) const;

    std::vector<Staff*> staves(const muse::IDList& stavesIds) const;
    std::vector<Part*> parts(const muse::IDList& partsIds) const;

    mu::engraving::InstrumentChange* findInstrumentChange(const Part* part, const Fraction& tick) const;

    void appendStaves(Part* part, const InstrumentTemplate& templ, const mu::engraving::KeyList& keyList);
    void insertStaff(Staff* staff, engraving::staff_idx_t destinationStaffIndex);
    void initStaff(Staff* staff, const InstrumentTemplate& templ, const mu::engraving::StaffType* staffType, size_t cleffIndex);

    void removeMissingParts(const PartInstrumentList& newParts);
    void insertNewParts(const PartInstrumentList& parts, const mu::engraving::KeyList& keyList);
    void insertNewParts(const PartInstrumentList& parts);
    void updateSoloist(const PartInstrumentList& parts);
    void sortParts(const PartInstrumentList& parts);

    int resolveNewInstrumentNumber(const InstrumentTemplate& instrument, const PartInstrumentList& allNewInstruments) const;

    void setBracketsAndBarlines();

    void endInteractionWithScore();

    void notifyAboutPartChanged(const Part* part) const;
    void notifyAboutPartAdded(const Part* part) const;
    void notifyAboutPartRemoved(const Part* part) const;
    void notifyAboutPartReplaced(const Part* oldPart, const Part* newPart) const;
    void notifyAboutStaffChanged(const Staff* staff) const;
    void notifyAboutStaffAdded(const Staff* staff, const muse::ID& partId) const;
    void notifyAboutStaffRemoved(const Staff* staff) const;

    IGetScore* m_getScore = nullptr;
    INotationUndoStackPtr m_undoStack;
    INotationInteractionPtr m_interaction;
    muse::async::Notification m_partsChanged;
    muse::async::Notification m_scoreOrderChanged;

    mutable muse::async::ChangedNotifier<const Part*> m_partChangedNotifier;
    mutable std::map<muse::ID, muse::async::ChangedNotifier<const Staff*> > m_staffChangedNotifierMap;
};
}

#endif // MU_NOTATION_NOTATIONPARTS_H
