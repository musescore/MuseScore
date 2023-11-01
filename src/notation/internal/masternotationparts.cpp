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

#include "masternotationparts.h"

#include "engraving/dom/masterscore.h"
#include "engraving/dom/scoreorder.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/undo.h"
#include "engraving/dom/utils.h"

#include "log.h"

using namespace mu::notation;

static NotationParts* get_impl(const INotationPartsPtr& parts)
{
    return static_cast<NotationParts*>(parts.get());
}

MasterNotationParts::MasterNotationParts(IGetScore* getScore, INotationInteractionPtr interaction, INotationUndoStackPtr undoStack)
    : NotationParts(getScore, interaction, undoStack)
{
}

void MasterNotationParts::setExcerpts(ExcerptNotationList excerpts)
{
    m_excerpts = excerpts;
}

void MasterNotationParts::startGlobalEdit()
{
    NotationParts::startEdit();
    undoStack()->lock();
}

void MasterNotationParts::endGlobalEdit()
{
    undoStack()->unlock();
    NotationParts::apply();
}

void MasterNotationParts::setParts(const PartInstrumentList& partList, const ScoreOrder& order)
{
    TRACEFUNC;

    mu::engraving::KeyList keyList = score()->keyList();

    endInteractionWithScore();
    startGlobalEdit();

    doSetScoreOrder(order);
    removeMissingParts(partList);
    insertNewParts(partList, keyList);
    updateSoloist(partList);
    sortParts(partList);
    setBracketsAndBarlines();

    for (INotationPartsPtr excerptParts : excerptsParts()) {
        auto impl = get_impl(excerptParts);

        impl->removeMissingParts(partList);

        PartInstrumentList excerptPartList;
        for (mu::engraving::Part* part: impl->score()->parts()) {
            PartInstrument pi;
            pi.isExistingPart = true;
            pi.partId = part->id();
            excerptPartList << pi;
        }

        impl->sortParts(excerptPartList);

        impl->setBracketsAndBarlines();
    }

    endGlobalEdit();

    m_partChangedNotifier.changed();
}

void MasterNotationParts::removeParts(const IDList& partsIds)
{
    TRACEFUNC;

    startGlobalEdit();

    NotationParts::removeParts(partsIds);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->removeParts(partsIds);
    }

    endGlobalEdit();
}

void MasterNotationParts::removeStaves(const IDList& stavesIds)
{
    TRACEFUNC;

    startGlobalEdit();

    NotationParts::removeStaves(stavesIds);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->removeStaves(stavesIds);
    }

    endGlobalEdit();
}

bool MasterNotationParts::appendStaff(Staff* staff, const ID& destinationPartId)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(staff) {
        return false;
    }

    startGlobalEdit();

    //! NOTE: will be generated later after adding to the score
    staff->setId(mu::engraving::INVALID_ID);

    NotationParts::appendStaff(staff, destinationPartId);

    for (INotationPartsPtr parts : excerptsParts()) {
        Staff* excerptStaff = mu::engraving::toStaff(staff->linkedClone());
        if (!parts->appendStaff(excerptStaff, destinationPartId)) {
            excerptStaff->unlink();
            delete excerptStaff;
        }
    }

    endGlobalEdit();
    return true;
}

bool MasterNotationParts::appendLinkedStaff(Staff* staff, const mu::ID& sourceStaffId, const mu::ID& destinationPartId)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(staff) {
        return false;
    }

    startGlobalEdit();

    //! NOTE: will be generated later after adding to the score
    staff->setId(mu::engraving::INVALID_ID);

    NotationParts::appendLinkedStaff(staff, sourceStaffId, destinationPartId);

    for (INotationPartsPtr parts : excerptsParts()) {
        Staff* excerptStaff = staff->clone();
        if (!parts->appendLinkedStaff(excerptStaff, sourceStaffId, destinationPartId)) {
            excerptStaff->unlink();
            delete excerptStaff;
        }
    }

    endGlobalEdit();
    return true;
}

void MasterNotationParts::replaceInstrument(const InstrumentKey& instrumentKey, const Instrument& newInstrument)
{
    TRACEFUNC;

    startGlobalEdit();

    const Part* part = partModifiable(instrumentKey.partId);
    bool isMainInstrument = part && isMainInstrumentForPart(instrumentKey, part);

    NotationParts::replaceInstrument(instrumentKey, newInstrument);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->replaceInstrument(instrumentKey, newInstrument);
    }

    if (isMainInstrument) {
        if (mu::engraving::Excerpt* excerpt = findExcerpt(part->id())) {
            StringList allExcerptLowerNames;
            for (const mu::engraving::Excerpt* excerpt2 : score()->masterScore()->excerpts()) {
                allExcerptLowerNames.push_back(excerpt2->name().toLower());
            }

            String newName = mu::engraving::formatUniqueExcerptName(part->partName(), allExcerptLowerNames);
            excerpt->excerptScore()->undo(new mu::engraving::ChangeExcerptTitle(excerpt, newName));
        }
    }

    endGlobalEdit();
}

void MasterNotationParts::replaceDrumset(const InstrumentKey& instrumentKey, const Drumset& newDrumset)
{
    TRACEFUNC;

    startGlobalEdit();

    NotationParts::replaceDrumset(instrumentKey, newDrumset);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->replaceDrumset(instrumentKey, newDrumset);
    }

    endGlobalEdit();
}

void MasterNotationParts::onPartsRemoved(const std::vector<Part*>& parts)
{
    mu::engraving::MasterScore* master = score()->masterScore();
    std::vector<mu::engraving::Excerpt*> excerpts = master->excerpts();

    for (mu::engraving::Excerpt* excerpt : excerpts) {
        const ID& initialPartId = excerpt->initialPartId();
        if (!initialPartId.isValid()) {
            continue;
        }

        bool deleteExcerpt = std::find_if(parts.cbegin(), parts.cend(), [initialPartId](const Part* part) {
            return part->id() == initialPartId;
        }) != parts.cend();

        if (deleteExcerpt) {
            master->deleteExcerpt(excerpt);
        }
    }
}

std::vector<INotationPartsPtr> MasterNotationParts::excerptsParts() const
{
    std::vector<INotationPartsPtr> result;

    for (IExcerptNotationPtr excerpt : m_excerpts) {
        result.push_back(excerpt->notation()->parts());
    }

    return result;
}

mu::engraving::Excerpt* MasterNotationParts::findExcerpt(const ID& initialPartId) const
{
    const std::vector<mu::engraving::Excerpt*>& excerpts = score()->masterScore()->excerpts();
    for (mu::engraving::Excerpt* excerpt : excerpts) {
        if (excerpt->initialPartId() == initialPartId) {
            return excerpt;
        }
    }

    return nullptr;
}
