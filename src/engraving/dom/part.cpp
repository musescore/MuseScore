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

#include "part.h"

#include "containers.h"
#include "style/style.h"
#include "rw/xmlwriter.h"

#include "chordrest.h"
#include "factory.h"
#include "fret.h"
#include "harppedaldiagram.h"
#include "instrtemplate.h"
#include "instrchange.h"
#include "linkedobjects.h"
#include "masterscore.h"
#include "measure.h"
#include "score.h"
#include "staff.h"
#include "stringtunings.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
const Fraction Part::MAIN_INSTRUMENT_TICK = Fraction(-1, 1);
//---------------------------------------------------------
//   Part
//---------------------------------------------------------

Part::Part(Score* s)
    : EngravingObject(ElementType::PART, s)
{
    m_color   = DEFAULT_COLOR;
    m_show    = true;
    m_soloist = false;
    m_instruments.setInstrument(new Instrument, -1);     // default instrument
    m_preferSharpFlat = PreferSharpFlat::AUTO;
}

//---------------------------------------------------------
//   initFromInstrTemplate
//---------------------------------------------------------

void Part::initFromInstrTemplate(const InstrumentTemplate* t)
{
    m_partName = !t->longNames.empty() ? t->longNames.front().name() : t->trackName;
    setInstrument(Instrument::fromTemplate(t));
}

const ID& Part::id() const
{
    return m_id;
}

void Part::setId(const ID& id)
{
    m_id = id;
}

Part* Part::clone() const
{
    return new Part(*this);
}

//---------------------------------------------------------
//   staff
//---------------------------------------------------------

Staff* Part::staff(staff_idx_t idx) const
{
    if (idx >= m_staves.size()) {
        return nullptr;
    }

    return m_staves[idx];
}

//---------------------------------------------------------
//   family
//---------------------------------------------------------

String Part::familyId() const
{
    if (m_instruments.empty()) {
        return String();
    }

    InstrumentIndex ii = searchTemplateIndexForId(instrumentId());
    return ii.instrTemplate && ii.instrTemplate->family ? ii.instrTemplate->family->id : String();
}

//---------------------------------------------------------
//   Part::masterPart
//---------------------------------------------------------

const Part* Part::masterPart() const
{
    if (score()->isMaster()) {
        return this;
    }
    if (m_staves.empty()) {
        return this;
    }

    Staff* st = m_staves[0];
    LinkedObjects* links = st->links();
    if (!links) {
        return this;
    }

    for (EngravingObject* le : *links) {
        if (le->isStaff() && toStaff(le)->score()->isMaster()) {
            if (Part* p = toStaff(le)->part()) {
                return p;
            }
        }
    }
    return this;
}

//---------------------------------------------------------
//   Part::masterPart
//---------------------------------------------------------

Part* Part::masterPart()
{
    return const_cast<Part*>(const_cast<const Part*>(this)->masterPart());
}

size_t Part::nstaves() const
{
    return m_staves.size();
}

const std::vector<Staff*>& Part::staves() const
{
    return m_staves;
}

std::set<staff_idx_t> Part::staveIdxList() const
{
    std::set<staff_idx_t> result;

    for (const Staff* stave : m_staves) {
        if (!stave) {
            continue;
        }

        result.insert(stave->idx());
    }

    return result;
}

void Part::appendStaff(Staff* staff)
{
    m_staves.push_back(staff);
}

void Part::clearStaves()
{
    m_staves.clear();
}

//---------------------------------------------------------
//   setLongNames
//---------------------------------------------------------

void Part::setLongNames(std::list<StaffName>& name, const Fraction& tick)
{
    instrument(tick)->setLongNames(StaffNameList(name));
}

void Part::setShortNames(std::list<StaffName>& name, const Fraction& tick)
{
    instrument(tick)->setShortNames(StaffNameList(name));
}

//---------------------------------------------------------
//   setStaves
//---------------------------------------------------------

