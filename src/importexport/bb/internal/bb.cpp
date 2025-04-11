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

#include <QFile>
#include <QFileInfo>

#include "bb.h"

#include "engraving/dom/box.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordlist.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/text.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/utils.h"
#include "engraving/engravingerrors.h"

#include "log.h"

using namespace mu::engraving;

namespace mu::iex::bb {
//---------------------------------------------------------
//   BBTrack
//---------------------------------------------------------

BBTrack::BBTrack(BBFile* f)
{
    bb          = f;
    _outChannel = -1;
    _drumTrack  = false;
}

BBTrack::~BBTrack()
{
}

//---------------------------------------------------------
//   MNote
//    special Midi Note
//---------------------------------------------------------

struct MNote {
    Event mc;
    QList<Tie*> ties;

    MNote(const Event& _mc)
        : mc(_mc)
    {
        for (size_t i = 0; i < mc.notes().size(); ++i) {
            ties.append(0);
        }
    }
};

//---------------------------------------------------------
//   BBFile
//---------------------------------------------------------

BBFile::BBFile()
{
    for (int i = 0; i < MAX_BARS; ++i) {
        _barType[i]  = 0;
    }
    bbDivision = 120;
}

//---------------------------------------------------------
//   BBFile
//---------------------------------------------------------

BBFile::~BBFile()
{
}

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

bool BBFile::read(const QString& name)
{
    _siglist.clear();
    _siglist.add(0, Fraction(4, 4));          // debug

    _path = name;
    QFile f(name);
    if (!f.open(QIODevice::ReadOnly)) {
        return false;
    }
    ba = f.readAll();
    f.close();

    a    = (const unsigned char*)ba.data();
    size = ba.size();

    //---------------------------------------------------
    //    read version
    //---------------------------------------------------

    int idx = 0;
    _version = a[idx++];
    switch (_version) {
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49:
        break;
    default:
        LOGD("BB: unknown file version %02x", _version);
        return false;
    }

    LOGD("read <%s> version 0x%02x", qPrintable(name), _version);

    //---------------------------------------------------
    //    read title
    //---------------------------------------------------

    int len = a[idx++];
    _title  = new char[len + 1];
    for (int i = 0; i < len; ++i) {
        _title[i] = a[idx++];
    }
    _title[len] = 0;

    //---------------------------------------------------
    //    read style(timesig), key and bpm
    //---------------------------------------------------

    ++idx;
    ++idx;
    _style = a[idx++] - 1;
    if (_style < 0 || _style >= int(sizeof(styles) / sizeof(*styles))) {
        LOGD("Import bb: unknown style %d", _style + 1);
        return false;
    }
    _key = a[idx++];

    // map D# G# A#   to Eb Ab Db
    // major   C, Db,  D, Eb,  E,  F, Gb,  G, Ab,  A, Bb,  B, C#, D#, F#, G#, A#
    // minor   C, Db,  D, Eb,  E,  F, Gb,  G, Ab,  A, Bb,  B, C#, D#, F#, G#, A#
    static int kt[] = {
        0,    0, -5,  2, -3,  4, -1, -6,  1, -4,  3, -2,  5,  7, -3,  6, -4, -2,
        -3, 4,  -1, -6,  1, -4,  3, -2,  5,  0, -5,  2,  4,  6,  3,  5, 7
    };
    if (_key >= int(sizeof(kt) / sizeof(*kt))) {
        LOGD("bad key %d", _key);
        return false;
    }
    _key = kt[_key];

    _bpm = a[idx] + (a[idx + 1] << 8);
    idx += 2;

    LOGD("Title <%s>", _title);
    LOGD("style %d",   _style);
    LOGD("key   %d",   _key);
    LOGD("sig   %d/%d", timesigZ(), timesigN());
    LOGD("bpm   %d", _bpm);

    //---------------------------------------------------
    //    read bar types
    //---------------------------------------------------

    int bar = a[idx++];             // starting bar number
    while (bar < 255) {
        int val = a[idx++];
        if (val == 0) {
            bar += a[idx++];
        } else {
            LOGD("bar type: bar %d val %d", bar, val);
            _barType[bar++] = val;
        }
    }

    //---------------------------------------------------
    //    read chord extensions
    //---------------------------------------------------

    int beat;
    for (beat = 0; beat < MAX_BARS * 4;) {
        int val = a[idx++];
        if (val == 0) {
            beat += a[idx++];
        } else {
            BBChord c;
            c.extension = val;
            c.beat      = beat * (timesigZ() / timesigN());
            ++beat;
            _chords.append(c);
        }
    }

    //---------------------------------------------------
    //    read chord root
    //---------------------------------------------------

    int roots = 0;
    int maxbeat = 0;
    for (beat = 0; beat < MAX_BARS * 4;) {
        int val = a[idx++];
        if (val == 0) {
            beat += a[idx++];
        } else {
            int root = val % 18;
            int bass = (root - 1 + val / 18) % 18 + 1;
            if (root == bass) {
                bass = 0;
            }
            int ibeat = beat * (timesigZ() / timesigN());
            if (ibeat != _chords[roots].beat) {
                LOGD("import bb: inconsistent chord type and root beat");
                return false;
            }
            _chords[roots].root = root;
            _chords[roots].bass = bass;
            if (maxbeat < beat) {
                maxbeat = beat;
            }
            ++roots;
            ++beat;
        }
    }

    _measures = ((maxbeat + timesigZ() - 1) / timesigZ()) + 1;

    if (roots != _chords.size()) {
        LOGD("import bb: roots %d != extensions %lld", roots, _chords.size());
        return false;
    }
    LOGD("Measures %d", _measures);

    if (a[idx] == 1) {              //??
        LOGD("Skip 0x%02x at 0x%04x", a[idx], idx);
        ++idx;
    }

    _startChorus = a[idx++];
    _endChorus   = a[idx++];
    _repeats     = a[idx++];

    LOGD("start chorus %d  end chorus %d repeats %d, pos now 0x%x",
         _startChorus, _endChorus, _repeats, idx);

    if (_startChorus >= _endChorus) {
        _startChorus = 0;
        _endChorus = 0;
        _repeats = 1;
    }

    //---------------------------------------------------
    //    read style file
    //---------------------------------------------------

    bool found = false;
    for (int i = idx; i < size; ++i) {
        if (a[i] == 0x42) {
            if (a[i + 1] < 16) {
                for (int k = i + 2; k < (i + 18); ++k) {
                    if (a[k] == '.' && a[k + 1] == 'S' && a[k + 2] == 'T' && a[k + 3] == 'Y') {
                        found = true;
                        break;
                    }
                }
            }
            if (found) {
                idx = i + 1;
                break;
            }
        }
    }
    if (!found) {
        LOGD("import bb: style file not found");
        return false;
    }

    LOGD("read styleName at 0x%x", idx);
    len = a[idx++];
    _styleName = new char[len + 1];

    for (int i = 0; i < len; ++i) {
        _styleName[i] = a[idx++];
    }
    _styleName[len] = 0;

    LOGD("style name <%s>", _styleName);

    // read midi events
    int eventStart = a[size - 4] + a[size - 3] * 256;
    int eventCount = a[size - 2] + a[size - 1] * 256;

    Fraction endTick = Fraction::fromTicks(_measures * bbDivision * 4 * timesigZ() / timesigN());

    if (eventCount == 0) {
        LOGD("no melody");
        return true;
    } else {
        idx = eventStart;
        LOGD("melody found at 0x%x", idx);
        int i = 0;
        int lastLen = 0;
        for (i = 0; i < eventCount; ++i, idx+=12) {
            int type = a[idx + 4] & 0xf0;
            if (type == 0x90) {
                int channel = a[idx + 7];
                BBTrack* track = 0;
                foreach (BBTrack* t, _tracks) {
                    if (t->outChannel() == channel) {
                        track = t;
                        break;
                    }
                }
                if (track == 0) {
                    track = new BBTrack(this);
                    track->setOutChannel(channel);
                    _tracks.append(track);
                }
                Fraction tick = Fraction::fromTicks(a[idx] + (a[idx + 1] << 8) + (a[idx + 2] << 16) + (a[idx + 3] << 24));
                tick -= Fraction::fromTicks(4 * bbDivision);
                if (tick >= endTick) {
                    LOGD("event tick %d > %d", tick.ticks(), endTick.ticks());
                    continue;
                }
                Event note(ME_NOTE);
                note.setOntime((tick.ticks() * Constants::DIVISION) / bbDivision);
                note.setPitch(a[idx + 5]);
                note.setVelo(a[idx + 6]);
                note.setChannel(channel);
                int len1 = a[idx + 8] + (a[idx + 9] << 8) + (a[idx + 10] << 16) + (a[idx + 11] << 24);
                if (len1 == 0) {
                    if (lastLen == 0) {
                        LOGD("note event of len 0 at idx %04x", idx);
                        continue;
                    }
                    len1 = lastLen;
                }
                lastLen = len1;
                note.setDuration((len1* Constants::DIVISION) / bbDivision);
                track->append(note);
            } else if (type == 0xb0 || type == 0xc0) {
                // ignore controller
            } else if (type == 0) {
                break;
            } else {
                LOGD("unknown event type 0x%02x at x%04x", a[idx + 4], idx);
                break;
            }
        }
        LOGD("Events found x%02x (%d)", i, i);
    }
    return true;
}

//---------------------------------------------------------
//   importBB
//---------------------------------------------------------

Err importBB(MasterScore* score, const QString& name)
{
    BBFile bb;
    if (!QFileInfo::exists(name)) {
        return engraving::Err::FileNotFound;
    }
    if (!bb.read(name)) {
        LOGD("Cannot open file <%s>", qPrintable(name));
        return engraving::Err::FileOpenError;
    }
    score->style().set(Sid::chordsXmlFile, true);
    score->chordList()->read(score->configuration()->appDataPath(), u"chords.xml");
    *(score->sigmap()) = bb.siglist();

    QList<BBTrack*>* tracks = bb.tracks();
    int ntracks = tracks->size();
    if (ntracks == 0) {             // no events?
        ntracks = 1;
    }
    for (int i = 0; i < ntracks; ++i) {
        Part* part = new Part(score);
        Staff* s = Factory::createStaff(part);
        score->appendStaff(s);
        score->appendPart(part);
    }

    //---------------------------------------------------
    //  create measures
    //---------------------------------------------------

    for (int i = 0; i < bb.measures(); ++i) {
        Measure* measure  = Factory::createMeasure(score->dummy()->system());
        Fraction tick = Fraction::fromTicks(score->sigmap()->bar2tick(i, 0));
        measure->setTick(tick);
        Fraction ts = score->sigmap()->timesig(tick.ticks()).timesig();
        measure->setTimesig(ts);
        measure->setTicks(ts);
        score->measures()->add(measure);
    }

    //---------------------------------------------------
    //  create notes
    //---------------------------------------------------

    foreach (BBTrack* track, * tracks) {
        track->cleanup();
    }

    if (tracks->isEmpty()) {
        for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
            if (mb->type() != ElementType::MEASURE) {
                continue;
            }
            Measure* measure = (Measure*)mb;
            Segment* s = measure->getSegment(SegmentType::ChordRest, measure->tick());
            Rest* rest = Factory::createRest(s, TDuration(DurationType::V_MEASURE));
            rest->setTicks(measure->ticks());
            rest->setTrack(0);
            s->add(rest);
        }
    } else {
        int staffIdx = 0;
        foreach (BBTrack* track, * tracks) {
            bb.convertTrack(score, track, staffIdx++);
        }
    }

    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (mb->type() != ElementType::MEASURE) {
            continue;
        }
        Measure* measure = (Measure*)mb;
        Segment* s = measure->findSegment(SegmentType::ChordRest, measure->tick());
        if (s == 0) {
            Rest* rest = Factory::createRest(s, TDuration(DurationType::V_MEASURE));
            rest->setTicks(measure->ticks());
            rest->setTrack(0);
            Segment* s1 = measure->getSegment(SegmentType::ChordRest, measure->tick());
            s1->add(rest);
        }
    }

    score->spell();

    //---------------------------------------------------
    //    create title
    //---------------------------------------------------

    MeasureBase* measureB = score->first();
    Text* text = Factory::createText(measureB, TextStyleType::TITLE);
    text->setPlainText(String::fromUtf8(bb.title()));

    if (measureB->type() != ElementType::VBOX) {
        measureB = Factory::createTitleVBox(score->dummy()->system());
        measureB->setNext(score->first());
        score->measures()->add(measureB);
    }
    measureB->add(text);

    //---------------------------------------------------
    //    create chord symbols
    //---------------------------------------------------

    static const int table[] = {
        //C  Db, D,  Eb,  E, F, Gb, G,  Ab, A,  Bb, B,  C#, D#, F#  G#  A#
        14, 9, 16, 11, 18, 13, 8, 15, 10, 17, 12, 19, 21, 23, 20, 22, 24
    };
    foreach (const BBChord& c, bb.chords()) {
        Fraction tick = Fraction(c.beat, 4);          // c.beat  * Constants::division;
// LOGD("CHORD %d %d", c.beat, tick);
        Measure* m = score->tick2measure(tick);
        if (m == 0) {
            LOGD("import BB: measure for tick %d not found", tick.ticks());
            continue;
        }
        Segment* s = m->getSegment(SegmentType::ChordRest, tick);
        Harmony* h = Factory::createHarmony(s);
        h->setTrack(0);
        h->setRootTpc(table[c.root - 1]);
        if (c.bass > 0) {
            h->setBaseTpc(table[c.bass - 1]);
        } else {
            h->setBaseTpc(Tpc::TPC_INVALID);
        }
        h->setId(c.extension);
        h->getDescription();
        h->render();
        s->add(h);
    }

    //---------------------------------------------------
    //    insert layout breaks
    //    add chorus repeat
    //---------------------------------------------------

    int startChorus = bb.startChorus() - 1;
    int endChorus   = bb.endChorus() - 1;

    int n = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (mb->type() != ElementType::MEASURE) {
            continue;
        }
        Measure* measure = (Measure*)mb;
        if (n && (n % 4) == 0) {
            LayoutBreak* lb = Factory::createLayoutBreak(measure);
            lb->setLayoutBreakType(LayoutBreakType::LINE);
            measure->add(lb);
        }
        if (startChorus == n) {
            measure->setRepeatStart(true);
        } else if (endChorus == n) {
            measure->setRepeatEnd(true);
            measure->setRepeatCount(bb.repeats());
        }
        ++n;
    }

    for (Staff* staff : score->staves()) {
        Fraction tick = Fraction(0, 1);
        KeySigEvent ke;
        Key key = Key(bb.key());
        Key cKey = key;
        Interval v = staff->part()->instrument(tick)->transpose();
        if (!v.isZero() && !score->style().styleB(Sid::concertPitch)) {
            cKey = transposeKey(key, v);
            // if there are more than 6 accidentals in transposing key, it cannot be PreferSharpFlat::AUTO
            if ((key > 6 || key < -6) && staff->part()->preferSharpFlat() == PreferSharpFlat::AUTO) {
                staff->part()->setPreferSharpFlat(PreferSharpFlat::NONE);
            }
        }
        ke.setConcertKey(cKey);
        ke.setKey(key);
        staff->setKey(tick, ke);
        Measure* mks = score->tick2measure(tick);
        Segment* sks = mks->getSegment(SegmentType::KeySig, tick);
        KeySig* keysig = Factory::createKeySig(sks);
        keysig->setTrack((static_cast<int>(score->staffIdx(staff->part())) + staff->rstaff()) * VOICES);
        keysig->setKey(cKey, key);
        sks->add(keysig);
    }
    score->setUpTempoMap();
    return engraving::Err::NoError;
}

