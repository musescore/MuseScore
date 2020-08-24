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
#include "notationparts.h"

#include "libmscore/score.h"
#include "libmscore/undo.h"
#include "libmscore/excerpt.h"
#include "libmscore/drumset.h"

#include "log.h"

using namespace mu::domain::notation;
using namespace mu::scene::instruments;

NotationParts::NotationParts(IGetScore* getScore)
    : m_getScore(getScore)
{
}

Ms::Score* NotationParts::score() const
{
    return m_getScore->score();
}

Ms::MasterScore* NotationParts::masterScore() const
{
    return score()->masterScore();
}

void NotationParts::startEdit()
{
    masterScore()->startCmd();
}

void NotationParts::apply()
{
    masterScore()->endCmd();
}

PartList NotationParts::partList() const
{
    PartList result;

    PartList parts;
    parts << scoreParts(score()) << excerptParts(score());

    QSet<QString> partIds;
    for (Part* part: parts) {
        if (partIds.contains(part->id())) {
            continue;
        }

        result << part;

        partIds.insert(part->id());
    }

    return result;
}

InstrumentList NotationParts::instrumentList(const QString& partId) const
{
    Part* part = this->part(partId);
    if (!part) {
        LOGW() << "Part not found" << partId;
        return InstrumentList();
    }

    InstrumentList result;

    auto instrumentList = part->instruments();
    for (auto it = instrumentList->begin(); it != instrumentList->end(); it++) {
        result << it->second;
    }

    return result;
}

StaffList NotationParts::staffList(const QString& partId, const QString& instrumentId) const
{
    Part* part = this->part(partId);
    if (!part) {
        LOGW() << "Part not found" << partId;
        return StaffList();
    }

    StaffList result;

    auto instrumentList = part->instruments();
    int staffGlobalIndex = 0;
    for (auto it = instrumentList->begin(); it != instrumentList->end(); it++) {
        if (it->second->instrumentId() == instrumentId) {
            for (int staffLocalIndex = 0; staffLocalIndex < it->second->nstaves(); staffLocalIndex++) {
                result << part->staff(staffGlobalIndex + staffLocalIndex);
            }
            return result;
        }
        staffGlobalIndex += it->second->nstaves();
    }
    return result;
}

void NotationParts::setInstruments(const std::vector<QString>& instrumentTemplateIds)
{
    InstrumentTemplateList instrumentTemplates = this->instrumentTemplates(instrumentTemplateIds);
    if (instrumentTemplates.empty()) {
        return;
    }

    removeUnselectedInstruments(instrumentTemplates);

    startEdit();

    std::vector<QString> missingInstrumentIds = this->missingInstrumentIds(instrumentTemplates);

    int lastGlobalStaffIndex = !score()->staves().empty() ? score()->staves().last()->idx() : 0;
    for (const InstrumentTemplate& instrumentTemplate: instrumentTemplates) {
        bool instrumentIsExists = std::find(missingInstrumentIds.begin(), missingInstrumentIds.end(),
                                            instrumentTemplate.musicXMLId) == missingInstrumentIds.end();
        if (instrumentIsExists) {
            continue;
        }

        Part* part = new Part(score());

        part->setPartName(instrumentTemplate.trackName);
        part->setInstrument(instrumentFromTemplate(instrumentTemplate));

        score()->undo(new Ms::InsertPart(part, lastGlobalStaffIndex));
        addStaves(part, instrumentTemplate, lastGlobalStaffIndex);
    }

    if (score()->measures()->size() == 0) {
        score()->insertMeasure(ElementType::MEASURE, 0, false);
    }

    cleanEmptyExcerpts();

    apply();

    m_partsChanged.notify();
}

void NotationParts::setPartVisible(const QString& partId, bool visible)
{
    Part* part = this->part(partId);
    if (!part) {
        part = this->part(partId, masterScore());
        if (!part) {
            LOGW() << "Part not found" << partId;
            return;
        }

        appendPart(part);
        m_partsChanged.notify();
        return;
    }

    startEdit();
    part->undoChangeProperty(Ms::Pid::VISIBLE, visible);
    apply();

    m_partChanged.send(part);
    m_partsChanged.notify();
}

