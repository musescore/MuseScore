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

#include "part.h"

#include "containers.h"
#include "style/style.h"
#include "rw/xml.h"

#include "chordrest.h"
#include "factory.h"
#include "fret.h"
#include "instrtemplate.h"
#include "linkedobjects.h"
#include "masterscore.h"
#include "measure.h"
#include "score.h"
#include "staff.h"

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
    _color   = DEFAULT_COLOR;
    _show    = true;
    _soloist = false;
    _instruments.setInstrument(new Instrument, -1);     // default instrument
    _preferSharpFlat = PreferSharpFlat::DEFAULT;
}

//---------------------------------------------------------
//   initFromInstrTemplate
//---------------------------------------------------------

void Part::initFromInstrTemplate(const InstrumentTemplate* t)
{
    _partName = !t->longNames.empty() ? t->longNames.front().name() : t->trackName;
    setInstrument(Instrument::fromTemplate(t));
}

const ID& Part::id() const
{
    return _id;
}

void Part::setId(const ID& id)
{
    _id = id;
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
    return _staves[idx];
}

//---------------------------------------------------------
//   family
//---------------------------------------------------------

String Part::familyId() const
{
    if (_instruments.size() <= 0) {
        return String(u"");
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
    if (_staves.empty()) {
        return this;
    }

    Staff* st = _staves[0];
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

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Part::readProperties(XmlReader& e)
{
    const AsciiStringView tag(e.name());
    if (tag == "id") {
        _id = e.readInt();
    } else if (tag == "Staff") {
        Staff* staff = Factory::createStaff(this);
        score()->appendStaff(staff);
        staff->read(e);
    } else if (tag == "Instrument") {
        Instrument* instr = new Instrument;
        instr->read(e, this);
        setInstrument(instr, Fraction(-1, 1));
    } else if (tag == "name") {
        instrument()->setLongName(e.readText());
    } else if (tag == "color") {
        _color = e.readInt();
    } else if (tag == "shortName") {
        instrument()->setShortName(e.readText());
    } else if (tag == "trackName") {
        _partName = e.readText();
    } else if (tag == "show") {
        _show = e.readInt();
    } else if (tag == "soloist") {
        _soloist = e.readInt();
    } else if (tag == "preferSharpFlat") {
        _preferSharpFlat
            =e.readText() == "sharps" ? PreferSharpFlat::SHARPS : PreferSharpFlat::FLATS;
    } else {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Part::read(XmlReader& e)
{
    _id = e.intAttribute("id", 0);

    while (e.readNextStartElement()) {
        if (!readProperties(e)) {
            e.unknown();
        }
    }
    if (_partName.isEmpty()) {
        _partName = instrument()->trackName();
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Part::write(XmlWriter& xml) const
{
    xml.startElement(this, { { "id", _id.toUint64() } });

    for (const Staff* staff : _staves) {
        staff->write(xml);
    }

    if (!_show) {
        xml.tag("show", _show);
    }

    if (_soloist) {
        xml.tag("soloist", _soloist);
    }

    xml.tag("trackName", _partName);

    if (_color != DEFAULT_COLOR) {
        xml.tag("color", _color);
    }

    if (_preferSharpFlat != PreferSharpFlat::DEFAULT) {
        xml.tag("preferSharpFlat",
                _preferSharpFlat == PreferSharpFlat::SHARPS ? "sharps" : "flats");
    }

    instrument()->write(xml, this);

    xml.endElement();
}

size_t Part::nstaves() const
{
    return _staves.size();
}

const std::vector<Staff*>& Part::staves() const
{
    return _staves;
}

std::set<staff_idx_t> Part::staveIdxList() const
{
    std::set<staff_idx_t> result;

    for (const Staff* stave : _staves) {
        if (!stave) {
            continue;
        }

        result.insert(stave->idx());
    }

    return result;
}

void Part::appendStaff(Staff* staff)
{
    _staves.push_back(staff);
}

void Part::clearStaves()
{
    _staves.clear();
}

//---------------------------------------------------------
//   setLongNames
//---------------------------------------------------------

void Part::setLongNames(std::list<StaffName>& name, const Fraction& tick)
{
    instrument(tick)->longNames() = name;
}

void Part::setShortNames(std::list<StaffName>& name, const Fraction& tick)
{
    instrument(tick)->shortNames() = name;
}

//---------------------------------------------------------
//   setStaves
//---------------------------------------------------------

void Part::setStaves(int n)
{
    int ns = static_cast<int>(_staves.size());
    if (n < ns) {
        LOGD("Part::setStaves(): remove staves not implemented!");
        return;
    }

    int staffIdx = static_cast<int>(score()->staffIdx(this)) + ns;
    for (int i = ns; i < n; ++i) {
        Staff* staff = Factory::createStaff(this);
        _staves.push_back(staff);
        const_cast<std::vector<Staff*>&>(score()->staves()).insert(score()->staves().begin() + staffIdx, staff);

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
    if (idx >= _staves.size()) {
        idx = _staves.size();
    }
    _staves.insert(_staves.begin() + idx, staff);
    staff->setPart(this);
}

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Part::removeStaff(Staff* staff)
{
    if (!mu::remove(_staves, staff)) {
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
    return _capoFret;
}

//---------------------------------------------------------
//   setCapoFret
//---------------------------------------------------------
void Part::setCapoFret(int capoFret)
{
    _capoFret = capoFret;
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
    _instruments.setInstrument(i, tick.ticks());
}

void Part::setInstrument(Instrument* i, int tick)
{
    _instruments.setInstrument(i, tick);
}

void Part::setInstrument(const Instrument&& i, Fraction tick)
{
    _instruments.setInstrument(new Instrument(i), tick.ticks());
}

void Part::setInstrument(const Instrument& i, Fraction tick)
{
    _instruments.setInstrument(new Instrument(i), tick.ticks());
}

void Part::setInstruments(const InstrumentList& instruments)
{
    _instruments.clear();

    for (auto it = instruments.begin(); it != instruments.end(); ++it) {
        _instruments.setInstrument(it->second, it->first);
    }
}

//---------------------------------------------------------
//   removeInstrument
//---------------------------------------------------------

void Part::removeInstrument(const Fraction& tick)
{
    auto i = _instruments.find(tick.ticks());
    if (i == _instruments.end()) {
        LOGD("Part::removeInstrument: not found at tick %d", tick.ticks());
        return;
    }
    _instruments.erase(i);
}

void Part::removeInstrument(const String& instrumentId)
{
    for (auto it = _instruments.begin(); it != _instruments.end(); ++it) {
        if (it->second->instrumentId() == instrumentId) {
            _instruments.erase(it);
            break;
        }
    }
}

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

Instrument* Part::instrument(Fraction tick)
{
    return _instruments.instrument(tick.ticks());
}

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

const Instrument* Part::instrument(Fraction tick) const
{
    return _instruments.instrument(tick.ticks());
}

const Instrument* Part::instrumentById(const std::string& id) const
{
    for (const auto& pair: _instruments) {
        if (pair.second->id().toStdString() == id) {
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
    return _instruments;
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
//   getProperty
//---------------------------------------------------------

PropertyValue Part::getProperty(Pid id) const
{
    switch (id) {
    case Pid::VISIBLE:
        return PropertyValue(_show);
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
    return _staves.front()->idx() * VOICES;
}

//---------------------------------------------------------
//   endTrack
//---------------------------------------------------------

track_idx_t Part::endTrack() const
{
    return _staves.back()->idx() * VOICES + VOICES;
}

InstrumentTrackIdList Part::instrumentTrackIdList() const
{
    InstrumentTrackIdList result;
    std::set<std::string> seen;

    for (const auto& pair : _instruments) {
        std::string instrId = pair.second->id().toStdString();
        if (seen.insert(instrId).second) {
            result.push_back({ _id, instrId });
        }
    }

    return result;
}

InstrumentTrackIdSet Part::instrumentTrackIdSet() const
{
    InstrumentTrackIdSet result;

    for (const auto& pair : _instruments) {
        result.insert({ _id, pair.second->id().toStdString() });
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

        auto si = _instruments.lower_bound(tick.ticks());
        auto ei = _instruments.lower_bound((tick - len).ticks());
        _instruments.erase(si, ei);
    }

    InstrumentList il;
    for (auto i = _instruments.lower_bound(tick.ticks()); i != _instruments.end();) {
        Instrument* instrument = i->second;
        int t = i->first;
        _instruments.erase(i++);
        il[t + len.ticks()] = instrument;
    }
    _instruments.insert(il.begin(), il.end());
}

bool Part::isVisible() const
{
    return _show;
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