//---------------------------------------------------------
//   processPendingNotes
//---------------------------------------------------------

Fraction BBFile::processPendingNotes(Score* score, QList<MNote*>* notes, const Fraction& l, int track)
{
    Fraction len(l);
    Staff* cstaff                = score->staff(track / VOICES);
    const Instrument* instrument = cstaff->part()->instrument();
    const Drumset* drumset       = instrument->drumset();
    bool useDrumset              = instrument->useDrumset();
    Fraction tick                = Fraction::fromTicks(notes->at(0)->mc.ontime());

    //
    // look for len of shortest note
    //
    for (const MNote* n : *notes) {
        if (n->mc.duration() < len.ticks()) {
            len = Fraction::fromTicks(n->mc.duration());
        }
    }

    //
    // split notes on measure boundary
    //
    Measure* measure = score->tick2measure(tick);
    if (measure == 0 || (tick >= measure->endTick())) {
        LOGD("no measure found for tick %d", tick.ticks());
        notes->clear();
        return len;
    }
    if ((tick + len) > measure->endTick()) {
        len = measure->endTick() - tick;
    }

    Segment* s = measure->getSegment(SegmentType::ChordRest, tick);
    Chord* chord = Factory::createChord(s);
    chord->setTrack(track);
    TDuration d;
    d.setVal(len.ticks());
    chord->setDurationType(d);
    chord->setTicks(d.fraction());
    s->add(chord);

    for (MNote* n : *notes) {
        std::vector<Event>& nl = n->mc.notes();
        for (size_t i = 0; i < nl.size(); ++i) {
            const Event& mn = nl[i];
            Note* note = Factory::createNote(chord);
            note->setPitch(mn.pitch(), mn.tpc(), mn.tpc());
            note->setTrack(track);
            chord->add(note);

            if (useDrumset) {
                if (!drumset->isValid(mn.pitch())) {
                    LOGD("unmapped drum note 0x%02x %d", mn.pitch(), mn.pitch());
                } else {
                    chord->setStemDirection(drumset->stemDirection(mn.pitch()));
                }
            }
            if (n->ties[static_cast<int>(i)]) {
                n->ties[static_cast<int>(i)]->setEndNote(note);
                n->ties[static_cast<int>(i)]->setTrack(note->track());
                note->setTieBack(n->ties[static_cast<int>(i)]);
            }
        }
        if (n->mc.duration() <= len.ticks()) {
            notes->removeAt(notes->indexOf(n));
            continue;
        }
        for (size_t i = 0; i < nl.size(); ++i) {
            const Event& mn = nl[i];
            Note* note = chord->findNote(mn.pitch());
            n->ties[static_cast<int>(i)] = Factory::createTie(score->dummy());
            n->ties[static_cast<int>(i)]->setStartNote(note);
            note->setTieFor(n->ties[static_cast<int>(i)]);
        }
        n->mc.setOntime(n->mc.ontime() + len.ticks());
        n->mc.setLen(n->mc.duration() - len.ticks());
    }
    return len;
}