void NotationParts::setInstrumentVisible(const QString& partId, const QString& instrumentId, bool visible)
{
    Part* part = this->part(partId);
    if (!part) {
        LOGW() << "Part not found" << partId;
        return;
    }

    startEdit();

    auto instrumentList = part->instruments();
    int staffGlobalIndex = 0;
    for (auto it = instrumentList->begin(); it != instrumentList->end(); it++) {
        if (it->second->instrumentId() == instrumentId) {
            for (int staffLocalIndex = 0; staffLocalIndex < it->second->nstaves(); staffLocalIndex++) {
                setStaffVisible(staffGlobalIndex + staffLocalIndex, visible);
            }

            apply();

            m_instrumentChanged.send(it->second);
            m_partsChanged.notify();
            return;
        }
        staffGlobalIndex += it->second->nstaves();
    }
}

void NotationParts::setStaffVisible(int staffIndex, bool visible)
{
    Staff* staff = this->staff(staffIndex);
    if (!staff) {
        return;
    }

    staff->setInvisible(!visible);

    score()->undo(new Ms::ChangeStaff(staff));
    apply();

    m_staffChanged.send(staff);
    m_partsChanged.notify();
}

void NotationParts::setStaffType(int staffIndex, StaffType type)
{
    Staff* staff = this->staff(staffIndex);
    const Ms::StaffType* staffType = Ms::StaffType::preset(type);

    if (!staff || !staffType) {
        return;
    }

    score()->undo(new Ms::ChangeStaffType(staff, *staffType));
    apply();

    m_staffChanged.send(staff);
    m_partsChanged.notify();
}

void NotationParts::setCutaway(int staffIndex, bool value)
{
    Staff* staff = this->staff(staffIndex);
    if (!staff) {
        return;
    }

    staff->setCutaway(value);

    score()->undo(new Ms::ChangeStaff(staff));
    apply();

    m_staffChanged.send(staff);
    m_partsChanged.notify();
}

void NotationParts::setSmallStaff(int staffIndex, bool value)
{
    Staff* staff = this->staff(staffIndex);
    Ms::StaffType* staffType = staff->staffType(Ms::Fraction(0, 1));

    if (!staff || !staffType) {
        return;
    }

    staffType->setSmall(value);

    score()->undo(new Ms::ChangeStaffType(staff, *staffType));
    apply();

    m_staffChanged.send(staff);
    m_partsChanged.notify();
}

void NotationParts::setVoiceVisible(int staffIndex, int voiceIndex, bool visible)
{
    Staff* staff = this->staff(staffIndex);
    if (!staff) {
        return;
    }

    staff->setPlaybackVoice(voiceIndex, visible);

    switch (voiceIndex) {
    case 0:
        staff->undoChangeProperty(Ms::Pid::PLAYBACK_VOICE1, visible);
        break;
    case 1:
        staff->undoChangeProperty(Ms::Pid::PLAYBACK_VOICE2, visible);
        break;
    case 2:
        staff->undoChangeProperty(Ms::Pid::PLAYBACK_VOICE3, visible);
        break;
    case 3:
        staff->undoChangeProperty(Ms::Pid::PLAYBACK_VOICE4, visible);
        break;
    }

    apply();

    m_staffChanged.send(staff);
    m_partsChanged.notify();
}

