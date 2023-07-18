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

#include "musedata.h"

#include "libmscore/factory.h"
#include "libmscore/masterscore.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/barline.h"
#include "libmscore/clef.h"
#include "libmscore/key.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/text.h"
#include "libmscore/bracket.h"
#include "libmscore/tuplet.h"
#include "libmscore/slur.h"
#include "libmscore/dynamic.h"
#include "libmscore/lyrics.h"
#include "libmscore/articulation.h"
#include "libmscore/sig.h"
#include "libmscore/measure.h"
#include "libmscore/timesig.h"
#include "libmscore/segment.h"

#include "log.h"

using namespace mu::iex::musedata;
using namespace mu::engraving;

//---------------------------------------------------------
//   musicalAttribute
//---------------------------------------------------------

void MuseData::musicalAttribute(QString s, Part* part)
{
    QStringList al = s.mid(2).split(" ", Qt::SkipEmptyParts);
    foreach (QString item, al) {
        if (item.startsWith("K:")) {
            Key key = Key(item.midRef(2).toInt());
            KeySigEvent ke;
            Interval v = part->instrument(curTick)->transpose();
            ke.setConcertKey(key);
            if (!v.isZero() && !score->style().styleB(Sid::concertPitch)) {
                v.flip();
                ke.setKey(transposeKey(key, v));
            }
            for (Staff* staff : part->staves()) {
                staff->setKey(curTick, ke);
            }
        } else if (item.startsWith("Q:")) {
            _division = item.midRef(2).toInt();
        } else if (item.startsWith("T:")) {
            QStringList tl = item.mid(2).split("/");
            if (tl.size() != 2) {
                LOGD() << "bad time sig: " << item;
                continue;
            }
            int z = tl[0].toInt();
            int n = tl[1].toInt();
            if ((z > 0) && (n > 0)) {
                Measure* mes = score->tick2measure(curTick);
                Segment* seg = mes->getSegment(SegmentType::TimeSig, curTick);
                TimeSig* ts = Factory::createTimeSig(seg);
                Staff* staff = part->staff(0);
                ts->setTrack(staff->idx() * VOICES);
                ts->setSig(Fraction(z, n));
                seg->add(ts);
            }
        } else if (item.startsWith("X:")) {
        } else if (item[0] == 'C') {
            int staffIdx = 1;
            int col = 2;
            if (item[1].isDigit()) {
                staffIdx = item.midRef(1, 1).toInt();
                col = 3;
            }
            staffIdx -= 1;
            int clefCode = item.mid(col).toInt();
            ClefType clefType = ClefType::G;
            switch (clefCode) {
            // G clef
            case 04: clefType = ClefType::G;
                break;
            case 05: clefType = ClefType::G_1;
                break;

            // C clef
            case 11: clefType = ClefType::C5;
                break;
            case 12: clefType = ClefType::C4;
                break;
            case 13: clefType = ClefType::C3;
                break;
            case 14: clefType = ClefType::C2;
                break;
            case 15: clefType = ClefType::C1;
                break;

            // F clef
            case 21: clefType = ClefType::F_C;
                break;
            case 22: clefType = ClefType::F;
                break;
            case 23: clefType = ClefType::F_B;
                break;

            // G clef 8vb
            case 34: clefType = ClefType::G8_VB;
                break;

            // F clef 8vb
            case 52: clefType = ClefType::F8_VB;
                break;

            // G clef 8va
            case 64: clefType = ClefType::G8_VA;
                break;

            // F clef 8va
            case 82: clefType = ClefType::F_8VA;
                break;

            default:
                LOGD() << "unknown clef code: " << clefCode;
                break;
            }

            Measure* mes = score->tick2measure(curTick);
            Segment* seg = mes->getSegment(SegmentType::Clef, curTick);
            Staff* staff = part->staff(staffIdx);
            Clef* clef = Factory::createClef(seg);
            clef->setTrack(staff->idx() * VOICES);
            clef->setClefType(clefType);
            seg->add(clef);
        } else {
            LOGD() << "unknown $key: " << item;
        }
    }
}

//---------------------------------------------------------
//   readChord
//---------------------------------------------------------