//---------------------------------------------------------
//   collectNotes
//---------------------------------------------------------

static ciEvent collectNotes(const Fraction& tick, int voice, ciEvent i, const EventList* el, QList<MNote*>* notes)
{
    for (; i != el->end(); ++i) {
        const Event& e = *i;
        if (e.type() != ME_CHORD) {
            continue;
        }
        if (e.voice() != voice) {
            continue;
        }
        if (e.ontime() > tick.ticks()) {
            break;
        }
        if (e.ontime() < tick.ticks()) {
            continue;
        }
        MNote* n = new MNote(e);
        notes->append(n);
    }
    return i;
}

//---------------------------------------------------------
//   convertTrack
//---------------------------------------------------------

void BBFile::convertTrack(Score* score, BBTrack* track, int staffIdx)
{
    track->findChords();
    int voices         = track->separateVoices(2);
    const EventList el = track->events();

    for (int voice = 0; voice < voices; ++voice) {
        int tr = staffIdx * VOICES + voice;
        QList<MNote*> notes;

        Fraction ctick = Fraction(0, 1);
        ciEvent i = collectNotes(ctick, voice, el.begin(), &el, &notes);

        for (; i != el.end();) {
            const Event& e = *i;
            if (e.type() != ME_CHORD || e.voice() != voice) {
                ++i;
                continue;
            }
            //
            // process pending notes
            //
            Fraction restLen = Fraction::fromTicks(e.ontime()) - ctick;
// LOGD("ctick %d  rest %d ontick %d size %d", ctick, restLen, e.ontime(), notes.size());

            if (restLen <= Fraction(0, 1)) {
                ASSERT_X(QString::asprintf("bad restlen ontime %d - ctick %d", e.ontime(), ctick.ticks()));
            }

            while (!notes.isEmpty()) {
                Fraction len = processPendingNotes(score, &notes, restLen, tr);
                if (len.isZero()) {
                    LOGD("processPendingNotes returns zero, restlen %d, track %d", restLen.ticks(), tr);
                    ctick += restLen;
                    restLen = Fraction(0, 1);
                    break;
                }
                ctick += len;
                restLen -= len;
            }
// LOGD("  1.ctick %d  rest %d", ctick, restLen);
            //
            // check for gap and fill with rest
            //
            if (voice == 0) {
                while (restLen > Fraction(0, 1)) {
                    Fraction len = restLen;
                    Measure* measure = score->tick2measure(ctick);
                    if (measure == 0 || (ctick >= measure->endTick())) {                 // at end?
                        ctick += len;
                        restLen -= len;
                        break;
                    }
                    // split rest on measure boundary
                    if ((ctick + len) > measure->endTick()) {
                        len = measure->endTick() - ctick;
                        if (len <= Fraction(0, 1)) {
                            LOGD("bad len %d", len.ticks());
                            break;
                        }
                    }
                    TDuration d;
                    d.setVal(len.ticks());
                    Segment* s = measure->getSegment(SegmentType::ChordRest, ctick);
                    Rest* rest = Factory::createRest(s, d);
                    rest->setTicks(d.fraction());
                    rest->setTrack(staffIdx * VOICES);
                    s->add(rest);
// LOGD("   add rest %d", len);

                    ctick   += len;
                    restLen -= len;
                }
            } else {
                ctick += restLen;
            }

// LOGD("  2.ctick %d  rest %d", ctick, restLen);
            //
            // collect all notes at ctick
            //
            i = collectNotes(ctick, voice, i, &el, &notes);
        }

        //
        // process pending notes
        //
        while (!notes.isEmpty()) {
            Fraction len = processPendingNotes(score, &notes, Fraction(0x7fffffff, 1), tr);
            ctick += len;
        }
        if (voice == 0) {
            Measure* measure = score->tick2measure(ctick);
            if (measure && (ctick < measure->endTick())) {             // at end?
                TDuration d;
                d.setVal((measure->endTick() - ctick).ticks());
                Segment* s = measure->getSegment(SegmentType::ChordRest, ctick);
                Rest* rest = Factory::createRest(s, d);
                rest->setTicks(d.fraction());
                rest->setTrack(staffIdx * VOICES);
                s->add(rest);
            }
        }
    }
}