Staff* NotationParts::appendStaff(const QString& partId, const QString& instrumentId)
{
    Part* part = this->part(partId);
    if (!part) {
        LOGW() << "Part not found" << partId;
        return nullptr;
    }

    startEdit();

    auto instrumentList = part->instruments();
    int staffGlobalIndex = 0;
    for (auto it = instrumentList->begin(); it != instrumentList->end(); it++) {
        if (it->second->instrumentId() == instrumentId) {
            int lastStaffLocalIndex = it->second->nstaves() - 1;
            int lastStaffGlobalIndex = part->staves()->at(staffGlobalIndex + lastStaffLocalIndex)->idx();

            Staff* staff = part->staves()->at(staffGlobalIndex + lastStaffLocalIndex)->clone();
            score()->undoInsertStaff(staff, lastStaffGlobalIndex + 1);

            it->second->setClefType(staffGlobalIndex + it->second->nstaves(), staff->defaultClefType());

            apply();

            m_instrumentChanged.send(it->second);
            m_partsChanged.notify();

            return staff;
        }
        staffGlobalIndex += it->second->nstaves();
    }

    return nullptr;
}

Staff* NotationParts::appendLinkedStaff(int staffIndex)
{
    Staff* staff = this->staff(staffIndex);
    if (!staff) {
        return nullptr;
    }

    Part* part = staff->part();
    if (!part) {
        return nullptr;
    }

    Staff* linkedStaff = new Staff(score());

    linkedStaff->setPart(part);
    linkedStaff->linkTo(staff);

    int linkedStaffIndex = part->staves()->last()->idx();

    score()->undoInsertStaff(linkedStaff, linkedStaffIndex);
    apply();

    Instrument* instrument = this->instrument(linkedStaff);
    m_instrumentChanged.send(instrument);
    m_partsChanged.notify();

    return linkedStaff;
}

mu::async::Channel<Part*> NotationParts::partChanged() const
{
    return m_partChanged;
}

mu::async::Channel<Instrument*> NotationParts::instrumentChanged() const
{
    return m_instrumentChanged;
}

mu::async::Channel<Staff*> NotationParts::staffChanged() const
{
    return m_staffChanged;
}

void NotationParts::removeParts(const std::vector<QString>& partsIds)
{
    if (partsIds.empty()) {
        return;
    }

    startEdit();

    for (const QString& partId: partsIds) {
        Part* part = this->part(partId);
        score()->cmdRemovePart(part);
        m_partChanged.send(part);
    }

    apply();

    m_partsChanged.notify();
}

void NotationParts::removeInstruments(const QString& partId, const std::vector<QString>& instrumentIds)
{
    Part* part = this->part(partId);
    if (!part) {
        LOGW() << "Part not found" << partId;
        return;
    }

    startEdit();

    auto instrumentList = part->instruments();
    int staffGlobalIndex = 0;
    for (auto it = instrumentList->begin(); it != instrumentList->end(); it++) {
        if (std::find(instrumentIds.begin(), instrumentIds.end(), it->second->instrumentId()) != instrumentIds.end()) {
            if (instrumentList->size() == 0) {
                removeParts({ partId });
                break;
            }

            std::vector<int> stavesToRemove(it->second->nstaves());
            for (int staffLocalIndex = 0; staffLocalIndex < it->second->nstaves(); staffLocalIndex++) {
                stavesToRemove.push_back(staffGlobalIndex + staffLocalIndex);
            }

            removeStaves(stavesToRemove);
            part->removeInstrument(it->second->instrumentId());
        }
        staffGlobalIndex += it->second->nstaves();
    }

    apply();

    m_partChanged.send(part);
    m_partsChanged.notify();
}

void NotationParts::removeStaves(const std::vector<int>& stavesIndexes)
{
    if (stavesIndexes.empty()) {
        return;
    }

    startEdit();

    for (int staffIndex: stavesIndexes) {
        Staff* staff = this->staff(staffIndex);
        Instrument* instrument = this->instrument(staff);

        score()->cmdRemoveStaff(staffIndex);
        m_instrumentChanged.send(instrument);
    }

    apply();

    m_partsChanged.notify();
}

