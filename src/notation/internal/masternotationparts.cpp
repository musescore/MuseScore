//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "masternotationparts.h"

using namespace mu::notation;

MasterNotationParts::MasterNotationParts(IGetScore *getScore, INotationInteractionPtr interaction, INotationUndoStackPtr undoStack)
    : NotationParts(getScore, interaction, undoStack)
{
}

void MasterNotationParts::setExcerpts(ExcerptNotationList excerpts)
{
    m_excerpts = excerpts;
}

void MasterNotationParts::setInstrumentName(const ID& instrumentId, const ID& fromPartId, const QString& name)
{
    NotationParts::setInstrumentName(instrumentId, fromPartId, name);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->setInstrumentName(instrumentId, fromPartId, name);
    }
}

void MasterNotationParts::setPartName(const ID& partId, const QString& name)
{
    NotationParts::setPartName(partId, name);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->setPartName(partId, name);
    }
}

void MasterNotationParts::setPartSharpFlat(const ID& partId, const SharpFlat& sharpFlat)
{
    NotationParts::setPartSharpFlat(partId, sharpFlat);

    for (INotationPartsPtr parts : excerptsParts())  {
        parts->setPartSharpFlat(partId, sharpFlat);
    }
}

void MasterNotationParts::setPartTransposition(const ID& partId, const instruments::Interval& transpose)
{
    NotationParts::setPartTransposition(partId, transpose);

    for (INotationPartsPtr parts : excerptsParts())  {
        parts->setPartTransposition(partId, transpose);
    }
}

void MasterNotationParts::setInstrumentAbbreviature(const ID& instrumentId, const ID& fromPartId, const QString& abbreviature)
{
    NotationParts::setInstrumentAbbreviature(instrumentId, fromPartId, abbreviature);

    for (INotationPartsPtr parts : excerptsParts())  {
        parts->setInstrumentAbbreviature(instrumentId, fromPartId, abbreviature);
    }
}

void MasterNotationParts::setStaffType(const ID& staffId, StaffType type)
{
    NotationParts::setStaffType(staffId, type);

    for (INotationPartsPtr parts : excerptsParts())  {
        parts->setStaffType(staffId, type);
    }
}

void MasterNotationParts::setCutawayEnabled(const ID& staffId, bool enabled)
{
    NotationParts::setCutawayEnabled(staffId, enabled);

    for (INotationPartsPtr parts : excerptsParts())  {
        parts->setCutawayEnabled(staffId, enabled);
    }
}

void MasterNotationParts::setSmallStaff(const ID& staffId, bool smallStaff)
{
    NotationParts::setSmallStaff(staffId, smallStaff);

    for (INotationPartsPtr parts : excerptsParts())  {
        parts->setSmallStaff(staffId, smallStaff);
    }
}

void MasterNotationParts::setStaffConfig(const ID& staffId, const StaffConfig& config)
{
    NotationParts::setStaffConfig(staffId, config);

    for (INotationPartsPtr parts : excerptsParts())  {
        parts->setStaffConfig(staffId, config);
    }
}

void MasterNotationParts::removeParts(const IDList& partsIds)
{
    NotationParts::removeParts(partsIds);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->removeParts(partsIds);
    }
}

void MasterNotationParts::removeInstruments(const IDList& instrumentsIds, const ID& fromPartId)
{
    NotationParts::removeInstruments(instrumentsIds, fromPartId);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->removeInstruments(instrumentsIds, fromPartId);
    }
}

void MasterNotationParts::removeStaves(const IDList& stavesIds)
{
    NotationParts::removeStaves(stavesIds);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->removeStaves(stavesIds);
    }
}

void MasterNotationParts::moveParts(const IDList& sourcePartsIds, const ID& destinationPartId, InsertMode mode)
{
    NotationParts::moveParts(sourcePartsIds, destinationPartId, mode);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->moveParts(sourcePartsIds, destinationPartId, mode);
    }
}

void MasterNotationParts::moveInstruments(const IDList& sourceInstrumentIds, const ID& sourcePartId, const ID& destinationPartId,
                                          const ID& destinationInstrumentId, InsertMode mode)
{
    NotationParts::moveInstruments(sourceInstrumentIds, sourcePartId, destinationPartId, destinationInstrumentId, mode);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->moveInstruments(sourceInstrumentIds, sourcePartId, destinationPartId, destinationInstrumentId, mode);
    }
}

void MasterNotationParts::moveStaves(const IDList& sourceStavesIds, const ID& destinationStaffId, InsertMode mode)
{
    NotationParts::moveStaves(sourceStavesIds, destinationStaffId, mode);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->moveStaves(sourceStavesIds, destinationStaffId, mode);
    }
}

void MasterNotationParts::appendDoublingInstrument(const instruments::Instrument& instrument, const ID& destinationPartId)
{
    NotationParts::appendDoublingInstrument(instrument, destinationPartId);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->appendDoublingInstrument(instrument, destinationPartId);
    }
}

void MasterNotationParts::appendStaff(const Staff* staff, const ID& destinationPartId)
{
    NotationParts::appendStaff(staff, destinationPartId);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->appendStaff(staff, destinationPartId);
    }
}

void MasterNotationParts::cloneStaff(const ID& sourceStaffId, const ID& destinationStaffId)
{
    NotationParts::cloneStaff(sourceStaffId, destinationStaffId);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->cloneStaff(sourceStaffId, destinationStaffId);
    }
}

void MasterNotationParts::replaceInstrument(const ID& instrumentId, const ID& fromPartId, const instruments::Instrument& newInstrument)
{
    NotationParts::replaceInstrument(instrumentId, fromPartId, newInstrument);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->replaceInstrument(instrumentId, fromPartId, newInstrument);
    }
}

std::vector<INotationPartsPtr> MasterNotationParts::excerptsParts() const
{
    std::vector<INotationPartsPtr> result;

    for (IExcerptNotationPtr excerpt : m_excerpts) {
        result.push_back(excerpt->parts());
    }

    return result;
}