//---------------------------------------------------------
//   quantize
//    process one segment (measure)
//---------------------------------------------------------

void BBTrack::quantize(int startTick, int endTick, EventList* dst)
{
    int mintick = Constants::DIVISION * 64;
    iEvent i = _events.begin();
    for (; i != _events.end(); ++i) {
        if (i->ontime() >= startTick) {
            break;
        }
    }
    iEvent si = i;
    for (; i != _events.end(); ++i) {
        const Event& e = *i;
        if (e.ontime() >= endTick) {
            break;
        }
        if (e.type() == ME_NOTE && (e.duration() < mintick)) {
            mintick = e.duration();
        }
    }
    if (mintick <= Constants::DIVISION / 16) {        // minimum duration is 1/64
        mintick = Constants::DIVISION / 16;
    } else if (mintick <= Constants::DIVISION / 8) {
        mintick = Constants::DIVISION / 8;
    } else if (mintick <= Constants::DIVISION / 4) {
        mintick = Constants::DIVISION / 4;
    } else if (mintick <= Constants::DIVISION / 2) {
        mintick = Constants::DIVISION / 2;
    } else if (mintick <= Constants::DIVISION) {
        mintick = Constants::DIVISION;
    } else if (mintick <= Constants::DIVISION* 2) {
        mintick = Constants::DIVISION * 2;
    } else if (mintick <= Constants::DIVISION* 4) {
        mintick = Constants::DIVISION * 4;
    } else if (mintick <= Constants::DIVISION* 8) {
        mintick = Constants::DIVISION * 8;
    }
    int raster;
    if (mintick > Constants::DIVISION) {
        raster = Constants::DIVISION;
    } else {
        raster = mintick;
    }

    //
    //  quantize onset
    //
    for (i = si; i != _events.end(); ++i) {
        Event e = *i;
        if (e.ontime() >= endTick) {
            break;
        }
        if (e.type() == ME_NOTE) {
            // prefer moving note to the right
            int tick = ((e.ontime() + raster / 2) / raster) * raster;
            int diff = tick - e.ontime();
            int len  = e.duration() - diff;
            e.setOntime(tick);
            e.setLen(len);
        }
        dst->insert(e);
    }
    //
    //  quantize duration
    //
    for (i = dst->begin(); i != dst->end(); ++i) {
        Event& e = *i;
        if (e.type() != ME_NOTE) {
            continue;
        }
        int tick   = e.ontime();
        int len    = e.duration();
        int ntick  = tick + len;
        int nntick = -1;
        for (iEvent ii = (i + 1); ii != dst->end(); ++ii) {
            if (ii->type() == ME_NOTE) {
                const Event& ee = *ii;
                if (ee.ontime() == tick) {
                    continue;
                }
                nntick = ee.ontime();
                break;
            }
        }
        if (nntick == -1) {
            len = quantizeLen(len, raster);
        } else {
            int diff = nntick - ntick;
            if (diff > 0) {
                // insert rest?
                if (diff <= raster) {
                    len = nntick - tick;
                } else {
                    len = quantizeLen(len, raster);
                }
            } else {
                if (diff > -raster) {
                    len = nntick - tick;
                } else {
                    len = quantizeLen(len, raster);
                }
            }
        }
        e.setLen(len);
    }
}