void NotationParts::movePart(const QString& partId, const QString& beforePartId)
{
    Part* part = this->part(partId);
    if (!part) {
        return;
    }

    startEdit();

    StaffList staves;
    for (const Staff* staff: *part->staves()) {
        staves.push_back(staff->clone());
    }

    score()->cmdRemovePart(part);

    Part* beforePart = this->part(beforePartId);
    if (!beforePart) {
        return;
    }

    int newFirstStaffIndex = score()->staffIdx(beforePart);
    score()->undoInsertPart(part, newFirstStaffIndex);

    for (int staffIndex = 0; staffIndex < staves.size(); ++staffIndex) {
        Staff* staff = staves[staffIndex];
        score()->undoInsertStaff(staff, staffIndex);
    }

    apply();

    m_partChanged.send(part);
    m_partChanged.send(beforePart);
    m_partsChanged.notify();
}

void NotationParts::moveInstrument(const QString& instrumentId, const QString& fromPartId, const QString& toPartId,
                                   const QString& beforeInstrumentId)
{
    Part* fromPart = part(fromPartId);
    Part* toPart = part(toPartId);

    if (!fromPart || !toPart) {
        return;
    }

    Instrument* instrument = this->instrument(fromPart, instrumentId);
    Instrument* newInstrument = new Instrument(*instrument);
    StaffList staves = staffList(fromPartId, instrument->instrumentId());

    startEdit();
    removeInstruments(fromPartId, { instrument->instrumentId() });
    insertInstrument(toPart, newInstrument, staves, beforeInstrumentId);
    apply();

    m_partChanged.send(fromPart);
    m_partChanged.send(toPart);
    m_partsChanged.notify();
}

void NotationParts::moveStaff(int staffIndex, int beforeStaffIndex)
{
    Staff* staff = this->staff(staffIndex);
    if (!staff) {
        return;
    }

    Instrument* fromInstrument = this->instrument(staff);

    startEdit();
    score()->undoRemoveStaff(staff);
    score()->undoInsertStaff(staff, beforeStaffIndex - 1);
    apply();

    Instrument* toInstrument = this->instrument(staff);

    m_instrumentChanged.send(fromInstrument);
    m_instrumentChanged.send(toInstrument);
    m_partsChanged.notify();
}

mu::async::Notification NotationParts::partsChanged() const
{
    return m_partsChanged;
}

PartList NotationParts::scoreParts(const Ms::Score* score) const
{
    PartList result;

    for (Part* part: score->parts()) {
        result << part;
    }

    return result;
}

PartList NotationParts::excerptParts(const Ms::Score* score) const
{
    if (!score->isMaster()) {
        return PartList();
    }

    PartList result;

    for (const Ms::Excerpt* excerpt: score->excerpts()) {
        for (Part* part: excerpt->parts()) {
            result << part;
        }
    }

    return result;
}

Part* NotationParts::part(const QString& partId, const Ms::Score* score) const
{
    if (!score) {
        score = this->score();
    }

    PartList _parts;
    _parts << scoreParts(score) << excerptParts(score);
    for (Part* part: _parts) {
        if (part->id() == partId) {
            return part;
        }
    }

    return nullptr;
}

Instrument* NotationParts::instrument(const Part* part, const QString& instrumentId) const
{
    auto instrumentList = part->instruments();
    for (auto it = instrumentList->begin(); it != instrumentList->end(); it++) {
        if (it->second->instrumentId() == instrumentId) {
            return it->second;
        }
    }

    return nullptr;
}

Instrument* NotationParts::instrument(const Staff* staff) const
{
    if (!staff) {
        return nullptr;
    }

    Part* part = staff->part();
    if (!part) {
        return nullptr;
    }

    auto instrumentList = part->instruments();
    int staffGlobalIndex = 0;
    for (auto it = instrumentList->begin(); it != instrumentList->end(); it++) {
        for (int staffLocalIndex = 0; staffLocalIndex < it->second->nstaves(); staffLocalIndex++) {
            if (part->staff(staffGlobalIndex + staffLocalIndex)->idx() == staff->idx()) {
                return it->second;
            }
        }
        staffGlobalIndex += it->second->nstaves();
    }

    return nullptr;
}

Staff* NotationParts::staff(int staffIndex) const
{
    Staff* staff = score()->staff(staffIndex);

    if (!staff) {
        LOGW() << "Could not find staff with index:" << staffIndex;
    }

    return staff;
}