void MuseData::readChord(Part*, const QString& s)
{
    //                       a  b   c  d  e  f  g
    static int table[7]  = { 9, 11, 0, 2, 4, 5, 7 };

    int step  = s[1].toLatin1() - 'A';
    int alter = 0;
    int octave = 0;
    for (int i = 2; i < 4; ++i) {
        if (s[i] == '#') {
            alter += 1;
        } else if (s[i] == 'f') {
            alter -= 1;
        } else if (s[i].isDigit()) {
            octave = s.midRef(i, 1).toInt();
            break;
        }
    }
    int staffIdx = 0;
    if (s.size() >= 24) {
        if (s[23].isDigit()) {
            staffIdx = s.midRef(23, 1).toInt() - 1;
        }
    }
    int pitch = table[step] + alter + (octave + 1) * 12;
    if (pitch < 0) {
        pitch = 0;
    }
    if (pitch > 127) {
        pitch = 127;
    }

    Chord* chord = (Chord*)chordRest;
    Note* note = Factory::createNote(chord);
    note->setPitch(pitch);
    note->setTpcFromPitch();
    note->setTrack(staffIdx * VOICES + voice);
    chord->add(note);
}

//---------------------------------------------------------
//   openSlur
//---------------------------------------------------------

void MuseData::openSlur(int idx, const Fraction& tick, Staff* staff, int voc)
{
    staff_idx_t staffIdx = staff->idx();
    if (slur[idx]) {
        LOGD("%06d: slur %d already open", tick.ticks(), idx + 1);
        return;
    }
    slur[idx] = Factory::createSlur(score->dummy());
    slur[idx]->setTick(tick);
    slur[idx]->setTrack(staffIdx * VOICES + voc);
    score->addElement(slur[idx]);
}

//---------------------------------------------------------
//   closeSlur
//---------------------------------------------------------

void MuseData::closeSlur(int idx, const Fraction& tick, Staff* staff, int voc)
{
    staff_idx_t staffIdx = staff->idx();
    if (slur[idx]) {
        slur[idx]->setTick2(tick);
        slur[idx]->setTrack2(staffIdx * VOICES + voc);
        slur[idx] = 0;
    } else {
        LOGD("%06d: slur %d not open", tick.ticks(), idx + 1);
    }
}

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