void Part::setStaves(int n)
{
    int ns = static_cast<int>(m_staves.size());
    if (n < ns) {
        LOGD("Part::setStaves(): remove staves not implemented!");
        return;
    }

    int staffIdx = static_cast<int>(score()->staffIdx(this)) + ns;
    for (int i = ns; i < n; ++i) {
        Staff* staff = Factory::createStaff(this);
        score()->insertStaff(staff, i);

        for (Measure* m = score()->firstMeasure(); m; m = m->nextMeasure()) {
            m->insertStaff(staff, staffIdx);
            if (m->hasMMRest()) {
                m->mmRest()->insertStaff(staff, staffIdx);
            }
        }
        ++staffIdx;
    }
}

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Part::insertStaff(Staff* staff, staff_idx_t idx)
{
    if (idx >= m_staves.size()) {
        idx = m_staves.size();
    }
    m_staves.insert(m_staves.begin() + idx, staff);
    staff->setPart(this);
}

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Part::removeStaff(Staff* staff)
{
    if (!muse::remove(m_staves, staff)) {
        LOGD("Part::removeStaff: not found %p", staff);
        return;
    }
}

//---------------------------------------------------------
//   setMidiProgram
//---------------------------------------------------------

void Part::setMidiProgram(int program, int bank)
{
    InstrChannel* c = instrument()->channel(0);
    c->setProgram(program);
    c->setBank(bank);
}

//---------------------------------------------------------
//   midiProgram
//---------------------------------------------------------

int Part::midiProgram() const
{
    return instrument()->playbackChannel(0, masterScore())->program();
}

//---------------------------------------------------------
//   capoFret
//---------------------------------------------------------
int Part::capoFret() const
{
    return m_capoFret;
}

//---------------------------------------------------------
//   setCapoFret
//---------------------------------------------------------
void Part::setCapoFret(int capoFret)
{
    m_capoFret = capoFret;
}

//---------------------------------------------------------
//   midiChannel
//---------------------------------------------------------

int Part::midiChannel() const
{
    return masterScore()->midiChannel(instrument()->channel(0)->channel());
}

//---------------------------------------------------------
//   midiPort
//---------------------------------------------------------

int Part::midiPort() const
{
    return masterScore()->midiPort(instrument()->channel(0)->channel());
}

//---------------------------------------------------------
//   setMidiChannel
//   Called from importmusicxml, importMidi and importGtp*.
//   Specify tick to set MIDI channel to an InstrumentChange element.
//   Usage:
//   setMidiChannel(channel)       to set channel
//   setMidiChannel(-1, port)      to set port
//   setMidiChannel(channel, port) to set both
//---------------------------------------------------------

void Part::setMidiChannel(int ch, int port, const Fraction& tick)
{
    InstrChannel* channel = instrument(tick)->channel(0);
    if (channel->channel() == -1) {
        masterScore()->addMidiMapping(channel, this, port, ch);
    } else {
        masterScore()->updateMidiMapping(channel, this, port, ch);
    }
}

//---------------------------------------------------------
//   setInstrument
//---------------------------------------------------------

void Part::setInstrument(Instrument* i, Fraction tick)
{
    m_instruments.setInstrument(i, tick.ticks());
}

void Part::setInstrument(Instrument* i, int tick)
{
    m_instruments.setInstrument(i, tick);
}

void Part::setInstrument(const Instrument&& i, Fraction tick)
{
    m_instruments.setInstrument(new Instrument(i), tick.ticks());
}

void Part::setInstrument(const Instrument& i, Fraction tick)
{
    m_instruments.setInstrument(new Instrument(i), tick.ticks());
}

void Part::setInstruments(const InstrumentList& instruments)
{
    m_instruments.clear();

    for (auto it = instruments.begin(); it != instruments.end(); ++it) {
        m_instruments.setInstrument(it->second, it->first);
    }
}

//---------------------------------------------------------
//   removeInstrument
//---------------------------------------------------------

void Part::removeInstrument(const Fraction& tick)
{
    auto i = m_instruments.find(tick.ticks());
    if (i == m_instruments.end()) {
        LOGD("Part::removeInstrument: not found at tick %d", tick.ticks());
        return;
    }
    m_instruments.erase(i);
}