InstrumentTemplateList NotationParts::instrumentTemplates(const std::vector<QString>& instrumentTemplateIds) const
{
    InstrumentTemplateList instrumentTemplates;

    InstrumentTemplateHash templates = instrumentsRepository()->instrumentsMeta().val.instrumentTemplates;

    for (const QString& templateId: instrumentTemplateIds) {
        if (!templates.contains(templateId)) {
            LOGW() << "Template not found" << templateId;
            continue;
        }

        instrumentTemplates << templates[templateId];
    }

    return instrumentTemplates;
}

void NotationParts::appendPart(Part* part)
{
    for (Staff* partStaff: *part->staves()) {
        Staff* staff = new Staff(score());
        staff->setPart(part);
        staff->init(partStaff);
        if (partStaff->links() && !part->staves()->isEmpty()) {
            Staff* linkedStaff = part->staves()->back();
            staff->linkTo(linkedStaff);
        }
        part->insertStaff(staff, -1);
        score()->staves().append(staff);
    }

    score()->appendPart(part);
}

void NotationParts::addStaves(Part* part, const InstrumentTemplate& instrumentTemplate, int& globalStaffIndex)
{
    for (int i = 0; i < instrumentTemplate.staves; i++) {
        Staff* staff = new Staff(score());
        staff->setPart(part);
        initStaff(staff, instrumentTemplate, Ms::StaffType::preset(StaffType(0)), i);

        if (globalStaffIndex > 0) {
            staff->setBarLineSpan(score()->staff(globalStaffIndex - 1)->barLineSpan());
        }

        score()->undoInsertStaff(staff, i);
        globalStaffIndex++;
    }
}

void NotationParts::insertInstrument(Part* part, Instrument* instrument, const StaffList& staves, const QString& beforeInstrumentId)
{
    part->setInstrument(instrument); // todo: insert by index

    int lastStaffIndex = part->staves()->first()->idx();
    for (int i = 0; i < staves.size(); i++) {
        score()->undoInsertStaff(staves[i], lastStaffIndex + i);
    }
}

void NotationParts::removeUnselectedInstruments(const InstrumentTemplateList& instrumentTemplates)
{
    PartList parts = this->partList();
    if (parts.isEmpty()) {
        return;
    }

    std::vector<QString> partsToRemove;
    for (const Part* part: parts) {
        std::vector<QString> instrumentsToRemove;
        auto instrumentList = part->instruments();
        for (auto it = instrumentList->begin(); it != instrumentList->end(); it++) {
            if (!templatesContainsInstrument(instrumentTemplates, it->second->instrumentId())) {
                instrumentsToRemove.push_back(it->second->instrumentId());
            }
        }

        bool removeAllInstruments = instrumentsToRemove.size() == part->instruments()->size();
        if (removeAllInstruments) {
            partsToRemove.push_back(part->id());
        } else {
            removeInstruments(part->id(), instrumentsToRemove);
        }
    }

    if (!partsToRemove.empty()) {
        removeParts(partsToRemove);
    }
}

bool NotationParts::templatesContainsInstrument(const InstrumentTemplateList& instrumentTemplates, const QString& instrumentId) const
{
    for (const InstrumentTemplate& instrumentTemplate: instrumentTemplates) {
        if (instrumentTemplate.musicXMLId == instrumentId) {
            return true;
        }
    }
    return false;
}

std::vector<QString> NotationParts::missingInstrumentIds(const InstrumentTemplateList& instrumentTemplates) const
{
    PartList parts = this->partList();
    if (parts.isEmpty()) {
        return {};
    }

    std::vector<QString> missingInstrumentIds;
    for (const InstrumentTemplate& instrumentTemplate: instrumentTemplates) {
        missingInstrumentIds.push_back(instrumentTemplate.musicXMLId);
    }

    for (const Part* part: parts) {
        auto instrumentList = part->instruments();
        for (auto it = instrumentList->begin(); it != instrumentList->end(); it++) {
            missingInstrumentIds.erase(std::remove(missingInstrumentIds.begin(), missingInstrumentIds.end(),
                                                   it->second->instrumentId()), missingInstrumentIds.end());
        }
    }

    return missingInstrumentIds;
}