void MuseData::readNote(Part* part, const QString& s)
{
    //                       a  b   c  d  e  f  g
    static int table[7]  = { 9, 11, 0, 2, 4, 5, 7 };

    int step  = s[0].toLatin1() - 'A';
    int alter = 0;
    int octave = 0;
    for (int i = 1; i < 3; ++i) {
        if (s[i] == '#') {
            alter += 1;
        } else if (s[i] == 'f') {
            alter -= 1;
        } else if (s[i].isDigit()) {
            octave = s.midRef(i, 1).toInt();
            break;
        }
    }
    DirectionV dir = DirectionV::AUTO;
    if (s.size() >= 23) {
        if (s[22] == 'u') {
            dir = DirectionV::UP;
        } else if (s[22] == 'd') {
            dir = DirectionV::DOWN;
        }
    }

    int staffIdx = 0;
    if (s.size() >= 24) {
        if (s[23].isDigit()) {
            staffIdx = s.midRef(23, 1).toInt() - 1;
        }
    }
    Staff* staff = part->staff(staffIdx);
    staff_idx_t gstaff = staff->idx();

    int pitch = table[step] + alter + (octave + 1) * 12;
    if (pitch < 0) {
        pitch = 0;
    }
    if (pitch > 127) {
        pitch = 127;
    }
    Fraction ticks = Fraction::fromTicks((s.midRef(5, 3).toInt() * Constants::DIVISION + _division / 2) / _division);
    Fraction tick  = curTick;
    curTick  += ticks;

    Tuplet* tuplet = 0;
    if (s.size() >= 22) {
        int a = 1;
        int b = 1;
        if (s[19] != ' ') {
            a = s[19].toLatin1() - '0';
            if (a == 3 && s[20] != ':') {
                b = 2;
            } else {
                b = s[21].toLatin1() - '0';
            }
        }
        if (a == 3 && b == 2) {           // triplet
            if (chordRest && chordRest->tuplet() && ntuplet) {
                tuplet = chordRest->tuplet();
            } else {
                tuplet = new Tuplet(measure);
                tuplet->setTrack(gstaff * VOICES);
                tuplet->setTick(tick);
                ntuplet = a;
                tuplet->setRatio(Fraction(a, b));
                measure->add(tuplet);
            }
        } else if (a == 1 && b == 1) {
        } else {
            LOGD("unsupported tuple %d/%d", a, b);
        }
    }

    Chord* chord = Factory::createChord(score->dummy()->segment());
    chordRest = chord;
    chord->setTrack(gstaff * VOICES);
    chord->setStemDirection(dir);
    if (tuplet) {
        chord->setTuplet(tuplet);
        tuplet->add(chord);
        --ntuplet;
    }
    TDuration d;
    d.setVal(ticks.ticks());
    chord->setDurationType(d);

    Segment* segment = measure->getSegment(SegmentType::ChordRest, tick);

    voice = 0;
    for (; voice < static_cast<int>(VOICES); ++voice) {
        EngravingItem* e = segment->element(gstaff * VOICES + voice);
        if (e == 0) {
            chord->setTrack(gstaff * VOICES + voice);
            segment->add(chord);
            break;
        }
    }
    if (voice == VOICES) {
        LOGD("cannot allocate voice");
        delete chord;
        return;
    }
    Note* note = Factory::createNote(chord);
    note->setPitch(pitch);
    note->setTpcFromPitch();
    note->setTrack(gstaff * VOICES + voice);
    chord->add(note);

    QString dynamics;
    QString an = s.mid(31, 11);
    for (int i = 0; i < an.size(); ++i) {
        if (an[i] == '(') {
            openSlur(0, tick, staff, voice);
        } else if (an[i] == ')') {
            closeSlur(0, tick, staff, voice);
        } else if (an[i] == '[') {
            openSlur(1, tick, staff, voice);
        } else if (an[i] == ']') {
            closeSlur(1, tick, staff, voice);
        } else if (an[i] == '{') {
            openSlur(2, tick, staff, voice);
        } else if (an[i] == '}') {
            closeSlur(2, tick, staff, voice);
        } else if (an[i] == 'z') {
            openSlur(3, tick, staff, voice);
        } else if (an[i] == 'x') {
            closeSlur(3, tick, staff, voice);
        } else if (an[i] == '.') {
            Articulation* atr = Factory::createArticulation(chord);
            atr->setSymId(SymId::articStaccatoAbove);
            chord->add(atr);
        } else if (an[i] == '_') {
            Articulation* atr = Factory::createArticulation(chord);
            atr->setSymId(SymId::articTenutoAbove);
            chord->add(atr);
        } else if (an[i] == 'v') {
            Articulation* atr = Factory::createArticulation(chord);
            atr->setSymId(SymId::stringsUpBow);
            chord->add(atr);
        } else if (an[i] == 'n') {
            Articulation* atr = Factory::createArticulation(chord);
            atr->setSymId(SymId::stringsDownBow);
            chord->add(atr);
        } else if (an[i] == 't') {
            Articulation* atr = Factory::createArticulation(chord);
            atr->setSymId(SymId::ornamentTrill);
            chord->add(atr);
        } else if (an[i] == 'F') {
            Articulation* atr = Factory::createArticulation(chord);
            atr->setUp(true);
            atr->setSymId(SymId::fermataAbove);
            chord->add(atr);
        } else if (an[i] == 'E') {
            Articulation* atr = Factory::createArticulation(chord);
            atr->setUp(false);
            atr->setSymId(SymId::fermataBelow);
            chord->add(atr);
        } else if (an[i] == 'O') {
            // Articulation* atr = new Articulation(score);
            // atr->setArticulationType(ArticulationType::Downbow);
            // chord->add(atr);
            LOGD("%06d: open string '%c' not implemented", tick.ticks(), an[i].toLatin1());
        } else if (an[i] == '&') {
            // skip editorial level
            if (i <= an.size() && an[i + 1].isDigit()) {
                ++i;
            }
        } else if (an[i] == 'p') {
            dynamics += "p";
        } else if (an[i] == 'm') {
            dynamics += "m";
        } else if (an[i] == 'f') {
            dynamics += "f";
        } else if (an[i] == '-') {        // tie
        } else if (an[i] == '*') {        // start tuplet
        } else if (an[i] == '!') {        // stop tuplet
        } else if (an[i] == '+') {        // cautionary accidental
        } else if (an[i] == 'X') {        // ???
        } else if (an[i] == ' ') {
        } else {
            LOGD("%06d: notation '%c' not implemented", tick.ticks(), an[i].toLatin1());
        }
    }
    if (!dynamics.isEmpty()) {
        Segment* seg = measure->getSegment(SegmentType::ChordRest, tick);
        Dynamic* dyn = Factory::createDynamic(seg);
        dyn->setDynamicType(dynamics);
        dyn->setTrack(gstaff * VOICES);
        seg->add(dyn);
    }

    QString txt = s.mid(43, 36);
    if (!txt.isEmpty()) {
        QStringList sl = txt.split("|");
        int no = 0;
        for (QString w : sl) {
            w = diacritical(w);
            Lyrics* l = Factory::createLyrics(chord);
            l->setPlainText(w);
            l->setNo(no++);
            l->setTrack(gstaff * VOICES);
            chord->add(l);
        }
    }
}