//---------------------------------------------------------
//   removeNonPrimaryInstruments
//---------------------------------------------------------

void Part::removeNonPrimaryInstruments()
{
    auto it = m_instruments.begin();
    while (it != m_instruments.end()) {
        if (it->first != -1) {
            it = m_instruments.erase(it);
            continue;
        }
        ++it;
    }
}

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

Instrument* Part::instrument(Fraction tick)
{
    return m_instruments.instrument(tick.ticks());
}

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

const Instrument* Part::instrument(Fraction tick) const
{
    return m_instruments.instrument(tick.ticks());
}

const Instrument* Part::instrumentById(const String& id) const
{
    for (const auto& pair: m_instruments) {
        if (pair.second->id() == id) {
            return pair.second;
        }
    }

    return nullptr;
}

//---------------------------------------------------------
//   instruments
//---------------------------------------------------------

const InstrumentList& Part::instruments() const
{
    return m_instruments;
}

const StringData* Part::stringData(const Fraction& tick, staff_idx_t staffIdx) const
{
    if (!score()) {
        return nullptr;
    }

    const Instrument* instrument = this->instrument(tick);
    if (!instrument) {
        return nullptr;
    }

    bool reflectTranspositionInLinkedTab = true;

    const Staff* staff = staffIdx != muse::nidx ? score()->staff(staffIdx) : nullptr;
    if (staff && staff->isTabStaff(tick)) {
        if (const Staff* primaryStaff = staff->primaryStaff()) {
            reflectTranspositionInLinkedTab = primaryStaff->reflectTranspositionInLinkedTab();
        }
    }

    StringTunings* stringTunings = nullptr;

    if (reflectTranspositionInLinkedTab) {
        auto it = muse::findLessOrEqual(m_stringTunings, tick.ticks());
        if (it != m_stringTunings.end()) {
            stringTunings = it->second;
        }
    }

    if (stringTunings) {
        //!NOTE: if there is string tunings element between current instrument and current tick,
        //! then return string data from string tunings element
        const Instrument* stringTuningsInstrument = this->instrument(stringTunings->tick());
        if (instrument == stringTuningsInstrument) {
            return stringTunings->stringData();
        }
    }

    return instrument->stringData();
}

void Part::addStringTunings(StringTunings* stringTunings)
{
    m_stringTunings[stringTunings->segment()->tick().ticks()] = stringTunings;
}

void Part::removeStringTunings(StringTunings* stringTunings)
{
    if (m_stringTunings[stringTunings->segment()->tick().ticks()] == stringTunings) {
        m_stringTunings.erase(stringTunings->segment()->tick().ticks());
    }
}

//---------------------------------------------------------
//   instrumentId
//---------------------------------------------------------

String Part::instrumentId(const Fraction& tick) const
{
    return instrument(tick)->id();
}

//---------------------------------------------------------
//   longName
//---------------------------------------------------------

String Part::longName(const Fraction& tick) const
{
    const std::list<StaffName>& nl = longNames(tick);
    return nl.empty() ? u"" : nl.front().name();
}

//---------------------------------------------------------
//   instrumentName
//---------------------------------------------------------

String Part::instrumentName(const Fraction& tick) const
{
    return instrument(tick)->trackName();
}

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

String Part::shortName(const Fraction& tick) const
{
    const std::list<StaffName>& nl = shortNames(tick);
    return nl.empty() ? u"" : nl.front().name();
}

//---------------------------------------------------------
//   setLongName
//---------------------------------------------------------

void Part::setLongName(const String& s)
{
    instrument()->setLongName(s);
}

//---------------------------------------------------------
//   setShortName
//---------------------------------------------------------

void Part::setShortName(const String& s)
{
    instrument()->setShortName(s);
}

//---------------------------------------------------------
//   setLongNameAll
//---------------------------------------------------------

void Part::setLongNameAll(const String& s)
{
    for (auto instrument : m_instruments) {
        instrument.second->setLongName(s);
    }
}

