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

#include "musedata.h"

#include <QFile>

#include "engraving/dom/articulation.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/bracket.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/key.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tuplet.h"

#include "log.h"

using namespace mu::iex::musedata;
using namespace mu::engraving;

//---------------------------------------------------------
//   musicalAttribute
//---------------------------------------------------------

void MuseData::musicalAttribute(QStringView s, Part* part)
{
    QList<QStringView> al = s.mid(2).split(u' ', Qt::SkipEmptyParts);
    foreach (QStringView item, al) {
        if (item.startsWith(u"K:")) {
            Key key = Key(item.mid(2).toInt());
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
        } else if (item.startsWith(u"Q:")) {
            _division = item.mid(2).toInt();
        } else if (item.startsWith(u"T:")) {
            QList<QStringView> tl = item.mid(2).split(u'/');
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
        } else if (item.startsWith(u"X:")) {
        } else if (item[0] == QChar('C')) {
            int staffIdx = 1;
            int col = 2;
            if (item[1].isDigit()) {
                staffIdx = item.mid(1, 1).toInt();
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

void MuseData::readChord(Part*, QStringView s)
{
    //                       a  b   c  d  e  f  g
    static int table[7]  = { 9, 11, 0, 2, 4, 5, 7 };

    int step  = s[1].toLatin1() - 'A';
    int alter = 0;
    int octave = 0;
    for (int i = 2; i < 4; ++i) {
        if (s[i] == QChar('#')) {
            alter += 1;
        } else if (s[i] == QChar('f')) {
            alter -= 1;
        } else if (s[i].isDigit()) {
            octave = s.mid(i, 1).toInt();
            break;
        }
    }
    int staffIdx = 0;
    if (s.size() >= 24) {
        if (s[23].isDigit()) {
            staffIdx = s.mid(23, 1).toInt() - 1;
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

void MuseData::openSlur(int idx, const Fraction& tick, Staff* staff, int voc, EngravingItem* startChord)
{
    staff_idx_t staffIdx = staff->idx();
    if (slur[idx]) {
        LOGD("%06d: slur %d already open", tick.ticks(), idx + 1);
        return;
    }
    slur[idx] = Factory::createSlur(score->dummy());
    slur[idx]->setTick(tick);
    slur[idx]->setTrack(staffIdx * VOICES + voc);
    slur[idx]->setStartElement(startChord);
    score->addElement(slur[idx]);
}

//---------------------------------------------------------
//   closeSlur
//---------------------------------------------------------

void MuseData::closeSlur(int idx, const Fraction& tick, Staff* staff, int voc, engraving::EngravingItem* endChord)
{
    staff_idx_t staffIdx = staff->idx();
    if (slur[idx]) {
        slur[idx]->setTick2(tick);
        slur[idx]->setTrack2(staffIdx * VOICES + voc);
        slur[idx]->setEndElement(endChord);
        slur[idx] = 0;
    } else {
        LOGD("%06d: slur %d not open", tick.ticks(), idx + 1);
    }
}

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

void MuseData::readNote(Part* part, QStringView s)
{
    //                       a  b   c  d  e  f  g
    static int table[7]  = { 9, 11, 0, 2, 4, 5, 7 };

    int step  = s[0].toLatin1() - 'A';
    int alter = 0;
    int octave = 0;
    for (int i = 1; i < 3; ++i) {
        if (s[i] == QChar('#')) {
            alter += 1;
        } else if (s[i] == QChar('f')) {
            alter -= 1;
        } else if (s[i].isDigit()) {
            octave = s.mid(i, 1).toInt();
            break;
        }
    }
    DirectionV dir = DirectionV::AUTO;
    if (s.size() >= 23) {
        if (s[22] == QChar('u')) {
            dir = DirectionV::UP;
        } else if (s[22] == QChar('d')) {
            dir = DirectionV::DOWN;
        }
    }

    int staffIdx = 0;
    if (s.size() >= 24) {
        if (s[23].isDigit()) {
            staffIdx = s.mid(23, 1).toInt() - 1;
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
    Fraction ticks = Fraction::fromTicks((s.mid(5, 3).toInt() * Constants::DIVISION + _division / 2) / _division);
    Fraction tick  = curTick;
    curTick  += ticks;

    Tuplet* tuplet = 0;
    if (s.size() >= 22) {
        int a = 1;
        int b = 1;
        if (s[19] != QChar(' ')) {
            a = s[19].toLatin1() - '0';
            if (a == 3 && s[20] != QChar(':')) {
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
    QStringView an = s.mid(31, 11);
    for (int i = 0; i < an.size(); ++i) {
        char c = an[i].toLatin1();
        if (c == '(') {
            openSlur(0, tick, staff, voice, chord);
        } else if (c == ')') {
            closeSlur(0, tick, staff, voice, chord);
        } else if (c == '[') {
            openSlur(1, tick, staff, voice, chord);
        } else if (c == ']') {
            closeSlur(1, tick, staff, voice, chord);
        } else if (c == '{') {
            openSlur(2, tick, staff, voice, chord);
        } else if (c == '}') {
            closeSlur(2, tick, staff, voice, chord);
        } else if (c == 'z') {
            openSlur(3, tick, staff, voice, chord);
        } else if (c == 'x') {
            closeSlur(3, tick, staff, voice, chord);
        } else if (c == '.') {
            Articulation* atr = Factory::createArticulation(chord);
            atr->setSymId(SymId::articStaccatoAbove);
            chord->add(atr);
        } else if (c == '_') {
            Articulation* atr = Factory::createArticulation(chord);
            atr->setSymId(SymId::articTenutoAbove);
            chord->add(atr);
        } else if (c == 'v') {
            Articulation* atr = Factory::createArticulation(chord);
            atr->setSymId(SymId::stringsUpBow);
            chord->add(atr);
        } else if (c == 'n') {
            Articulation* atr = Factory::createArticulation(chord);
            atr->setSymId(SymId::stringsDownBow);
            chord->add(atr);
        } else if (c == 't') {
            Articulation* atr = Factory::createArticulation(chord);
            atr->setSymId(SymId::ornamentTrill);
            chord->add(atr);
        } else if (c == 'F') {
            Articulation* atr = Factory::createArticulation(chord);
            atr->setUp(true);
            atr->setSymId(SymId::fermataAbove);
            chord->add(atr);
        } else if (c == 'E') {
            Articulation* atr = Factory::createArticulation(chord);
            atr->setUp(false);
            atr->setSymId(SymId::fermataBelow);
            chord->add(atr);
        } else if (c == 'O') {
            // Articulation* atr = new Articulation(score);
            // atr->setArticulationType(ArticulationType::Downbow);
            // chord->add(atr);
            LOGD("%06d: open string '%c' not implemented", tick.ticks(), an[i].toLatin1());
        } else if (c == '&') {
            // skip editorial level
            if (i <= an.size() && an[i + 1].isDigit()) {
                ++i;
            }
        } else if (c == 'p') {
            dynamics += "p";
        } else if (c == 'm') {
            dynamics += "m";
        } else if (c == 'f') {
            dynamics += "f";
        } else if (c == '-') {        // tie
        } else if (c == '*') {        // start tuplet
        } else if (c == '!') {        // stop tuplet
        } else if (c == '+') {        // cautionary accidental
        } else if (c == 'X') {        // ???
        } else if (c == ' ') {
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

    QStringView txt = s.mid(43, 36);
    if (!txt.isEmpty()) {
        QList<QStringView> sl = txt.split(u'|');
        int no = 0;
        for (const QStringView w : sl) {
            Lyrics* l = Factory::createLyrics(chord);
            l->setPlainText(diacritical(w));
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

QString MuseData::diacritical(QStringView s)
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

    QString result = s.toString();
    for (unsigned int i = 0; i < sizeof(tab) / sizeof(*tab); ++i) {
        result = result.replace(tab[i].a, QString::fromUtf8(tab[i].b));
    }
    return result;
}

//---------------------------------------------------------
//   readRest
//---------------------------------------------------------

void MuseData::readRest(Part* part, QStringView s)
{
    Fraction ticks = Fraction::fromTicks((s.mid(5, 3).toInt() * Constants::DIVISION + _division / 2) / _division);

    Fraction tick  = curTick;
    curTick  += ticks;

    int staffIdx = 0;
    if (s.size() >= 24) {
        if (s[23].isDigit()) {
            staffIdx = s.mid(23, 1).toInt() - 1;
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

void MuseData::readBackup(QStringView s)
{
    Fraction ticks = Fraction::fromTicks((s.mid(5, 3).toInt() * Constants::DIVISION + _division / 2) / _division);
    if (s[0] == QChar('b')) {
        curTick -= ticks;
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

    score->measures()->append(mes);
    return mes;
}

//---------------------------------------------------------
//   readPart
//---------------------------------------------------------

void MuseData::readPart(const QStringList& sl, Part* part)
{
    int line = 10;
    QString s;
    for (; line < sl.size(); ++line) {
        s = sl[line];
        if (!s.isEmpty() && s[0] == QChar('$')) {
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
        QStringView s { sl[i] };
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
                    staffIdx = s.mid(23, 1).toInt();
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
        char c = s[0].toLatin1();
        if (c == '&') {
            commentMode = !commentMode;
            continue;
        }
        if (commentMode) {
            continue;
        }
        if (c == '@') {
            continue;
        }
        if (c == '/') {
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
        if (c == 'a') {
            part.back().append(s.mid(1));
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