//---------------------------------------------------------
//   cleanup
//    - quantize
//    - remove overlaps
//---------------------------------------------------------

void BBTrack::cleanup()
{
    EventList dl;

    //
    // quantize
    //
    int lastTick = 0;
    for (const Event& e : _events) {
        if (e.type() != ME_NOTE) {
            continue;
        }
        int offtime  = e.offtime();
        if (offtime > lastTick) {
            lastTick = offtime;
        }
    }
    int startTick = 0;
    for (int i = 1;; ++i) {
        int endTick = bb->siglist().bar2tick(i, 0);
        quantize(startTick, endTick, &dl);
        if (endTick > lastTick) {
            break;
        }
        startTick = endTick;
    }

    //
    //
    //
    _events.clear();

    for (iEvent i = dl.begin(); i != dl.end(); ++i) {
        Event& e = *i;
        if (e.type() == ME_NOTE) {
            iEvent ii = i;
            ++ii;
            for (; ii != dl.end(); ++ii) {
                const Event& ee = *ii;
                if (ee.type() != ME_NOTE || ee.pitch() != e.pitch()) {
                    continue;
                }
                if (ee.ontime() >= e.ontime() + e.duration()) {
                    break;
                }
                e.setLen(ee.ontime() - e.ontime());
                break;
            }
            if (e.duration() <= 0) {
                continue;
            }
        }
        _events.insert(e);
    }
}

