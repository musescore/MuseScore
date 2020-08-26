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
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "notationinstruments.h"

#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/stafflines.h"
#include "libmscore/undo.h"

#include "log.h"

using namespace mu::domain::notation;
using namespace mu::async;

NotationInstruments::NotationInstruments(IGetScore* getScore)
    : m_getScore(getScore)
{
}

PartList NotationInstruments::parts() const
{
    PartList result;

    PartList _parts;
    _parts << scoreParts() << excerptParts();

    QSet<QString> partIds;
    for (Part* part: _parts) {
        if (partIds.contains(part->id())) {
            continue;
        }

        result << part;

        partIds.insert(part->id());
    }

    return result;
}

InstrumentList NotationInstruments::instrumentList(const QString& partId) const
{
    Part* _part = part(partId);
    if (!_part) {
        LOGW() << "Part not found" << partId;
        return InstrumentList();
    }

    InstrumentList result;

    auto instrumentList = _part->instruments();
    for (auto it = instrumentList->begin(); it != instrumentList->end(); it++) {
        result << it->second;
    }

    return result;
}

StaffList NotationInstruments::staffList(const QString &partId, const QString &instrumentId) const
{
    Part* _part = part(partId);
    if (!_part) {
        LOGW() << "Part not found" << partId;
    }

}

void NotationInstruments::setPartVisible(const QString &partId, bool visible)
{

}

void NotationInstruments::setInstrumentVisible(const QString &partId, const QString &instrumentId, bool visible)
{
    Part* _part = part(partId);
    if (!_part) {
        LOGW() << "Part not found" << partId;
        return;
    }

    auto instrumentList = _part->instruments();
    for (auto it = instrumentList->begin(); it != instrumentList->end(); it++) {
        if (it->second->instrumentId() == instrumentId) {
            _part->undoChangeProperty(Ms::Pid::VISIBLE, visible);
        }
    }

    m_getScore->score()->masterScore()->endCmd();

    m_instrumentsChanged.notify();
}

void NotationInstruments::setStaffVisible(int staffIndex, bool visible)
{
    Staff* staff = this->staff(staffIndex);
    if (!staff) {
        return;
    }

    staff->setInvisible(!visible);

    m_getScore->score()->undo(new Ms::ChangeStaff(staff));
    m_getScore->score()->masterScore()->endCmd();

    m_instrumentsChanged.notify();
}

void NotationInstruments::setStaffType(int staffIndex, StaffType type)
{
    Staff* staff = this->staff(staffIndex);
    const Ms::StaffType* staffType = Ms::StaffType::preset(type);

    if (!staff || !staffType) {
        return;
    }

    m_getScore->score()->undo(new Ms::ChangeStaffType(staff, *staffType));
    m_getScore->score()->masterScore()->endCmd();

    m_instrumentsChanged.notify();
}

void NotationInstruments::setCutaway(int staffIndex, bool value)
{
    Staff* staff = this->staff(staffIndex);
    if (!staff) {
        return;
    }

    staff->setCutaway(value);

    m_getScore->score()->undo(new Ms::ChangeStaff(staff));
    m_getScore->score()->masterScore()->endCmd();

    m_instrumentsChanged.notify();
}

void NotationInstruments::setSmallStaff(int staffIndex, bool value)
{
    Staff* staff = this->staff(staffIndex);
    Ms::StaffType* staffType = staff->staffType(Ms::Fraction(0, 1));

    if (!staff || !staffType) {
        return;
    }

    staffType->setSmall(value);

    m_getScore->score()->undo(new Ms::ChangeStaffType(staff, *staffType));
    m_getScore->score()->masterScore()->endCmd();

    m_instrumentsChanged.notify();
}

void NotationInstruments::setVoiceVisible(int staffIndex, int voiceIndex, bool value)
{
    Staff* staff = this->staff(staffIndex);
    if (!staff) {
        return;
    }

    staff->setPlaybackVoice(voiceIndex, value);

    switch (voiceIndex) {
    case 0:
        staff->undoChangeProperty(Ms::Pid::PLAYBACK_VOICE1, value);
        break;
    case 1:
        staff->undoChangeProperty(Ms::Pid::PLAYBACK_VOICE2, value);
        break;
    case 2:
        staff->undoChangeProperty(Ms::Pid::PLAYBACK_VOICE3, value);
        break;
    case 3:
        staff->undoChangeProperty(Ms::Pid::PLAYBACK_VOICE4, value);
        break;
    }

    m_getScore->score()->masterScore()->endCmd();

    m_instrumentsChanged.notify();
}

Staff* NotationInstruments::appendLinkedStaff(int staffIndex)
{
    Staff* staff = this->staff(staffIndex);
    Part* part = staff->part();

    if (!part || !staff) {
        return nullptr;
    }

    Staff* linkedStaff = new Staff(m_getScore->score());

    linkedStaff->setPart(part);
    linkedStaff->linkTo(staff);

    int linkedStaffIndex = part->staves()->last()->idx();

    m_getScore->score()->undoInsertStaff(linkedStaff, linkedStaffIndex);
    m_getScore->score()->masterScore()->endCmd();

    m_instrumentsChanged.notify();

    return linkedStaff;
}

void NotationInstruments::removeStaff(int staffIndex)
{
    m_getScore->score()->cmdRemoveStaff(staffIndex);
    m_getScore->score()->masterScore()->endCmd();

    m_instrumentsChanged.notify();
}

void NotationInstruments::moveStaff(int fromIndex, int toIndex)
{
    Staff* staff = this->staff(fromIndex);
    if (!staff) {
        return;
    }

    m_getScore->score()->undoRemoveStaff(staff);
    m_getScore->score()->undoInsertStaff(staff, toIndex);
    m_getScore->score()->masterScore()->endCmd();

    m_instrumentsChanged.notify();
}

Notification NotationInstruments::instrumentsChanged() const
{
    return m_instrumentsChanged;
}

PartList NotationInstruments::scoreParts() const
{
    PartList result;

    for (Part* part: m_getScore->score()->parts()) {
        result << part;
    }

    return result;
}

PartList NotationInstruments::excerptParts() const
{
    if (!m_getScore->score()->isMaster()) {
        return PartList();
    }

    PartList result;

    for (Ms::Excerpt* excerpt: m_getScore->score()->excerpts()) {
        for (Part* part: excerpt->parts()) {
            result << part;
        }
    }

    return result;
}

Part* NotationInstruments::part(const QString& id) const
{
    PartList _parts;
    _parts << scoreParts() << excerptParts();
    for (Part* part: _parts) {
        if (part->id() == id) {
            return part;
        }
    }

    return nullptr;
}

Instrument *NotationInstruments::instrument(const QString &partId, const QString &instrumentId) const
{
    Part* _part = part(partId);
    if (!_part) {
        return nullptr;
    }

    auto instrumentList = _part->instruments();
    for (auto it = instrumentList->begin(); it != instrumentList->end(); it++) {
        if (it->second->instrumentId() == instrumentId) {
            return it->second;
        }
    }

    return nullptr;
}

Staff* NotationInstruments::staff(int staffIndex) const
{
    Staff* staff = m_getScore->score()->staff(staffIndex);

    if (!staff) {
        LOGW() << "Could not find staff with index:" << staffIndex;
    }

    return staff;
}
