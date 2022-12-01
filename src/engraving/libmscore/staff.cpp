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

#include "containers.h"
#include "rw/xml.h"
#include "types/typesconv.h"

#include "barline.h"
#include "bracket.h"
#include "bracketItem.h"
#include "chord.h"
#include "clef.h"
#include "cleflist.h"
#include "factory.h"
#include "instrtemplate.h"
#include "linkedobjects.h"
#include "masterscore.h"
#include "measure.h"
#include "mscore.h"
#include "note.h"
#include "ottava.h"
#include "part.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "stafftype.h"
#include "timesig.h"

// #define DEBUG_CLEFS

#include "log.h"

using namespace mu;
using namespace mu::engraving;

#ifdef DEBUG_CLEFS
#define DUMP_CLEFS(s) dumpClefs(s)
#else
#define DUMP_CLEFS(s)
#endif

namespace mu::engraving {
//---------------------------------------------------------
//   Staff
//---------------------------------------------------------

Staff::Staff(Part* parent)
    : EngravingItem(ElementType::STAFF, parent)
{
    initFromStaffType(0);
}

//---------------------------------------------------------
//   copy ctor
//---------------------------------------------------------

Staff::Staff(const Staff& staff)
    : EngravingItem(staff)
{
    init(&staff);
    _part = staff._part;
}

//---------------------------------------------------------
//   clone
//---------------------------------------------------------

Staff* Staff::clone() const
{
    return new Staff(*this);
}

//---------------------------------------------------------
//   idx
//---------------------------------------------------------

staff_idx_t Staff::idx() const
{
    return mu::indexOf(score()->staves(), (Staff*)this);
}

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void Staff::triggerLayout() const
{
    score()->setLayoutAll(idx());
}

void Staff::triggerLayout(const Fraction& tick)
{
    score()->setLayout(tick, idx());
}

//---------------------------------------------------------
//   fillBrackets
//    make sure index idx is valid
//---------------------------------------------------------

void Staff::fillBrackets(size_t idx)
{
    for (size_t i = _brackets.size(); i <= idx; ++i) {
        BracketItem* bi = Factory::createBracketItem(score()->dummy());
        bi->setStaff(this);
        bi->setColumn(i);
        _brackets.push_back(bi);
    }
}

//---------------------------------------------------------
//   cleanBrackets
//    remove NO_BRACKET entries from the end of list
//---------------------------------------------------------

void Staff::cleanBrackets()
{
    while (!_brackets.empty() && (_brackets.back()->bracketType() == BracketType::NO_BRACKET)) {
        BracketItem* bi = mu::takeLast(_brackets);
        delete bi;
    }
}

//---------------------------------------------------------
//   bracket
//---------------------------------------------------------

BracketType Staff::bracketType(size_t idx) const
{
    if (idx < _brackets.size()) {
        return _brackets[idx]->bracketType();
    }
    return BracketType::NO_BRACKET;
}

//---------------------------------------------------------
//   bracketSpan
//---------------------------------------------------------

size_t Staff::bracketSpan(size_t idx) const
{
    if (idx < _brackets.size()) {
        return _brackets[idx]->bracketSpan();
    }
    return 0;
}

//---------------------------------------------------------
//   setBracket
//---------------------------------------------------------

void Staff::setBracketType(size_t idx, BracketType val)
{
    fillBrackets(idx);
    _brackets[idx]->setBracketType(val);
    cleanBrackets();
}

//---------------------------------------------------------
//   swapBracket
//---------------------------------------------------------

void Staff::swapBracket(size_t oldIdx, size_t newIdx)
{
    size_t idx = std::max(oldIdx, newIdx);
    fillBrackets(idx);
    _brackets[oldIdx]->setColumn(newIdx);
    _brackets[newIdx]->setColumn(oldIdx);
    mu::swapItemsAt(_brackets, oldIdx, newIdx);
    cleanBrackets();
}

//---------------------------------------------------------
//   changeBracketColumn
//---------------------------------------------------------

void Staff::changeBracketColumn(size_t oldColumn, size_t newColumn)
{
    size_t idx = std::max(oldColumn, newColumn);
    fillBrackets(idx);
    int step = newColumn > oldColumn ? 1 : -1;
    for (size_t i = oldColumn; i != newColumn; i += step) {
        size_t oldIdx = i;
        size_t newIdx = i + step;
        _brackets[oldIdx]->setColumn(newIdx);
        _brackets[newIdx]->setColumn(oldIdx);
        mu::swapItemsAt(_brackets, oldIdx, newIdx);
    }
    cleanBrackets();
}

//---------------------------------------------------------
//   setBracketSpan
//---------------------------------------------------------

void Staff::setBracketSpan(size_t idx, size_t val)
{
    fillBrackets(idx);
    _brackets[idx]->setBracketSpan(val);
}

//---------------------------------------------------------
//   addBracket
//---------------------------------------------------------

void Staff::addBracket(BracketItem* b)
{
    b->setStaff(this);
    if (!_brackets.empty() && _brackets[0]->bracketType() == BracketType::NO_BRACKET) {
        _brackets[0] = b;
    } else {
        //
        // create new bracket level
        //
        for (Staff* s : score()->staves()) {
            if (s == this) {
                s->_brackets.push_back(b);
            } else {
                BracketItem* bi = Factory::createBracketItem(score()->dummy());
                bi->setStaff(this);
                s->_brackets.push_back(bi);
            }
        }
    }
}

//---------------------------------------------------------
//   innerBracket
//    Return type inner bracket.
//    The bracket type determines the staff distance.
//---------------------------------------------------------

BracketType Staff::innerBracket() const
{
    staff_idx_t staffIdx = idx();

    BracketType t = BracketType::NO_BRACKET;
    size_t level = 1000;
    for (size_t i = 0; i < score()->nstaves(); ++i) {
        Staff* staff = score()->staff(i);
        for (size_t k = 0; k < staff->brackets().size(); ++k) {
            const BracketItem* bi = staff->brackets().at(k);
            if (bi->bracketType() != BracketType::NO_BRACKET) {
                if (i < staffIdx && ((i + bi->bracketSpan()) > staffIdx) && k < level) {
                    t = bi->bracketType();
                    level = k;
                    break;
                }
            }
        }
    }
    return t;
}

bool Staff::playbackVoice(int voice) const
{
    return _playbackVoice[voice];
}

void Staff::setPlaybackVoice(int voice, bool val)
{
    _playbackVoice[voice] = val;
}

std::array<bool, VOICES> Staff::visibilityVoices() const
{
    return _visibilityVoices;
}

bool Staff::isVoiceVisible(voice_idx_t voice) const
{
    if (voice >= VOICES) {
        return false;
    }

    return _visibilityVoices[voice];
}

void Staff::setVoiceVisible(voice_idx_t voice, bool visible)
{
    if (voice >= VOICES) {
        return;
    }

    _visibilityVoices[voice] = visible;
}

bool Staff::canDisableVoice() const
{
    auto voices = visibilityVoices();
    int countOfVisibleVoices = 0;
    for (size_t i = 0; i < voices.size(); ++i) {
        if (voices[i]) {
            countOfVisibleVoices++;
        }
    }

    return countOfVisibleVoices > 1;
}

void Staff::updateVisibilityVoices(Staff* masterStaff, const TracksMap& tracks)
{
    if (tracks.empty()) {
        _visibilityVoices = { true, true, true, true };
        return;
    }

    std::array<bool, VOICES> voices{ false, false, false, false };

    voice_idx_t voiceIndex = 0;
    for (voice_idx_t voice = 0; voice < VOICES; voice++) {
        std::vector<track_idx_t> masterStaffTracks = mu::values(tracks, masterStaff->idx() * VOICES + voice % VOICES);
        bool isVoiceVisible = mu::contains(masterStaffTracks, idx() * VOICES + voiceIndex % VOICES);
        if (isVoiceVisible) {
            voices[voice] = true;
            voiceIndex++;
        }
    }

    _visibilityVoices = voices;
}

//---------------------------------------------------------
//   cleanupBrackets
//---------------------------------------------------------

void Staff::cleanupBrackets()
{
    staff_idx_t index = idx();
    size_t n = score()->nstaves();
    for (size_t i = 0; i < _brackets.size(); ++i) {
        if (_brackets[i]->bracketType() == BracketType::NO_BRACKET) {
            continue;
        }
        size_t span = _brackets[i]->bracketSpan();
        if (span > (n - index)) {
            span = n - index;
            _brackets[i]->setBracketSpan(span);
        }
    }
    for (size_t i = 0; i < _brackets.size(); ++i) {
        if (_brackets[i]->bracketType() == BracketType::NO_BRACKET) {
            continue;
        }
        size_t span = _brackets[i]->bracketSpan();
        if (span <= 1) {
            _brackets[i] = Factory::createBracketItem(score()->dummy());
            _brackets[i]->setStaff(this);
        } else {
            // delete all other brackets with same span
            for (size_t k = i + 1; k < _brackets.size(); ++k) {
                if (span == _brackets[k]->bracketSpan()) {
                    _brackets[k] = Factory::createBracketItem(score()->dummy());
                    _brackets[k]->setStaff(this);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   bracketLevels
//---------------------------------------------------------

size_t Staff::bracketLevels() const
{
    size_t columns = 0;
    for (auto bi : _brackets) {
        columns = std::max(columns, bi->column());
    }
    return columns;
}

//---------------------------------------------------------
//   partName
//---------------------------------------------------------

String Staff::partName() const
{
    return _part->partName();
}

//---------------------------------------------------------
//   Staff::clefType
//---------------------------------------------------------

ClefTypeList Staff::clefType(const Fraction& tick) const
{
    ClefTypeList ct = clefs.clef(tick.ticks());
    if (ct._concertClef == ClefType::INVALID) {
        // Clef compatibility based on instrument (override StaffGroup)
        StaffGroup staffGroup = staffType(tick)->group();
        if (staffGroup != StaffGroup::TAB) {
            staffGroup = part()->instrument(tick)->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;
        }

        switch (staffGroup) {
        case StaffGroup::TAB:
        {
            ClefType sct = ClefType(score()->styleI(Sid::tabClef));
            ct = staffType(tick)->lines() <= 4 ? ClefTypeList(sct == ClefType::TAB ? ClefType::TAB4 : ClefType::TAB4_SERIF) : ClefTypeList(
                sct == ClefType::TAB ? ClefType::TAB : ClefType::TAB_SERIF);
        }
        break;
        case StaffGroup::STANDARD:
            ct = defaultClefType();
            break;
        case StaffGroup::PERCUSSION:
            ct = ClefTypeList(ClefType::PERC);
            break;
        }
    }
    return ct;
}

//---------------------------------------------------------
//   Staff::clef
//---------------------------------------------------------

ClefType Staff::clef(const Fraction& tick) const
{
    ClefTypeList c = clefType(tick);
    return score()->styleB(Sid::concertPitch) ? c._concertClef : c._transposingClef;
}

//---------------------------------------------------------
//   Staff::nextClefTick
//
//    return the tick of next clef after tick
//    return last tick of score if not found
//---------------------------------------------------------

Fraction Staff::nextClefTick(const Fraction& tick) const
{
    Fraction t = Fraction::fromTicks(clefs.nextClefTick(tick.ticks()));
    return t != Fraction(-1, 1) ? t : score()->endTick();
}

//---------------------------------------------------------
//   Staff::currentClefTick
//
//    return the tick position of the clef currently
//    in effect at tick
//    return 0, if no such clef
//---------------------------------------------------------

Fraction Staff::currentClefTick(const Fraction& tick) const
{
    return Fraction::fromTicks(clefs.currentClefTick(tick.ticks()));
}

String Staff::staffName() const
{
    return TConv::translatedUserName(clefType(Fraction())._transposingClef);
}

#ifndef NDEBUG
//---------------------------------------------------------
//   dumpClef
//---------------------------------------------------------

void Staff::dumpClefs(const char* title) const
{
    LOGD("(%zd): %s", clefs.size(), title);
    for (auto& i : clefs) {
        LOGD("  %d: %d %d", i.first, int(i.second._concertClef), int(i.second._transposingClef));
    }
}

//---------------------------------------------------------
//   dumpKeys
//---------------------------------------------------------

void Staff::dumpKeys(const char* title) const
{
    LOGD("(%zd): %s", _keys.size(), title);
    for (auto& i : _keys) {
        LOGD("  %d: %d", i.first, int(i.second.key()));
    }
}

//---------------------------------------------------------
//   dumpTimeSigs
//---------------------------------------------------------

void Staff::dumpTimeSigs(const char* title) const
{
    LOGD("size (%zd) staffIdx %zu: %s", timesigs.size(), idx(), title);
    for (auto& i : timesigs) {
        LOGD("  %d: %d/%d", i.first, i.second->sig().numerator(), i.second->sig().denominator());
    }
}

#endif

//---------------------------------------------------------
//   setClef
//---------------------------------------------------------

void Staff::setClef(Clef* clef)
{
    if (clef->generated()) {
        return;
    }
    Fraction tick = clef->segment()->tick();
    for (Segment* s = clef->segment()->next1(); s && s->tick() == tick; s = s->next1()) {
        if ((s->segmentType() == SegmentType::Clef || s->segmentType() == SegmentType::HeaderClef)
            && s->element(clef->track())
            && !s->element(clef->track())->generated()) {
            // adding this clef has no effect on the clefs list
            return;
        }
    }
    clefs.setClef(clef->segment()->tick().ticks(), clef->clefTypeList());
    DUMP_CLEFS("setClef");
}

//---------------------------------------------------------
//   removeClef
//---------------------------------------------------------

void Staff::removeClef(const Clef* clef)
{
    if (clef->generated()) {
        return;
    }
    Fraction tick = clef->segment()->tick();
    for (Segment* s = clef->segment()->next1(); s && s->tick() == tick; s = s->next1()) {
        if ((s->segmentType() == SegmentType::Clef || s->segmentType() == SegmentType::HeaderClef)
            && s->element(clef->track())
            && !s->element(clef->track())->generated()) {
            // removal of this clef has no effect on the clefs list
            return;
        }
    }
    clefs.erase(clef->segment()->tick().ticks());
    for (Segment* s = clef->segment()->prev1(); s && s->tick() == tick; s = s->prev1()) {
        if ((s->segmentType() == SegmentType::Clef || s->segmentType() == SegmentType::HeaderClef)
            && s->element(clef->track())
            && !s->element(clef->track())->generated()) {
            // a previous clef at the same tick position gets valid
            clefs.setClef(tick.ticks(), toClef(s->element(clef->track()))->clefTypeList());
            break;
        }
    }
    DUMP_CLEFS("removeClef");
}

//---------------------------------------------------------
//   timeStretch
//---------------------------------------------------------

Fraction Staff::timeStretch(const Fraction& tick) const
{
    TimeSig* timesig = timeSig(tick);
    return timesig ? timesig->stretch() : Fraction(1, 1);
}

//---------------------------------------------------------
//   timeSig
//    lookup time signature before or at tick
//---------------------------------------------------------

TimeSig* Staff::timeSig(const Fraction& tick) const
{
    auto i = timesigs.upper_bound(tick.ticks());
    if (i != timesigs.begin()) {
        --i;
    }
    if (i == timesigs.end()) {
        return 0;
    } else if (tick < Fraction::fromTicks(i->first)) {
        return 0;
    }
    return i->second;
}

//---------------------------------------------------------
//   nextTimeSig
//    lookup time signature at tick or after
//---------------------------------------------------------

TimeSig* Staff::nextTimeSig(const Fraction& tick) const
{
    auto i = timesigs.lower_bound(tick.ticks());
    return (i == timesigs.end()) ? 0 : i->second;
}

//---------------------------------------------------------
//   currentTimeSigTick
//
//    return the tick position of the time sig currently
//    in effect at tick
//---------------------------------------------------------

Fraction Staff::currentTimeSigTick(const Fraction& tick) const
{
    if (timesigs.empty()) {
        return Fraction(0, 1);
    }
    auto i = timesigs.upper_bound(tick.ticks());
    if (i == timesigs.begin()) {
        return Fraction(0, 1);
    }
    --i;
    return Fraction::fromTicks(i->first);
}

//---------------------------------------------------------
//   group
//---------------------------------------------------------

const Groups& Staff::group(const Fraction& tick) const
{
    TimeSig* ts = timeSig(tick);
    if (ts) {
        if (!ts->groups().empty()) {
            return ts->groups();
        }
        return Groups::endings(ts->sig());
    }
    Measure* m = score()->tick2measure(tick);
    return Groups::endings(m ? m->timesig() : Fraction(4, 4));
}

//---------------------------------------------------------
//   addTimeSig
//---------------------------------------------------------

void Staff::addTimeSig(TimeSig* timesig)
{
    if (timesig->segment()->segmentType() == SegmentType::TimeSig) {
        timesigs[timesig->segment()->tick().ticks()] = timesig;
    }
//      dumpTimeSigs("after addTimeSig");
}

//---------------------------------------------------------
//   removeTimeSig
//---------------------------------------------------------

void Staff::removeTimeSig(TimeSig* timesig)
{
    if (timesig->segment()->segmentType() == SegmentType::TimeSig) {
        if (timesigs[timesig->segment()->tick().ticks()] == timesig) {
            timesigs.erase(timesig->segment()->tick().ticks());
        }
    }
//      dumpTimeSigs("after removeTimeSig");
}

//---------------------------------------------------------
//   clearTimeSig
//---------------------------------------------------------

void Staff::clearTimeSig()
{
    timesigs.clear();
}

//---------------------------------------------------------
//   Staff::keySigEvent
//
//    locates the key sig currently in effect at tick
//---------------------------------------------------------

KeySigEvent Staff::keySigEvent(const Fraction& tick) const
{
    return _keys.key(tick.ticks());
}

//---------------------------------------------------------
//   setKey
//---------------------------------------------------------

void Staff::setKey(const Fraction& tick, KeySigEvent k)
{
    _keys.setKey(tick.ticks(), k);
}

//---------------------------------------------------------
//   removeKey
//---------------------------------------------------------

void Staff::removeKey(const Fraction& tick)
{
    _keys.erase(tick.ticks());
}

//---------------------------------------------------------
//   prevkey
//---------------------------------------------------------

KeySigEvent Staff::prevKey(const Fraction& tick) const
{
    return _keys.prevKey(tick.ticks());
}

//---------------------------------------------------------
//   Staff::nextKeyTick
//
//    return the tick at which the key sig after tick is located
//    return 0, if no such a key sig
//---------------------------------------------------------

Fraction Staff::nextKeyTick(const Fraction& tick) const
{
    Fraction t = Fraction::fromTicks(_keys.nextKeyTick(tick.ticks()));
    return t != Fraction(-1, 1) ? t : score()->endTick();
}

//---------------------------------------------------------
//   Staff::currentKeyTick
//
//    return the tick position of the key currently
//    in effect at tick
//    return 0, if no such a key sig
//---------------------------------------------------------

Fraction Staff::currentKeyTick(const Fraction& tick) const
{
    return Fraction::fromTicks(_keys.currentKeyTick(tick.ticks()));
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Staff::write(XmlWriter& xml) const
{
    xml.startElement(this, { { "id", idx() + 1 } });

    if (links()) {
        Score* s = masterScore();
        for (auto le : *links()) {
            Staff* staff = toStaff(le);
            if ((staff->score() == s) && (staff != this)) {
                xml.tag("linkedTo", static_cast<int>(staff->idx() + 1));
            }
        }
    }

    // for copy/paste we need to know the actual transposition
    if (xml.context()->clipboardmode()) {
        Interval v = part()->instrument()->transpose();     // TODO: tick?
        if (v.diatonic) {
            xml.tag("transposeDiatonic", v.diatonic);
        }
        if (v.chromatic) {
            xml.tag("transposeChromatic", v.chromatic);
        }
    }

    staffType(Fraction(0, 1))->write(xml);
    ClefTypeList ct = _defaultClefType;
    if (ct._concertClef == ct._transposingClef) {
        if (ct._concertClef != ClefType::G) {
            xml.tag("defaultClef", TConv::toXml(ct._concertClef));
        }
    } else {
        xml.tag("defaultConcertClef", TConv::toXml(ct._concertClef));
        xml.tag("defaultTransposingClef", TConv::toXml(ct._transposingClef));
    }

    if (isLinesInvisible(Fraction(0, 1))) {
        xml.tag("invisible", isLinesInvisible(Fraction(0, 1)));
    }
    if (hideWhenEmpty() != HideMode::AUTO) {
        xml.tag("hideWhenEmpty", int(hideWhenEmpty()));
    }
    if (cutaway()) {
        xml.tag("cutaway", cutaway());
    }
    if (showIfEmpty()) {
        xml.tag("showIfSystemEmpty", showIfEmpty());
    }
    if (_hideSystemBarLine) {
        xml.tag("hideSystemBarLine", _hideSystemBarLine);
    }
    if (_mergeMatchingRests) {
        xml.tag("mergeMatchingRests", _mergeMatchingRests);
    }
    if (!visible()) {
        xml.tag("isStaffVisible", visible());
    }

    for (const BracketItem* i : _brackets) {
        BracketType a = i->bracketType();
        size_t b = i->bracketSpan();
        size_t c = i->column();
        if (a != BracketType::NO_BRACKET || b > 0) {
            xml.tag("bracket", { { "type", static_cast<int>(a) }, { "span", b }, { "col", c } });
        }
    }

    writeProperty(xml, Pid::STAFF_BARLINE_SPAN);
    writeProperty(xml, Pid::STAFF_BARLINE_SPAN_FROM);
    writeProperty(xml, Pid::STAFF_BARLINE_SPAN_TO);
    writeProperty(xml, Pid::STAFF_USERDIST);
    writeProperty(xml, Pid::STAFF_COLOR);
    writeProperty(xml, Pid::PLAYBACK_VOICE1);
    writeProperty(xml, Pid::PLAYBACK_VOICE2);
    writeProperty(xml, Pid::PLAYBACK_VOICE3);
    writeProperty(xml, Pid::PLAYBACK_VOICE4);
    xml.endElement();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Staff::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        if (!readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Staff::readProperties(XmlReader& e)
{
    const AsciiStringView tag(e.name());
    if (tag == "StaffType") {
        StaffType st;
        st.read(e);
        setStaffType(Fraction(0, 1), st);
    } else if (tag == "defaultClef") {           // sets both default transposing and concert clef
        ClefType ct = TConv::fromXml(e.readAsciiText(), ClefType::G);
        setDefaultClefType(ClefTypeList(ct, ct));
    } else if (tag == "defaultConcertClef") {
        setDefaultClefType(ClefTypeList(TConv::fromXml(e.readAsciiText(), ClefType::G), defaultClefType()._transposingClef));
    } else if (tag == "defaultTransposingClef") {
        setDefaultClefType(ClefTypeList(defaultClefType()._concertClef, TConv::fromXml(e.readAsciiText(), ClefType::G)));
    } else if (tag == "small") {                // obsolete
        staffType(Fraction(0, 1))->setSmall(e.readInt());
    } else if (tag == "invisible") {
        staffType(Fraction(0, 1))->setInvisible(e.readInt());              // same as: setInvisible(Fraction(0,1)), e.readInt())
    } else if (tag == "hideWhenEmpty") {
        setHideWhenEmpty(HideMode(e.readInt()));
    } else if (tag == "cutaway") {
        setCutaway(e.readInt());
    } else if (tag == "showIfSystemEmpty") {
        setShowIfEmpty(e.readInt());
    } else if (tag == "hideSystemBarLine") {
        _hideSystemBarLine = e.readInt();
    } else if (tag == "mergeMatchingRests") {
        _mergeMatchingRests = e.readInt();
    } else if (tag == "isStaffVisible") {
        setVisible(e.readBool());
    } else if (tag == "keylist") {
        _keys.read(e, score());
    } else if (tag == "bracket") {
        int col = e.intAttribute("col", -1);
        if (col == -1) {
            col = static_cast<int>(_brackets.size());
        }
        setBracketType(col, BracketType(e.intAttribute("type", -1)));
        setBracketSpan(col, e.intAttribute("span", 0));
        e.readNext();
    } else if (tag == "barLineSpan") {
        _barLineSpan = e.readInt();
    } else if (tag == "barLineSpanFrom") {
        _barLineFrom = e.readInt();
    } else if (tag == "barLineSpanTo") {
        _barLineTo = e.readInt();
    } else if (tag == "distOffset") {
        _userDist = e.readDouble() * score()->spatium();
    } else if (tag == "mag") {
        /*_userMag =*/
        e.readDouble(0.1, 10.0);
    } else if (tag == "linkedTo") {
        int v = e.readInt() - 1;
        Staff* st = score()->masterScore()->staff(v);
        if (_links) {
            LOGD("Staff::readProperties: multiple <linkedTo> tags");
            if (!st || isLinked(st)) {     // maybe we don't need actually to relink...
                return true;
            }
            // not using unlink() here as it may delete _links
            // a pointer to which is stored also in XmlReader.
            _links->remove(this);
            _links = nullptr;
        }
        if (st && st != this) {
            linkTo(st);
        } else if (!score()->isMaster() && !st) {
            // if it is a master score it is OK not to find
            // a staff which is going after the current one.
            LOGD("staff %d not found in parent", v);
        }
    } else if (tag == "color") {
        staffType(Fraction(0, 1))->setColor(e.readColor());
    } else if (tag == "transposeDiatonic") {
        e.context()->setTransposeDiatonic(static_cast<int8_t>(e.readInt()));
    } else if (tag == "transposeChromatic") {
        e.context()->setTransposeChromatic(static_cast<int8_t>(e.readInt()));
    } else if (tag == "playbackVoice1") {
        setPlaybackVoice(0, e.readInt());
    } else if (tag == "playbackVoice2") {
        setPlaybackVoice(1, e.readInt());
    } else if (tag == "playbackVoice3") {
        setPlaybackVoice(2, e.readInt());
    } else if (tag == "playbackVoice4") {
        setPlaybackVoice(3, e.readInt());
    } else {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   height
//---------------------------------------------------------

double Staff::height() const
{
    Fraction tick = Fraction(0, 1);
    return (lines(tick) - 1) * spatium(tick) * staffType(tick)->lineDistance().val();
}

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

double Staff::spatium(const Fraction& tick) const
{
    return score()->spatium() * staffMag(tick);
}

double Staff::spatium(const EngravingItem* e) const
{
    return score()->spatium() * staffMag(e);
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double Staff::staffMag(const StaffType* stt) const
{
    return (stt->isSmall() ? score()->styleD(Sid::smallStaffMag) : 1.0) * stt->userMag();
}

double Staff::staffMag(const Fraction& tick) const
{
    return staffMag(staffType(tick));
}

double Staff::staffMag(const EngravingItem* element) const
{
    return staffMag(staffTypeForElement(element));
}

//---------------------------------------------------------
//   swing
//---------------------------------------------------------

SwingParameters Staff::swing(const Fraction& tick) const
{
    SwingParameters sp;
    int swingUnit = 0;
    ByteArray ba = score()->styleSt(Sid::swingUnit).toAscii();
    DurationType unit = TConv::fromXml(ba.constChar(), DurationType::V_INVALID);
    int swingRatio = score()->styleI(Sid::swingRatio);
    if (unit == DurationType::V_EIGHTH) {
        swingUnit = Constants::division / 2;
    } else if (unit == DurationType::V_16TH) {
        swingUnit = Constants::division / 4;
    } else if (unit == DurationType::V_ZERO) {
        swingUnit = 0;
    }
    sp.swingRatio = swingRatio;
    sp.swingUnit = swingUnit;
    if (_swingList.empty()) {
        return sp;
    }

    std::vector<int> ticks = mu::keys(_swingList);
    auto it = std::upper_bound(ticks.cbegin(), ticks.cend(), tick.ticks());
    if (it == ticks.cbegin()) {
        return sp;
    }
    --it;
    return _swingList.at(*it);
}

//---------------------------------------------------------
//   capo
//---------------------------------------------------------

int Staff::capo(const Fraction& tick) const
{
    if (_capoList.empty()) {
        return 0;
    }

    std::vector<int> ticks = mu::keys(_capoList);
    auto it = std::upper_bound(ticks.cbegin(), ticks.cend(), tick.ticks());
    if (it == ticks.cbegin()) {
        return 0;
    }
    --it;
    return _capoList.at(*it);
}

//---------------------------------------------------------
//   getNotes
//---------------------------------------------------------

std::list<Note*> Staff::getNotes() const
{
    std::list<Note*> list;

    staff_idx_t staffIdx = idx();

    SegmentType st = SegmentType::ChordRest;
    for (Segment* s = score()->firstSegment(st); s; s = s->next1(st)) {
        for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
            track_idx_t track = voice + staffIdx * VOICES;
            EngravingItem* e = s->element(track);
            if (e && e->isChord()) {
                addChord(list, toChord(e), voice);
            }
        }
    }

    return list;
}

//---------------------------------------------------------
//   addChord
//---------------------------------------------------------

void Staff::addChord(std::list<Note*>& list, Chord* chord, voice_idx_t voice) const
{
    for (Chord* c : chord->graceNotes()) {
        addChord(list, c, voice);
    }
    for (Note* note : chord->notes()) {
        if (note->tieBack()) {
            continue;
        }
        list.push_back(note);
    }
}

//---------------------------------------------------------
//   channel
//---------------------------------------------------------

int Staff::channel(const Fraction& tick, voice_idx_t voice) const
{
    if (_channelList[voice].empty()) {
        return 0;
    }

    std::vector<int> ticks = mu::keys(_channelList[voice]);
    auto it = std::upper_bound(ticks.cbegin(), ticks.cend(), tick.ticks());
    if (it == ticks.cbegin()) {
        return 0;
    }
    --it;
    return _channelList[voice].at(*it);
}

//---------------------------------------------------------
//   middleLine
//    returns logical line number of middle staff line
//---------------------------------------------------------

int Staff::middleLine(const Fraction& tick) const
{
    return staffType(tick)->middleLine();
}

//---------------------------------------------------------
//   bottomLine
//    returns logical line number of bottom staff line
//---------------------------------------------------------

int Staff::bottomLine(const Fraction& tick) const
{
    return staffType(tick)->bottomLine();
}

//---------------------------------------------------------
//   stemless
//---------------------------------------------------------

bool Staff::stemless(const Fraction& tick) const
{
    return staffType(tick)->stemless();
}

//---------------------------------------------------------
//   setSlashStyle
//---------------------------------------------------------

void Staff::setSlashStyle(const Fraction& tick, bool val)
{
    staffType(tick)->setStemless(val);
}

//---------------------------------------------------------
//   isPrimaryStaff
///   if there are linked staves, the primary staff is
///   the one who is played back and it's not a tab staff
///   because we don't have enough information  to play
///   e.g ornaments. NOTE: it's not necessarily the top staff!
//---------------------------------------------------------

bool Staff::isPrimaryStaff() const
{
    if (!_links) {
        return true;
    }
    std::list<Staff*> s;
    std::list<Staff*> ss;
    for (auto e : *_links) {
        Staff* staff = toStaff(e);
        if (staff->score() == score()) {
            s.push_back(staff);
            if (!staff->isTabStaff(Fraction(0, 1))) {
                ss.push_back(staff);
            }
        }
    }
    if (s.size() == 1) { // the linked staves are in different scores
        return s.front() == this;
    } else { // return a non tab linked staff in this score
        return ss.front() == this;
    }
}

//---------------------------------------------------------
//   staffType
//---------------------------------------------------------

const StaffType* Staff::staffType(const Fraction& tick) const
{
    return &_staffTypeList.staffType(tick);
}

const StaffType* Staff::constStaffType(const Fraction& tick) const
{
    return &_staffTypeList.staffType(tick);
}

StaffType* Staff::staffType(const Fraction& tick)
{
    return &_staffTypeList.staffType(tick);
}

const StaffType* Staff::staffTypeForElement(const EngravingItem* e) const
{
    if (_staffTypeList.uniqueStaffType()) {
        // if one staff type spans for the entire staff, optimize by omitting a call to `tick()`
        return &_staffTypeList.staffType({ 0, 1 });
    }
    return &_staffTypeList.staffType(e->tick());
}

bool Staff::isStaffTypeStartFrom(const Fraction& tick) const
{
    return _staffTypeList.isStaffTypeStartFrom(tick);
}

void Staff::moveStaffType(const Fraction& from, const Fraction& to)
{
    _staffTypeList.moveStaffType(from, to);
    staffTypeListChanged(from);
}

//---------------------------------------------------------
//   staffTypeListChanged
//    Signal that the staffTypeList has changed at
//    position tick. Update layout range.
//---------------------------------------------------------

void Staff::staffTypeListChanged(const Fraction& tick)
{
    std::pair<int, int> range = _staffTypeList.staffTypeRange(tick);

    if (range.first < 0) {
        triggerLayout(Fraction(0, 1));
    } else {
        triggerLayout(Fraction::fromTicks(range.first));
    }

    if (range.second < 0) {
        // When reading a score and there is a Staff Change on the first
        // measure, there are no measures yet and nothing to layout.
        if (score()->lastMeasure()) {
            triggerLayout(score()->lastMeasure()->endTick());
        }
    } else {
        triggerLayout(Fraction::fromTicks(range.second));
    }
}

//---------------------------------------------------------
//   setStaffType
//---------------------------------------------------------

StaffType* Staff::setStaffType(const Fraction& tick, const StaffType& nst)
{
    return _staffTypeList.setStaffType(tick, nst);
}

//---------------------------------------------------------
//   setStaffType
//---------------------------------------------------------

void Staff::removeStaffType(const Fraction& tick)
{
    double old = spatium(tick);
    const bool removed = _staffTypeList.removeStaffType(tick);
    if (!removed) {
        return;
    }
    setLocalSpatium(old, spatium(tick), tick);
    staffTypeListChanged(tick);
}

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Staff::init(const InstrumentTemplate* t, const StaffType* staffType, int cidx)
{
    // set staff-type-independent parameters
    const StaffType* pst = staffType ? staffType : t->staffTypePreset;
    if (!pst) {
        pst = StaffType::getDefaultPreset(t->staffGroup);
    }

    StaffType* stt = setStaffType(Fraction(0, 1), *pst);
    if (cidx >= MAX_STAVES) {
        stt->setSmall(false);
    } else {
        stt->setSmall(t->smallStaff[cidx]);
        setBracketType(0, t->bracket[cidx]);
        setBracketSpan(0, t->bracketSpan[cidx]);
        setBarLineSpan(t->barlineSpan[cidx]);
    }
    setDefaultClefType(t->clefType(cidx));
}

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Staff::init(const Staff* s)
{
    _id                = s->_id;
    _staffTypeList     = s->_staffTypeList;
    setDefaultClefType(s->defaultClefType());
    for (BracketItem* i : s->_brackets) {
        BracketItem* ni = new BracketItem(*i);
        ni->setScore(score());
        ni->setStaff(this);
        _brackets.push_back(ni);
    }
    _barLineSpan       = s->_barLineSpan;
    _barLineFrom       = s->_barLineFrom;
    _barLineTo         = s->_barLineTo;
    _hideWhenEmpty     = s->_hideWhenEmpty;
    _cutaway           = s->_cutaway;
    _showIfEmpty       = s->_showIfEmpty;
    _hideSystemBarLine = s->_hideSystemBarLine;
    _mergeMatchingRests = s->_mergeMatchingRests;
    _color             = s->_color;
    _userDist          = s->_userDist;
    _visibilityVoices = s->_visibilityVoices;
}

const ID& Staff::id() const
{
    return _id;
}

void Staff::setId(const ID& id)
{
    _id = id;
}

void Staff::setScore(Score* score)
{
    EngravingItem::setScore(score);

    for (BracketItem* bracket: _brackets) {
        bracket->setScore(score);
    }
}

//---------------------------------------------------------
//   initFromStaffType
//---------------------------------------------------------

void Staff::initFromStaffType(const StaffType* staffType)
{
    // get staff type if given (if none, get default preset for default staff group)
    if (!staffType) {
        staffType = StaffType::getDefaultPreset(StaffGroup::STANDARD);
    }

    // use selected staff type
    setStaffType(Fraction(0, 1), *staffType);
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Staff::spatiumChanged(double oldValue, double newValue)
{
    _userDist = (_userDist / oldValue) * newValue;
}

//---------------------------------------------------------
//   show
//---------------------------------------------------------

bool Staff::show() const
{
    return _part->show() && visible();
}

//---------------------------------------------------------
//   genKeySig
//---------------------------------------------------------

bool Staff::genKeySig()
{
    if (constStaffType(Fraction(0, 1))->group() == StaffGroup::TAB) {
        return false;
    } else {
        return constStaffType(Fraction(0, 1))->genKeysig();
    }
}

//---------------------------------------------------------
//   showLedgerLines
//---------------------------------------------------------

bool Staff::showLedgerLines(const Fraction& tick) const
{
    return staffType(tick)->showLedgerLines();
}

//---------------------------------------------------------
//   color
//---------------------------------------------------------

mu::draw::Color Staff::color(const Fraction& tick) const
{
    return staffType(tick)->color();
}

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void Staff::setColor(const Fraction& tick, const mu::draw::Color& val)
{
    staffType(tick)->setColor(val);
}

//---------------------------------------------------------
//   updateOttava
//---------------------------------------------------------

void Staff::updateOttava()
{
    staff_idx_t staffIdx = idx();
    _pitchOffsets.clear();
    for (auto i : score()->spanner()) {
        const Spanner* s = i.second;
        if (s->type() == ElementType::OTTAVA && s->staffIdx() == staffIdx) {
            const Ottava* o = static_cast<const Ottava*>(s);
            _pitchOffsets.setPitchOffset(o->tick().ticks(), o->pitchShift());
            _pitchOffsets.setPitchOffset(o->tick2().ticks(), 0);
        }
    }
}

//---------------------------------------------------------
//   undoSetColor
//---------------------------------------------------------

void Staff::undoSetColor(const mu::draw::Color& /*val*/)
{
//      undoChangeProperty(Pid::COLOR, val);
}

//---------------------------------------------------------
//   insertTime
//---------------------------------------------------------

void Staff::insertTime(const Fraction& tick, const Fraction& len)
{
    if (len.isZero()) {
        return;
    }

    // move all keys and clefs >= tick

    if (len < Fraction(0, 1)) {
        // remove entries between tickpos >= tick and tickpos < (tick+len)
        _keys.erase(_keys.lower_bound(tick.ticks()), _keys.lower_bound((tick - len).ticks()));
        clefs.erase(clefs.lower_bound(tick.ticks()), clefs.lower_bound((tick - len).ticks()));
    }

    KeyList kl2;
    for (auto i = _keys.lower_bound(tick.ticks()); i != _keys.end();) {
        KeySigEvent kse = i->second;
        Fraction t = Fraction::fromTicks(i->first);
        _keys.erase(i++);
        kl2[(t + len).ticks()] = kse;
    }
    _keys.insert(kl2.begin(), kl2.end());

    // check if there is a clef at the end of measure
    // before tick
    Clef* clef = 0;
    Measure* m = score()->tick2measure(tick);
    if (m && (m->tick() == tick) && (m->prevMeasure())) {
        m = m->prevMeasure();
        Segment* s = m->findSegment(SegmentType::Clef, tick);
        if (s) {
            track_idx_t track = idx() * VOICES;
            clef = toClef(s->element(track));
        }
    }

    ClefList cl2;
    for (auto i = clefs.lower_bound(tick.ticks()); i != clefs.end();) {
        ClefTypeList ctl = i->second;
        Fraction t = Fraction::fromTicks(i->first);
        if (clef && tick == t) {
            ++i;
            continue;
        }
        clefs.erase(i++);
        cl2.setClef((t + len).ticks(), ctl);
    }
    clefs.insert(cl2.begin(), cl2.end());

    // check if there is a clef at the end of measure
    // before tick: do not remove from clefs list

    if (clef) {
        setClef(clef);
    }

    updateOttava();
    DUMP_CLEFS("  insertTime");
}

//---------------------------------------------------------
//   staffList
//    return list of linked staves
//---------------------------------------------------------

std::list<Staff*> Staff::staffList() const
{
    std::list<Staff*> staffList;
    if (_links) {
        for (EngravingObject* e : *_links) {
            staffList.push_back(toStaff(e));
        }
//            staffList = _linkedStaves->staves();
    } else {
        staffList.push_back(const_cast<Staff*>(this));
    }
    return staffList;
}

Staff* Staff::primaryStaff() const
{
    const LinkedObjects* linkedElements = links();
    if (!linkedElements) {
        return nullptr;
    }

    for (auto linkedElement : *linkedElements) {
        Staff* staff = toStaff(linkedElement);
        if (staff && staff->isPrimaryStaff()) {
            return staff;
        }
    }

    return nullptr;
}

//---------------------------------------------------------
//   rstaff
//---------------------------------------------------------

staff_idx_t Staff::rstaff() const
{
    return mu::indexOf(_part->staves(), this);
}

//---------------------------------------------------------
//   isTop
//---------------------------------------------------------

bool Staff::isTop() const
{
    if (_part->staves().empty()) {
        return false;
    }

    return _part->staves().front() == this;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Staff::getProperty(Pid id) const
{
    switch (id) {
    case Pid::SMALL:
        return staffType(Fraction(0, 1))->isSmall();
    case Pid::MAG:
        return staffType(Fraction(0, 1))->userMag();
    case Pid::STAFF_INVISIBLE:
        return staffType(Fraction(0, 1))->invisible();
    case Pid::STAFF_COLOR:
        return PropertyValue::fromValue(staffType(Fraction(0, 1))->color());
    case Pid::PLAYBACK_VOICE1:
        return playbackVoice(0);
    case Pid::PLAYBACK_VOICE2:
        return playbackVoice(1);
    case Pid::PLAYBACK_VOICE3:
        return playbackVoice(2);
    case Pid::PLAYBACK_VOICE4:
        return playbackVoice(3);
    case Pid::STAFF_BARLINE_SPAN:
        return barLineSpan();
    case Pid::STAFF_BARLINE_SPAN_FROM:
        return barLineFrom();
    case Pid::STAFF_BARLINE_SPAN_TO:
        return barLineTo();
    case Pid::STAFF_USERDIST:
        return userDist();
    case Pid::GENERATED:
        return false;
    default:
        LOGD("unhandled id <%s>", propertyName(id));
        return PropertyValue();
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Staff::setProperty(Pid id, const PropertyValue& v)
{
    switch (id) {
    case Pid::SMALL: {
        double _spatium = spatium(Fraction(0, 1));
        staffType(Fraction(0, 1))->setSmall(v.toBool());
        setLocalSpatium(_spatium, spatium(Fraction(0, 1)), Fraction(0, 1));
        break;
    }
    case Pid::MAG: {
        double _spatium = spatium(Fraction(0, 1));
        staffType(Fraction(0, 1))->setUserMag(v.toReal());
        setLocalSpatium(_spatium, spatium(Fraction(0, 1)), Fraction(0, 1));
    }
    break;
    case Pid::STAFF_COLOR:
        setColor(Fraction(0, 1), v.value<mu::draw::Color>());
        break;
    case Pid::PLAYBACK_VOICE1:
        setPlaybackVoice(0, v.toBool());
        break;
    case Pid::PLAYBACK_VOICE2:
        setPlaybackVoice(1, v.toBool());
        break;
    case Pid::PLAYBACK_VOICE3:
        setPlaybackVoice(2, v.toBool());
        break;
    case Pid::PLAYBACK_VOICE4:
        setPlaybackVoice(3, v.toBool());
        break;
    case Pid::STAFF_BARLINE_SPAN: {
        setBarLineSpan(v.toInt());
        // update non-generated barlines
        track_idx_t track = idx() * VOICES;
        std::vector<EngravingItem*> blList;
        for (Measure* m = score()->firstMeasure(); m; m = m->nextMeasure()) {
            Segment* s = m->getSegmentR(SegmentType::EndBarLine, m->ticks());
            if (s && s->element(track)) {
                blList.push_back(s->element(track));
            }
            if (Measure* mm = m->mmRest()) {
                Segment* ss = mm->getSegmentR(SegmentType::EndBarLine, mm->ticks());
                if (ss && ss->element(track)) {
                    blList.push_back(ss->element(track));
                }
            }
        }
        for (EngravingItem* e : blList) {
            if (e && e->isBarLine() && !e->generated()) {
                toBarLine(e)->setSpanStaff(v.toInt());
            }
        }
    }
    break;
    case Pid::STAFF_BARLINE_SPAN_FROM:
        setBarLineFrom(v.toInt());
        break;
    case Pid::STAFF_BARLINE_SPAN_TO:
        setBarLineTo(v.toInt());
        break;
    case Pid::STAFF_USERDIST:
        setUserDist(v.value<Millimetre>());
        break;
    default:
        LOGD("unhandled id <%s>", propertyName(id));
        break;
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Staff::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SMALL:
        return false;
    case Pid::MAG:
        return 1.0;
    case Pid::STAFF_COLOR:
        return PropertyValue::fromValue(engravingConfiguration()->defaultColor());
    case Pid::PLAYBACK_VOICE1:
    case Pid::PLAYBACK_VOICE2:
    case Pid::PLAYBACK_VOICE3:
    case Pid::PLAYBACK_VOICE4:
        return true;
    case Pid::STAFF_BARLINE_SPAN:
        return false;
    case Pid::STAFF_BARLINE_SPAN_FROM:
    case Pid::STAFF_BARLINE_SPAN_TO:
        return 0;
    case Pid::STAFF_USERDIST:
        return Millimetre(0.0);
    default:
        LOGD("unhandled id <%s>", propertyName(id));
        return PropertyValue();
    }
}

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void Staff::setLocalSpatium(double oldVal, double newVal, Fraction tick)
{
    const int intEndTick = _staffTypeList.staffTypeRange(tick).second;
    const Fraction etick = (intEndTick == -1) ? score()->lastMeasure()->endTick() : Fraction::fromTicks(intEndTick);

    staff_idx_t staffIdx = idx();
    track_idx_t startTrack = staffIdx * VOICES;
    track_idx_t endTrack = startTrack + VOICES;
    for (Segment* s = score()->tick2rightSegment(tick); s && s->tick() < etick; s = s->next1()) {
        for (EngravingItem* e : s->annotations()) {
            if (e->track() >= startTrack && e->track() < endTrack) {
                e->localSpatiumChanged(oldVal, newVal);
            }
        }
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            if (s->element(track)) {
                s->element(track)->localSpatiumChanged(oldVal, newVal);
            }
        }
    }
    auto spanners = score()->spannerMap().findContained(tick.ticks(), etick.ticks());
    for (auto interval : spanners) {
        Spanner* spanner = interval.value;
        if (spanner->staffIdx() == staffIdx) {
            for (auto k : spanner->spannerSegments()) {
                k->localSpatiumChanged(oldVal, newVal);
            }
        }
    }
}

//---------------------------------------------------------
//   isPitchedStaff
//---------------------------------------------------------

bool Staff::isPitchedStaff(const Fraction& tick) const
{
    //return staffType(tick)->group() == StaffGroup::STANDARD;
    return staffType(tick)->group() != StaffGroup::TAB && !part()->instrument(tick)->useDrumset();
}

//---------------------------------------------------------
//   isTabStaff
//---------------------------------------------------------

bool Staff::isTabStaff(const Fraction& tick) const
{
    return staffType(tick)->group() == StaffGroup::TAB;
}

//---------------------------------------------------------
//   isDrumStaff
//---------------------------------------------------------

bool Staff::isDrumStaff(const Fraction& tick) const
{
    //check for instrument instead of staffType (for pitched to unpitched instr. changes)
    return part()->instrument(tick)->useDrumset();
}

//---------------------------------------------------------
//   lines
//---------------------------------------------------------

int Staff::lines(const Fraction& tick) const
{
    return staffType(tick)->lines();
}

//---------------------------------------------------------
//   setLines
//---------------------------------------------------------

void Staff::setLines(const Fraction& tick, int val)
{
    staffType(tick)->setLines(val);
}

//---------------------------------------------------------
//   lineDistance
//    distance between staff lines
//---------------------------------------------------------

double Staff::lineDistance(const Fraction& tick) const
{
    return staffType(tick)->lineDistance().val();
}

//---------------------------------------------------------
//   invisible
//---------------------------------------------------------

bool Staff::isLinesInvisible(const Fraction& tick) const
{
    return staffType(tick)->invisible();
}

//---------------------------------------------------------
//   setInvisible
//---------------------------------------------------------

void Staff::setIsLinesInvisible(const Fraction& tick, bool val)
{
    staffType(tick)->setInvisible(val);
}
}