//---------------------------------------------------------
//   setShortNameAll
//---------------------------------------------------------

void Part::setShortNameAll(const String& s)
{
    for (auto instrument : m_instruments) {
        instrument.second->setShortName(s);
    }
}

//---------------------------------------------------------
//   setPlainLongName
//---------------------------------------------------------

void Part::setPlainLongName(const String& s)
{
    setLongName(XmlWriter::xmlString(s));
}

//---------------------------------------------------------
//   setPlainShortName
//---------------------------------------------------------

void Part::setPlainShortName(const String& s)
{
    setShortName(XmlWriter::xmlString(s));
}

//---------------------------------------------------------
//   setPlainLongNameAll
//---------------------------------------------------------

void Part::setPlainLongNameAll(const String& s)
{
    setLongNameAll(XmlWriter::xmlString(s));
}

//---------------------------------------------------------
//   setPlainShortNameAll
//---------------------------------------------------------

void Part::setPlainShortNameAll(const String& s)
{
    setShortNameAll(XmlWriter::xmlString(s));
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Part::getProperty(Pid id) const
{
    switch (id) {
    case Pid::VISIBLE:
        return PropertyValue(m_show);
    case Pid::HIDE_WHEN_EMPTY:
        return PropertyValue(m_hideWhenEmpty);
    case Pid::USE_DRUMSET:
        return instrument()->useDrumset();
    case Pid::PREFER_SHARP_FLAT:
        return int(preferSharpFlat());
    default:
        return PropertyValue();
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Part::setProperty(Pid id, const PropertyValue& property)
{
    switch (id) {
    case Pid::VISIBLE:
        setShow(property.toBool());
        break;
    case Pid::HIDE_WHEN_EMPTY:
        setHideWhenEmpty(property.value<AutoOnOff>());
        break;
    case Pid::USE_DRUMSET:
        instrument()->setUseDrumset(property.toBool());
        break;
    case Pid::PREFER_SHARP_FLAT:
        setPreferSharpFlat(PreferSharpFlat(property.toInt()));
        break;
    default:
        LOGD("Part::setProperty: unknown id %d", int(id));
        break;
    }
    score()->setLayoutAll();
    return true;
}

//---------------------------------------------------------
//   startTrack
//---------------------------------------------------------

track_idx_t Part::startTrack() const
{
    IF_ASSERT_FAILED(!m_staves.empty()) {
        return muse::nidx;
    }

    return m_staves.front()->idx() * VOICES;
}

//---------------------------------------------------------
//   endTrack
//---------------------------------------------------------

track_idx_t Part::endTrack() const
{
    IF_ASSERT_FAILED(!m_staves.empty()) {
        return muse::nidx;
    }

    return m_staves.back()->idx() * VOICES + VOICES;
}

InstrumentTrackIdList Part::instrumentTrackIdList() const
{
    InstrumentTrackIdList result;
    std::set<String> seen;

    for (const auto& pair : m_instruments) {
        String instrId = pair.second->id();
        if (seen.insert(instrId).second) {
            result.push_back({ m_id, instrId });
        }
    }

    return result;
}

InstrumentTrackIdSet Part::instrumentTrackIdSet() const
{
    InstrumentTrackIdSet result;

    for (const auto& pair : m_instruments) {
        result.insert({ m_id, pair.second->id() });
    }

    return result;
}

//---------------------------------------------------------
//   insertTime
//---------------------------------------------------------

void Part::insertTime(const Fraction& tick, const Fraction& len)
{
    if (len.isZero()) {
        return;
    }

    // move all instruments

    if (len < Fraction(0, 1)) {
        // remove instruments between tickpos >= tick and tickpos < (tick+len)
        // ownership goes back to class InstrumentChange()

        auto si = m_instruments.lower_bound(tick.ticks());
        auto ei = m_instruments.lower_bound((tick - len).ticks());
        m_instruments.erase(si, ei);

        // remove harp pedal diagrams between tickpo >= tick
        harpDiagrams.erase(harpDiagrams.lower_bound(tick.ticks()), harpDiagrams.lower_bound((tick - len).ticks()));
    }

    InstrumentList il;

    for (auto i = m_instruments.lower_bound(tick.ticks()); i != m_instruments.end();) {
        Instrument* instrument = i->second;
        int t = i->first;
        m_instruments.erase(i++);
        il[t + len.ticks()] = instrument;
    }
    m_instruments.insert(il.begin(), il.end());

    std::map<int, HarpPedalDiagram*> hd2;
    for (auto h = harpDiagrams.lower_bound(tick.ticks()); h != harpDiagrams.end();) {
        HarpPedalDiagram* diagram = h->second;
        int t = h->first;
        harpDiagrams.erase(h++);
        hd2[t + len.ticks()] = diagram;
    }
    harpDiagrams.insert(hd2.begin(), hd2.end());
}

//---------------------------------------------------------
//   addHarpDiagram
//---------------------------------------------------------

void Part::addHarpDiagram(HarpPedalDiagram* harpDiagram)
{
    harpDiagrams[harpDiagram->segment()->tick().ticks()] = harpDiagram;
}

//---------------------------------------------------------
//   removeHarpDiagram
//---------------------------------------------------------

void Part::removeHarpDiagram(HarpPedalDiagram* harpDiagram)
{
    if (harpDiagrams[harpDiagram->segment()->tick().ticks()] == harpDiagram) {
        harpDiagrams.erase(harpDiagram->segment()->tick().ticks());
    }
}

//---------------------------------------------------------
//   clearHarpDiagrams
//---------------------------------------------------------

void Part::clearHarpDiagrams()
{
    harpDiagrams.clear();
}

//---------------------------------------------------------
//   currentHarpDiagram
//---------------------------------------------------------

HarpPedalDiagram* Part::currentHarpDiagram(const Fraction& tick) const
{
    auto i = harpDiagrams.upper_bound(tick.ticks());
    if (i != harpDiagrams.begin()) {
        --i;
    }
    if (i == harpDiagrams.end()) {
        return nullptr;
    } else if (tick < Fraction::fromTicks(i->first)) {
        return nullptr;
    }
    return i->second;
}

//---------------------------------------------------------
//   nextHarpDiagram
//---------------------------------------------------------

HarpPedalDiagram* Part::nextHarpDiagram(const Fraction& tick) const
{
    auto i = harpDiagrams.upper_bound(tick.ticks());
    return (i == harpDiagrams.end()) ? nullptr : i->second;
}

//---------------------------------------------------------
//   prevHarpDiagram
//---------------------------------------------------------

HarpPedalDiagram* Part::prevHarpDiagram(const Fraction& tick) const
{
    auto i = harpDiagrams.lower_bound(tick.ticks());
    if (i == harpDiagrams.begin()) {
        return nullptr;
    }
    i--;
    return i->second;
}

//---------------------------------------------------------
//   currentHarpDiagramTick
//---------------------------------------------------------

Fraction Part::currentHarpDiagramTick(const Fraction& tick) const
{
    if (harpDiagrams.empty()) {
        return Fraction(0, 1);
    }
    auto i = harpDiagrams.upper_bound(tick.ticks());
    if (i == harpDiagrams.begin()) {
        return Fraction(0, 1);
    }
    --i;
    return Fraction::fromTicks(i->first);
}

bool Part::isVisible() const
{
    return m_show;
}

//---------------------------------------------------------
//   lyricCount
//---------------------------------------------------------

int Part::lyricCount() const
{
    if (!score()) {
        return 0;
    }

    if (!score()->firstMeasure()) {
        return 0;
    }

    size_t count = 0;
    SegmentType st = SegmentType::ChordRest;
    for (Segment* seg = score()->firstMeasure()->first(st); seg; seg = seg->next1(st)) {
        for (track_idx_t i = startTrack(); i < endTrack(); ++i) {
            ChordRest* cr = toChordRest(seg->element(i));
            if (cr) {
                count += cr->lyrics().size();
            }
        }
    }
    return int(count);
}

//---------------------------------------------------------
//   harmonyCount
//---------------------------------------------------------

int Part::harmonyCount() const
{
    if (!score()) {
        return 0;
    }

    Measure* firstM = score()->firstMeasure();
    if (!firstM) {
        return 0;
    }

    SegmentType st = SegmentType::ChordRest;
    int count = 0;
    for (const Segment* seg = firstM->first(st); seg; seg = seg->next1(st)) {
        for (const EngravingItem* e : seg->annotations()) {
            if ((e->isHarmony() || (e->isFretDiagram() && toFretDiagram(e)->harmony())) && e->track() >= startTrack()
                && e->track() < endTrack()) {
                count++;
            }
        }
    }
    return count;
}

//---------------------------------------------------------
//   updateHarmonyChannels
///   update the harmony channel by creating a new channel
///   when appropriate or using the existing one
///
///   checkRemoval can be set to true to check to see if we
///   can remove the harmony channel
//---------------------------------------------------------
void Part::updateHarmonyChannels(bool isDoOnInstrumentChanged, bool checkRemoval)
{
    auto onInstrumentChanged = [this]() {
        masterScore()->rebuildMidiMapping();
        masterScore()->updateChannel();
        score()->setInstrumentsChanged(true);
        score()->setLayoutAll();     //do we need this?
    };

    // usage of harmony count is okay even if expensive since checking harmony channel will shortcircuit if existent
    // harmonyCount will only be called on loading of a score (where it will need to be scanned for harmony anyway)
    // or when the first harmony of a score is just added
    if (checkRemoval) {
        //may be a bit expensive since it gets called after every single delete or undo, but it should be okay for now
        //~OPTIM~
        if (harmonyCount() == 0) {
            Instrument* instr = instrument();
            int hChIdx = instr->channelIdx(String::fromUtf8(InstrChannel::HARMONY_NAME));
            if (hChIdx != -1) {
                InstrChannel* hChan = instr->channel(hChIdx);
                instr->removeChannel(hChan);
                delete hChan;
                if (isDoOnInstrumentChanged) {
                    onInstrumentChanged();
                }
                return;
            }
        }
    }

    if (!harmonyChannel() && harmonyCount() > 0) {
        Instrument* instr = instrument();
        InstrChannel* c = new InstrChannel(*instr->channel(0));
        // default to program 0, which is piano in General MIDI
        c->setProgram(0);
        if (c->bank() == 128) { // drumset?
            c->setBank(0);
        }
        c->setName(String::fromUtf8(InstrChannel::HARMONY_NAME));
        instr->appendChannel(c);
        onInstrumentChanged();
    }
}

//---------------------------------------------------------
//   harmonyChannel
//---------------------------------------------------------

const InstrChannel* Part::harmonyChannel() const
{
    const Instrument* instr = instrument();
    if (!instr) {
        return nullptr;
    }

    int chanIdx = instr->channelIdx(String::fromUtf8(InstrChannel::HARMONY_NAME));
    if (chanIdx == -1) {
        return nullptr;
    }

    const InstrChannel* chan = instr->channel(chanIdx);
    assert(chan);
    return chan;
}

bool Part::hasChordSymbol() const
{
    return harmonyChannel() != nullptr;
}

//---------------------------------------------------------
//   hasPitchedStaff
//---------------------------------------------------------

bool Part::hasPitchedStaff() const
{
    for (Staff* s : staves()) {
        if (s && s->isPitchedStaff(Fraction(0, 1))) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   hasTabStaff
//---------------------------------------------------------

bool Part::hasTabStaff() const
{
    for (Staff* s : staves()) {
        if (s && s->isTabStaff(Fraction(0, 1))) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   hasDrumStaff
//---------------------------------------------------------

bool Part::hasDrumStaff() const
{
    for (Staff* s : staves()) {
        if (s && s->isDrumStaff(Fraction(0, 1))) {
            return true;
        }
    }
    return false;
}
}