//---------------------------------------------------------
//   diacritical
// TODO: not complete
//---------------------------------------------------------

QString MuseData::diacritical(QString s)
{
    struct TAB {
        const char* a;
        const char* b;
    } tab[] = {
        { "\\\\", "\\" },
        { "\\2s", "ß" },
        { "\\3a", "ä" },
        { "\\3o", "ö" },
        { "\\3u", "ü" },

        { "\\s2", "ß" },
        { "\\a3", "ä" },
        { "\\o3", "ö" },
        { "\\u3", "ü" },
    };
    for (unsigned int i = 0; i < sizeof(tab) / sizeof(*tab); ++i) {
        s = s.replace(tab[i].a, QString::fromUtf8(tab[i].b));
    }
    return s;
}

//---------------------------------------------------------
//   readRest
//---------------------------------------------------------

void MuseData::readRest(Part* part, const QString& s)
{
    Fraction ticks = Fraction::fromTicks((s.midRef(5, 3).toInt() * Constants::DIVISION + _division / 2) / _division);

    Fraction tick  = curTick;
    curTick  += ticks;

    int staffIdx = 0;
    if (s.size() >= 24) {
        if (s[23].isDigit()) {
            staffIdx = s.midRef(23, 1).toInt() - 1;
        }
    }
    Staff* staff = part->staff(staffIdx);
    staff_idx_t gstaff = staff->idx();

    TDuration d;
    d.setVal(ticks.ticks());
    Rest* rest = Factory::createRest(score->dummy()->segment(), d);
    rest->setTicks(d.fraction());
    chordRest  = rest;
    rest->setTrack(gstaff * VOICES);
    Segment* segment = measure->getSegment(SegmentType::ChordRest, tick);

    voice = 0;
    for (; voice < static_cast<int>(VOICES); ++voice) {
        EngravingItem* e = segment->element(gstaff * VOICES + voice);
        if (e == 0) {
            rest->setTrack(gstaff * VOICES + voice);
            segment->add(rest);
            break;
        }
    }
    if (voice == VOICES) {
        LOGD("cannot allocate voice");
        delete rest;
        return;
    }
}

//---------------------------------------------------------
//   readBackup
//---------------------------------------------------------

void MuseData::readBackup(const QString& s)
{
    Fraction ticks = Fraction::fromTicks((s.midRef(5, 3).toInt() * Constants::DIVISION + _division / 2) / _division);
    if (s[0] == 'b') {
        curTick  -= ticks;
    } else {
        curTick += ticks;
    }
}

//---------------------------------------------------------
//   createMeasure
//---------------------------------------------------------

Measure* MuseData::createMeasure()
{
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (mb->type() != ElementType::MEASURE) {
            continue;
        }
        Measure* m = (Measure*)mb;
        Fraction st = m->tick();
        Fraction l  = m->ticks();
        if (curTick == st) {
            return m;
        }
        if (curTick > st && curTick < (st + l)) {
            // irregular measure
            break;
        }
        if (curTick < st + l) {
            LOGD("cannot create measure at %d", curTick.ticks());
            return 0;
        }
    }
    Measure* mes  = Factory::createMeasure(score->dummy()->system());
    mes->setTick(curTick);

    score->measures()->add(mes);
    return mes;
}