//---------------------------------------------------------
//   findChords
//---------------------------------------------------------

void BBTrack::findChords()
{
    EventList dl;
    size_t n = _events.size();

    Drumset* drumset;
    if (_drumTrack) {
        drumset = smDrumset;
    } else {
        drumset = 0;
    }
    int jitter = 3;     // tick tolerance for note on/off

    for (size_t i = 0; i < n; ++i) {
        Event e = _events[i];
        if (e.type() == ME_INVALID) {
            continue;
        }
        if (e.type() != ME_NOTE) {
            dl.push_back(e);
            continue;
        }

        Event note(e);
        int ontime       = note.ontime();
        int offtime      = note.offtime();
        Event chord(ME_CHORD);
        chord.setOntime(ontime);
        chord.setLen(note.duration());
        chord.notes().push_back(note);
        int voice = 0;
        chord.setVoice(voice);
        dl.push_back(chord);
        _events[i].setType(ME_INVALID);

        bool useDrumset = false;
        if (drumset) {
            int pitch = note.pitch();
            if (drumset->isValid(pitch)) {
                useDrumset = true;
                voice = drumset->voice(pitch);
                chord.setVoice(voice);
            }
        }
        for (size_t k = i + 1; k < n; ++k) {
            if (_events[k].type() != ME_NOTE) {
                continue;
            }
            Event nn = _events[k];
            if (nn.ontime() - jitter > ontime) {
                break;
            }
            if (qAbs(nn.ontime() - ontime) > jitter || qAbs(nn.offtime() - offtime) > jitter) {
                continue;
            }
            int pitch = nn.pitch();
            if (useDrumset) {
                if (drumset->isValid(pitch) && drumset->voice(pitch) == voice) {
                    chord.notes().push_back(nn);
                    _events[k].setType(ME_INVALID);
                }
            } else {
                chord.notes().push_back(nn);
                _events[k].setType(ME_INVALID);
            }
        }
    }
    _events = dl;
}

//---------------------------------------------------------
//   separateVoices
//---------------------------------------------------------

int BBTrack::separateVoices(int /*maxVoices*/)
{
    return 1;
}
}
