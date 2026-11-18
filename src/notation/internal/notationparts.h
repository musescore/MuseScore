/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <functional>
#include <map>

#include "async/asyncable.h"
#include "async/notifylist.h"

#include "engraving/dom/score.h"
#include "engraving/editing/editpart.h"

#include "inotation_fwd.h"
#include "inotationparts.h"

namespace muse {
class TranslatableString;
}

namespace mu::notation {
class IGetScore;
class NotationParts : public INotationParts, public muse::async::Asyncable
{
public:
    NotationParts(IGetScore* getScore, INotationInteractionPtr interaction, INotationUndoStackPtr undoStack, INotationStylePtr style);

    muse::async::NotifyList<const Part*> partList() const override;
    muse::async::NotifyList<const Staff*> staffList(const muse::ID& partId) const override;

    bool hasParts() const override;

    const Part* part(const muse::ID& partId) const override;
    bool partExists(const muse::ID& partId) const override;

    const Staff* staff(const muse::ID& staffId) const override;
    bool staffExists(const muse::ID& staffId) const override;

    StaffConfig staffConfig(const muse::ID& staffId, engraving::Fraction tick = engraving::Fraction(0, 1)) const override;
    ScoreOrder scoreOrder() const override;

    void setParts(const PartInstrumentList& parts, const ScoreOrder& order) override;
    void setScoreOrder(const ScoreOrder& order) override;
    void setPartVisible(const muse::ID& partId, bool visible) override;
    bool setVoiceVisible(const muse::ID& staffId, int voiceIndex, bool visible) override;
    void setStaffVisible(const muse::ID& staffId, bool visible) override;
    void setPartSharpFlat(const muse::ID& partId, const SharpFlat& sharpFlat) override;
    void setInstrumentName(const InstrumentKey& instrumentKey, const QString& name) override;
    void setInstrumentAbbreviature(const InstrumentKey& instrumentKey, const QString& abbreviature) override;
    void setInstrumentGroupNameOptions(const std::vector<InstrumentKey>& instruments, bool useCustom, const QString& longName,
                                       const QString& shortName) override;
    void setInstrumentNumber(const InstrumentKey& instrumentKey, int v) override;
    void setStaffType(const muse::ID& staffId, StaffTypeId type) override;
    void setStaffConfig(const muse::ID& staffId, const StaffConfig& config, engraving::Fraction tick = engraving::Fraction(0, 1)) override;
    void setSharedPartEnabled(const muse::ID& partId, bool enable) override;

    void removeParts(const muse::IDList& partsIds) override;
    void removeStaves(const muse::IDList& stavesIds) override;

    bool setPartsLayout(const PartsLayout& layout) override;
    void reorderStaves(const muse::IDList& staffIds) override;

    bool appendStaff(Staff* staff, const muse::ID& destinationPartId) override;
    bool appendLinkedStaff(Staff* staff, const muse::ID& sourceStaffId, const muse::ID& destinationPartId) override;

    void insertPart(Part* part, size_t index) override;

    void replacePart(const muse::ID& partId, Part* newPart) override;
    void replaceInstrument(const InstrumentKey& instrumentKey, const Instrument& newInstrument,
                           const StaffType* newStaffType = nullptr) override;
    void replaceDrumset(const InstrumentKey& instrumentKey, const Drumset& newDrumset, bool undoable = true) override;

    const std::vector<Staff*>& systemObjectStaves() const override;
    muse::async::Notification systemObjectStavesChanged() const override;

    void addSystemObjects(const muse::IDList& stavesIds) override;
    void removeSystemObjects(const muse::IDList& stavesIds) override;

    void toggleStaveSharing(bool on) override;
    const SharedPart* movePartsToNewSharedPart(const muse::IDList& partIds) override;
    void movePartsToSharedPart(const muse::IDList& partIds, const muse::ID& sharedPartId) override;
    void ungroupSharedPart(const muse::ID& sharedPartId) override;

    muse::async::Notification partsChanged() const override;
    muse::async::Notification scoreOrderChanged() const override;

    muse::async::Notification sharedPartsChanged() const override;

private:
    mu::engraving::Score* score() const;

    muse::Ret transaction(const muse::TranslatableString& actionName, const std::function<void(engraving::Transaction&)>& func);

    void listenUndoStackChanges();
    void updatePartsAndSystemObjectStaves(const mu::engraving::ScoreChanges& changes = {});
    void listenStyleChanges();

    std::vector<engraving::EditPart::PartEntry> partEntries(const PartInstrumentList& parts) const;

    std::vector<Staff*> staves(const muse::IDList& stavesIds) const;
    std::vector<Part*> parts(const muse::IDList& partsIds) const;

    std::vector<engraving::EditPart::PartPlacement> editPartPlacements(const std::vector<PartPlacement>& placements) const;
    std::vector<engraving::EditPart::SystemObjectsMove> editPartSystemObjectsMoves(const std::vector<SystemObjectsMove>& moves) const;

    void endInteractionWithScore();

    void notifyAboutPartChanged(const Part* part) const;
    void notifyAboutPartAdded(const Part* part) const;
    void notifyAboutPartRemoved(const Part* part) const;
    void notifyAboutPartReplaced(const Part* oldPart, const Part* newPart) const;
    void notifyAboutStaffChanged(const Staff* staff) const;
    void notifyAboutStaffAdded(const Staff* staff) const;
    void notifyAboutStaffRemoved(const Staff* staff) const;

    IGetScore* m_getScore = nullptr;
    INotationUndoStackPtr m_undoStack;
    INotationInteractionPtr m_interaction;
    INotationStylePtr m_style;
    muse::async::Notification m_partsChanged;
    muse::async::Notification m_scoreOrderChanged;

    std::vector<Part*> m_parts;
    std::vector<Staff*> m_systemObjectStaves;
    muse::async::Notification m_systemObjectStavesChanged;
    muse::async::Notification m_sharedPartsChanged;

    bool m_ignoreUndoStackChanges = false;

    mutable muse::async::ChangedNotifier<const Part*> m_partChangedNotifier;
    mutable std::map<muse::ID, muse::async::ChangedNotifier<const Staff*> > m_staffChangedNotifierMap;
};
}
