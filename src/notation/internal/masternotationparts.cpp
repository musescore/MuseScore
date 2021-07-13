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

using namespace mu::notation;

MasterNotationParts::MasterNotationParts(IGetScore* getScore, INotationInteractionPtr interaction, INotationUndoStackPtr undoStack)
    : NotationParts(getScore, interaction, undoStack)
{
}

void MasterNotationParts::setExcerpts(ExcerptNotationList excerpts)
{
    m_excerpts = excerpts;
}

void MasterNotationParts::startEdit()
{
    undoStack()->prepareChanges();
}

void MasterNotationParts::apply()
{
    undoStack()->commitChanges();
    partsChanged().notify();
}

void MasterNotationParts::setParts(const PartInstrumentList& instruments)
{
    startEdit();
    NotationParts::setParts(instruments);
    apply();
}

void MasterNotationParts::setScoreOrder(const ScoreOrder& order)
{
    startEdit();
    NotationParts::setScoreOrder(order);
    apply();
}

void MasterNotationParts::setInstrumentName(const ID& instrumentId, const ID& fromPartId, const QString& name)
{
    startEdit();

    NotationParts::setInstrumentName(instrumentId, fromPartId, name);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->setInstrumentName(instrumentId, fromPartId, name);
    }

    apply();
}

void MasterNotationParts::setPartName(const ID& partId, const QString& name)
{
    startEdit();

    NotationParts::setPartName(partId, name);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->setPartName(partId, name);
    }

    apply();
}

void MasterNotationParts::setPartSharpFlat(const ID& partId, const SharpFlat& sharpFlat)
{
    startEdit();

    NotationParts::setPartSharpFlat(partId, sharpFlat);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->setPartSharpFlat(partId, sharpFlat);
    }

    apply();
}

void MasterNotationParts::setPartTransposition(const ID& partId, const Interval& transpose)
{
    startEdit();

    NotationParts::setPartTransposition(partId, transpose);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->setPartTransposition(partId, transpose);
    }

    apply();
}

void MasterNotationParts::setInstrumentAbbreviature(const ID& instrumentId, const ID& fromPartId, const QString& abbreviature)
{
    startEdit();

    NotationParts::setInstrumentAbbreviature(instrumentId, fromPartId, abbreviature);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->setInstrumentAbbreviature(instrumentId, fromPartId, abbreviature);
    }

    apply();
}

void MasterNotationParts::setStaffType(const ID& staffId, StaffType type)
{
    startEdit();

    NotationParts::setStaffType(staffId, type);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->setStaffType(staffId, type);
    }

    apply();
}

void MasterNotationParts::setCutawayEnabled(const ID& staffId, bool enabled)
{
    startEdit();

    NotationParts::setCutawayEnabled(staffId, enabled);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->setCutawayEnabled(staffId, enabled);
    }

    apply();
}

void MasterNotationParts::setSmallStaff(const ID& staffId, bool smallStaff)
{
    startEdit();

    NotationParts::setSmallStaff(staffId, smallStaff);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->setSmallStaff(staffId, smallStaff);
    }

    apply();
}

void MasterNotationParts::setStaffConfig(const ID& staffId, const StaffConfig& config)
{
    startEdit();

    NotationParts::setStaffConfig(staffId, config);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->setStaffConfig(staffId, config);
    }

    apply();
}

void MasterNotationParts::removeParts(const IDList& partsIds)
{
    startEdit();

    NotationParts::removeParts(partsIds);

    apply();
}

void MasterNotationParts::removeInstruments(const IDList& instrumentsIds, const ID& fromPartId)
{
    startEdit();

    NotationParts::removeInstruments(instrumentsIds, fromPartId);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->removeInstruments(instrumentsIds, fromPartId);
    }

    apply();
}

void MasterNotationParts::removeStaves(const IDList& stavesIds)
{
    startEdit();

    NotationParts::removeStaves(stavesIds);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->removeStaves(stavesIds);
    }

    apply();
}

void MasterNotationParts::moveParts(const IDList& sourcePartsIds, const ID& destinationPartId, InsertMode mode)
{
    startEdit();

    NotationParts::moveParts(sourcePartsIds, destinationPartId, mode);

    apply();
}

void MasterNotationParts::moveInstruments(const IDList& sourceInstrumentIds, const ID& sourcePartId, const ID& destinationPartId,
                                          const ID& destinationInstrumentId, InsertMode mode)
{
    startEdit();

    NotationParts::moveInstruments(sourceInstrumentIds, sourcePartId, destinationPartId, destinationInstrumentId, mode);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->moveInstruments(sourceInstrumentIds, sourcePartId, destinationPartId, destinationInstrumentId, mode);
    }

    apply();
}

void MasterNotationParts::moveStaves(const IDList& sourceStavesIds, const ID& destinationStaffId, InsertMode mode)
{
    startEdit();

    NotationParts::moveStaves(sourceStavesIds, destinationStaffId, mode);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->moveStaves(sourceStavesIds, destinationStaffId, mode);
    }

    apply();
}

void MasterNotationParts::appendDoublingInstrument(const Instrument& instrument, const ID& destinationPartId)
{
    startEdit();

    NotationParts::appendDoublingInstrument(instrument, destinationPartId);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->appendDoublingInstrument(instrument, destinationPartId);
    }

    apply();
}

void MasterNotationParts::appendStaff(Staff* staff, const ID& destinationPartId)
{
    startEdit();

    NotationParts::appendStaff(staff, destinationPartId);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->appendStaff(staff->clone(), destinationPartId);
    }

    apply();
}

void MasterNotationParts::cloneStaff(const ID& sourceStaffId, const ID& destinationStaffId)
{
    startEdit();

    NotationParts::cloneStaff(sourceStaffId, destinationStaffId);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->cloneStaff(sourceStaffId, destinationStaffId);
    }

    apply();
}

void MasterNotationParts::replaceInstrument(const ID& instrumentId, const ID& fromPartId, const Instrument& newInstrument)
{
    startEdit();

    NotationParts::replaceInstrument(instrumentId, fromPartId, newInstrument);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->replaceInstrument(instrumentId, fromPartId, newInstrument);
    }

    apply();
}

std::vector<INotationPartsPtr> MasterNotationParts::excerptsParts() const
{
    std::vector<INotationPartsPtr> result;

    for (IExcerptNotationPtr excerpt : m_excerpts) {
        result.push_back(excerpt->notation()->parts());
    }

    return result;
}
