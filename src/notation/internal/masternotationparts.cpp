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

#include "scoreorderconverter.h"

#include "libmscore/masterscore.h"
#include "libmscore/scoreorder.h"

#include "log.h"

using namespace mu::notation;

MasterNotationParts::MasterNotationParts(IGetScore* getScore, INotationInteractionPtr interaction, INotationUndoStackPtr undoStack)
    : NotationParts(getScore, interaction, undoStack)
{
}

void MasterNotationParts::setExcerpts(ExcerptNotationList excerpts)
{
    m_excerpts = excerpts;
}

Ms::MasterScore* MasterNotationParts::masterScore() const
{
    return dynamic_cast<Ms::MasterScore*>(score());
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

void MasterNotationParts::setParts(const PartInstrumentList& instruments)
{
    startEdit();
    NotationParts::setParts(instruments);
    apply();
}

void MasterNotationParts::setScoreOrder(const ScoreOrder& order)
{
    TRACEFUNC;

    startEdit();

    Ms::ScoreOrder so = ScoreOrderConverter::convertScoreOrder(order);
    masterScore()->undo(new Ms::ChangeScoreOrder(masterScore(), so));
    masterScore()->setBracketsAndBarlines();

    apply();
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

    startEdit();

    NotationParts::removeStaves(stavesIds);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->removeStaves(stavesIds);
    }

    apply();
}

void MasterNotationParts::appendStaff(Staff* staff, const ID& destinationPartId)
{
    TRACEFUNC;

    startEdit();

    NotationParts::appendStaff(staff, destinationPartId);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->appendStaff(staff->clone(), destinationPartId);
    }

    apply();
}

void MasterNotationParts::linkStaves(const ID& sourceStaffId, const ID& destinationStaffId)
{
    TRACEFUNC;

    startEdit();

    NotationParts::linkStaves(sourceStaffId, destinationStaffId);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->linkStaves(sourceStaffId, destinationStaffId);
    }

    apply();
}

void MasterNotationParts::replaceInstrument(const InstrumentKey& instrumentKey, const Instrument& newInstrument)
{
    TRACEFUNC;

    startEdit();

    NotationParts::replaceInstrument(instrumentKey, newInstrument);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->replaceInstrument(instrumentKey, newInstrument);
    }

    apply();
}

void MasterNotationParts::replaceDrumset(const InstrumentKey& instrumentKey, const Drumset& newDrumset)
{
    TRACEFUNC;

    startEdit();

    NotationParts::replaceDrumset(instrumentKey, newDrumset);

    for (INotationPartsPtr parts : excerptsParts()) {
        parts->replaceDrumset(instrumentKey, newDrumset);
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