void NotationParts::cleanEmptyExcerpts()
{
    const QList<Ms::Excerpt*> excerpts(masterScore()->excerpts());
    for (Ms::Excerpt* excerpt: excerpts) {
        QList<Staff*> staves = excerpt->partScore()->staves();
        if (staves.size() == 0) {
            masterScore()->undo(new Ms::RemoveExcerpt(excerpt));
        }
    }
}

Instrument NotationParts::instrumentFromTemplate(const InstrumentTemplate& instrumentTemplate) const
{
    Instrument instrument;
    instrument.setAmateurPitchRange(instrumentTemplate.aPitchRange.min, instrumentTemplate.aPitchRange.max);
    instrument.setProfessionalPitchRange(instrumentTemplate.pPitchRange.min, instrumentTemplate.pPitchRange.max);
    for (Ms::StaffName sn: instrumentTemplate.longNames) {
        instrument.addLongName(StaffName(sn.name(), sn.pos()));
    }
    for (Ms::StaffName sn: instrumentTemplate.shortNames) {
        instrument.addShortName(StaffName(sn.name(), sn.pos()));
    }
    instrument.setTrackName(instrumentTemplate.trackName);
    instrument.setTranspose(instrumentTemplate.transpose);
    instrument.setInstrumentId(instrumentTemplate.musicXMLId);
    if (instrumentTemplate.useDrumset) {
        instrument.setDrumset(instrumentTemplate.drumset ? instrumentTemplate.drumset : Ms::smDrumset);
    }
    for (int i = 0; i < instrumentTemplate.staves; ++i) {
        instrument.setClefType(i, instrumentTemplate.clefs[i]);
    }
    instrument.setMidiActions(convertedMidiActions(instrumentTemplate.midiActions));
    instrument.setArticulation(instrumentTemplate.midiArticulations);
    for (const Channel& c : instrumentTemplate.channels) {
        instrument.appendChannel(new Channel(c));
    }
    instrument.setStringData(instrumentTemplate.stringData);
    instrument.setSingleNoteDynamics(instrumentTemplate.singleNoteDynamics);
    return instrument;
}

void NotationParts::initStaff(Staff* staff, const InstrumentTemplate& instrumentTemplate, const Ms::StaffType* staffType, int cidx)
{
    const Ms::StaffType* pst = staffType ? staffType : instrumentTemplate.staffTypePreset;
    if (!pst) {
        pst = Ms::StaffType::getDefaultPreset(instrumentTemplate.staffGroup);
    }

    Ms::StaffType* stt = staff->setStaffType(Ms::Fraction(0, 1), *pst);
    if (cidx >= MAX_STAVES) {
        stt->setSmall(false);
    } else {
        stt->setSmall(instrumentTemplate.smallStaff[cidx]);
        staff->setBracketType(0, instrumentTemplate.bracket[cidx]);
        staff->setBracketSpan(0, instrumentTemplate.bracketSpan[cidx]);
        staff->setBarLineSpan(instrumentTemplate.barlineSpan[cidx]);
    }
    staff->setDefaultClefType(instrumentTemplate.clefs[cidx]);
}

QList<Ms::NamedEventList> NotationParts::convertedMidiActions(const MidiActionList& templateMidiActions) const
{
    QList<Ms::NamedEventList> result;

    for (const MidiAction& action: templateMidiActions) {
        Ms::NamedEventList event;
        event.name = action.name;
        event.descr = action.description;

        for (const midi::Event& midiEvent: action.events) {
            Ms::MidiCoreEvent midiCoreEvent;
            midiCoreEvent.setType(static_cast<uchar>(midiEvent.type));
            midiCoreEvent.setChannel(midiCoreEvent.channel());
            midiCoreEvent.setData(midiEvent.a, midiEvent.b);
            event.events.push_back(midiCoreEvent);
        }
    }

    return result;
}