//---------------------------------------------------------
//   readPart
//---------------------------------------------------------

void MuseData::readPart(QStringList sl, Part* part)
{
    int line = 10;
    QString s;
    for (; line < sl.size(); ++line) {
        s = sl[line];
        if (!s.isEmpty() && s[0] == '$') {
            break;
        }
    }
    if (line >= sl.size()) {
        LOGD(" $ not found in part");
        return;
    }
    curTick = Fraction(0, 1);
    slur[0] = 0;
    slur[1] = 0;
    slur[2] = 0;
    slur[3] = 0;
    measure = 0;
    measure = createMeasure();
    for (; line < sl.size(); ++line) {
        s = sl[line];
// LOGD("%6d: <%s>", curTick.ticks(), qPrintable(s));
        char c = s[0].toLatin1();
        switch (c) {
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
            readNote(part, s);
            break;
        case ' ':                   // chord
            readChord(part, s);
            break;
        case 'r':
            readRest(part, s);
            break;
        case 'g':                   // grace note
        case 'c':                   // cue note
        case 'f':                   // basso continuo
            break;
        case 'b':                   // backspace
        case 'i':                   // forward space
            readBackup(s);
            break;
        case 'm':                   // measure line / bar line
            measure = createMeasure();
            break;
        case '*':                   // musical direction
            break;
        case 'P':                   // print suggestion
            break;
        case 'S':                   // sound record
            break;
        case '$':
            musicalAttribute(s, part);
            break;
        default:
            LOGD("unknown record <%s>", qPrintable(s));
            break;
        }
    }
}

//---------------------------------------------------------
//   countStaves
//---------------------------------------------------------

int MuseData::countStaves(const QStringList& sl)
{
    int staves = 1;
    for (int i = 10; i < sl.size(); ++i) {
        QString s = sl[i];
        char c = s[0].toLatin1();
        switch (c) {
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'r':
        {
            int staffIdx = 1;
            if (s.size() >= 24) {
                if (s[23].isDigit()) {
                    staffIdx = s.midRef(23, 1).toInt();
                }
            }
            if (staffIdx > staves) {
                staves = staffIdx;
            }
        }
        break;
        }
    }
    return staves;
}

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

bool MuseData::read(const QString& name)
{
    QFile fp(name);
    if (!fp.open(QIODevice::ReadOnly)) {
        LOGD("Cannot open file <%s>", qPrintable(name));
        return false;
    }
    QTextStream ts(&fp);
    QStringList part;
    bool commentMode = false;
    for (;;) {
        QString s(ts.readLine());
        if (s.isNull()) {
            break;
        }
        if (s.isEmpty()) {
            if (!commentMode) {
                part.append(QString(""));
            }
            continue;
        }
        if (s[0] == '&') {
            commentMode = !commentMode;
            continue;
        }
        if (commentMode) {
            continue;
        }
        if (s[0] == '@') {
            continue;
        }
        if (s[0] == '/') {
            parts.append(part);

            Part* mpart = new Part(score);
            int staves  = countStaves(part);
            for (int i = 0; i < staves; ++i) {
                Staff* staff = Factory::createStaff(mpart);
                score->appendStaff(staff);

                if ((staves == 2) && (i == 0)) {
                    staff->setBracketType(0, BracketType::BRACE);
                    staff->setBracketSpan(0, 2);
                }
            }
            score->appendPart(mpart);
            if (part.size() > 8) {
                mpart->setPlainLongName(part[8]);
            }
            part.clear();
            continue;
        }
        if (s[0] == 'a') {
            part.back().append(s.midRef(1));
            continue;
        }
        part.append(s);
    }
    fp.close();
    return true;
}

//---------------------------------------------------------
//   convert
//---------------------------------------------------------

void MuseData::convert()
{
    for (int pn = 0; pn < parts.size(); ++pn) {
        Part* part = (score->parts())[pn];
        readPart(parts[pn], part);
    }
}
