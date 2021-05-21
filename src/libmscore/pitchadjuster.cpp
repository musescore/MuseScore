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

#include "pitchadjuster.h"
#include "property.h"
#include "style.h"
#include "xml.h"

/*
 * MusicXml tracks harmonics as BASE, TOUCHING and SOUNDING pitch.
 *
 *   Base Pitch:     the tuning for the string. For artificial harmonics its
 *                   the tuning for the string + a fret number
 *   Touching Pitch: the pitch of the note if it was played normally. This
 *                   is base pitch + fret number
 *   Sounding Pitch: the pitch of the harmonic. It may be several octaves higher
 *                   than the base/touching pitch.
 *
 * PitchAdjuster lives within a MuseScore Note. For this reason the class was optimized
 * for size (i.e. pitches are stored as unsigned char). This is ugly, but compact.
 *
 * PitchAdjuster does the following:
 *
 * 1. Tracks optional harmonic info for a Note that works for music notation and
 *    tablature.
 * 2. Saves/loads harmonic info to/from MuseScore mscx/mscz files.
 * 3. Provides a getAlter() method that does harmonic math
 * 4. Supports fractional fret numbers (e.g. touching fret = 2.7).
 * 5. Supports a way to alter a Note's pitch by a specific amount. This is needed to
 *    handle Tuner string bends (e.g. TablEdit Pitch Bends)
 */

namespace Ms {
//---------------------------------------------------------
// PitchAdjuster
//---------------------------------------------------------

PitchAdjuster::PitchAdjuster()
    : _harmonicType(HarmonicType::None),
    _basePitch(0),
    _touchingFret(0),
    _alter(0)
{
}

//---------------------------------------------------------
// clear
//---------------------------------------------------------

void PitchAdjuster::clear()
{
    _harmonicType = HarmonicType::None;
    _basePitch = 0;
    _touchingFret = 0;
    _alter = 0;
}

//---------------------------------------------------------
// setNaturalHarmonic
//---------------------------------------------------------

bool PitchAdjuster::setNaturalHarmonic(int basePitch, int touchingPitch)
{
    if (basePitch < 0 || basePitch > 127
        || touchingPitch < 0 || touchingPitch > 127
        || touchingPitch < basePitch || (touchingPitch - basePitch) > 24) {
        clear();
        return false;
    }
    _harmonicType = HarmonicType::Natural;
    setBasePitch(basePitch);
    setTouchingFret(touchingPitch - basePitch);
    return true;
}

//---------------------------------------------------------
// setArtificialHarmonic
//---------------------------------------------------------

bool PitchAdjuster::setArtificialHarmonic(int basePitch, int touchingPitch, HarmonicType hType)
{
    if (basePitch < 0 || basePitch > 127
        || touchingPitch < 0 || touchingPitch > 127
        || touchingPitch < basePitch || (touchingPitch - basePitch) > 24) {
        clear();
        return false;
    }
    _harmonicType = hType;
    setBasePitch(basePitch);
    setTouchingFret(touchingPitch - basePitch);
    return true;
}

//---------------------------------------------------------
// setNaturalHarmonic
//---------------------------------------------------------

bool PitchAdjuster::setNaturalHarmonic(int basePitch, double touchingPitch)
{
    if (basePitch < 0 || basePitch > 127
        || touchingPitch < 0.0 || touchingPitch > 127.0
        || touchingPitch < (double)basePitch || (touchingPitch - (double)basePitch) > 24.0) {
        clear();
        return false;
    }
    _harmonicType = HarmonicType::Natural;
    setBasePitch(basePitch);
    setTouchingFret(touchingPitch - double(basePitch));
    return true;
}

//---------------------------------------------------------
// setNaturalHarmonic
//---------------------------------------------------------

bool PitchAdjuster::setNaturalHarmonic(int basePitch, const QString& touchingPitch)
{
    bool ok = false;
    double touch = touchingPitch.toDouble(&ok);
    if (!ok) {
        clear();
        return false;
    }
    return setNaturalHarmonic(basePitch, touch);
}

//---------------------------------------------------------
// getHarmonicType
//---------------------------------------------------------

HarmonicType PitchAdjuster::getHarmonicType() const
{
    return _harmonicType;
}

//---------------------------------------------------------
// getBasePitch
//---------------------------------------------------------

int PitchAdjuster::getBasePitch() const
{
    return int(_basePitch);
}

//---------------------------------------------------------
// getTouchingPitch
//---------------------------------------------------------

int PitchAdjuster::getTouchingPitch() const
{
    return getBasePitch() + int(getTouchingFret());
}

//---------------------------------------------------------
// getSoundingPitch
//---------------------------------------------------------

int PitchAdjuster::getSoundingPitch() const
{
    if (_harmonicType == HarmonicType::Natural) {
        return getTouchingPitch() + getAlter();
    }
    return getBasePitch() + getAlter();
}

//---------------------------------------------------------
// getTouchingFret
//---------------------------------------------------------

double PitchAdjuster::getTouchingFret() const
{
    return (double)_touchingFret / 10.0;
}

//---------------------------------------------------------
// toStringTouchingFret
//   e.g  "12", "7", "3.2"
//---------------------------------------------------------

QString PitchAdjuster::toStringTouchingFret() const
{
    return QString::number(getTouchingFret(), 'f', (_touchingFret % 10) == 0 ? 0 : 1);
}

//---------------------------------------------------------
// getAlter
//---------------------------------------------------------

int PitchAdjuster::getAlter() const
{
    return getHarmonicAlter() + getPitchAlter();
}

//---------------------------------------------------------
// getPitchAlter
//---------------------------------------------------------

int PitchAdjuster::getPitchAlter() const
{
    return (_alter & 0x80) ? 128 - (int)_alter : int(_alter);
}

//---------------------------------------------------------
// getHarmonicAlter
//---------------------------------------------------------

int PitchAdjuster::getHarmonicAlter() const
{
    int alt = 0;
    if (_harmonicType == HarmonicType::Natural) {
        alt = computeHarmonicFret() - int(getTouchingFret());
    } else if (_harmonicType != HarmonicType::None) {
        alt = computeHarmonicFret();
    }
    return alt;
}

//---------------------------------------------------------
// setBasePitch
//---------------------------------------------------------

void PitchAdjuster::setBasePitch(int pitch)
{
    if (pitch < 0 || pitch > 127) {
        pitch = 60; // middle C
    }
    _basePitch = (unsigned char)pitch;
}

//---------------------------------------------------------
// setTouchingFret
//---------------------------------------------------------

void PitchAdjuster::setTouchingFret(int fret)
{
    if (fret < 1 || fret > 24) {
        fret = 12;
    }
    _touchingFret = (unsigned char)(fret * 10);
}

//---------------------------------------------------------
// setTouchingFret
//---------------------------------------------------------

void PitchAdjuster::setTouchingFret(double fret)
{
    if (fret < 1.0 || fret > 24.0) {
        fret = 12.0;
    }
    _touchingFret = (unsigned char)(fret * 10.0 + 0.5);
}

//---------------------------------------------------------
// setPitchAlter
//---------------------------------------------------------

void PitchAdjuster::setPitchAlter(int alter)
{
    if (alter < -100 || alter > 100) {
        alter = 0;
    }
    _alter = (unsigned char)((alter < 0) ? 128 - alter : alter);
}

//---------------------------------------------------------
// computeHarmonicFret
//   - returns the fret number with the same pitch as the harmonic
//
// Fraction  | Alter | Frets
// --------------------------------------------------------
// 1/2       | 12    | 12
// 1/3  (P5) | 12+7  | 7, 19
// 1/4       | 24    | 5, 24
//
// The following are rare and hard to sound cleanly
//
// 1/5  (M3) | 24+4  | 3.9, 8.9, 15.9
// 1/6  (P5) | 24+7  | 3.2
// 1/7  (m7) | 24+10 | 2.7, 5.8, 9.7, 14.7, 21.7
// 1/8       | 36    | 2.3, 8.1, 17
// 1/9  (M2) | 36+2  | 2, 4.4, 10.2, 14
// 1/10 (M3) | 36+4  | 1.8, 6.2, 20.8
//---------------------------------------------------------

int PitchAdjuster::computeHarmonicFret() const
{
    switch ((int)_touchingFret) {
    // 1/2
    case 120:      // 12th fret
        return 12;

    // 1/3
    case 70:       // 7th fret
    case 190:      // 19th fret
        return 19;

    // 1/4
    case 50:       // 5th fret
    case 240:      // 24th fret
        return 24;

    // 1/5
    case 39:
    case 40:
    case 89:
    case 90:
    case 159:
    case 160:
        return 28;

    // 1/6
    case 32:
        return 31;

    // 1/7
    case 27:
    case 58:
    case 97:
    case 147:
    case 217:
        return 34;

    // 1/8
    case 23:
    case 24:
    case 81:
    case 170:
        return 36;

    // 1/9
    case 20:
    case 44:
    case 102:
    case 140:
        return 38;

    // 1/10
    case 18:
    case 62:
    case 208:
        return 40;
    }
    return 0;
}

//---------------------------------------------------------
// write
//   <pitchadjust type="1" base="55" touch="12"/>
//   <pitchadjust alter="-2"/>
//---------------------------------------------------------

static const char* _xTag   = "pitchAdjust";
static const char* _xType  = "type";
static const char* _xBase  = "base";
static const char* _xTouch = "touch";
static const char* _xAlter = "alter";

void PitchAdjuster::write(XmlWriter& xml) const
{
    if (_alter != 0 || _harmonicType != HarmonicType::None) {
        QString tag(_xTag);
        if (_harmonicType != HarmonicType::None) {
            tag += QString(" %1=\"%2\" %3=\"%4\" %5=\"%6\"")
                   .arg(_xType).arg(int(_harmonicType))
                   .arg(_xBase).arg(getBasePitch())
                   .arg(_xTouch).arg(toStringTouchingFret());
        }
        if (_alter != 0) {
            tag += QString(" %1=\"%2\"").arg(_xAlter).arg(getPitchAlter());
        }
        xml.tagE(tag);
    }
}

//---------------------------------------------------------
// read
//---------------------------------------------------------

void PitchAdjuster::read(XmlReader& e)
{
    clear();
    if (e.hasAttribute(_xType)) {
        _harmonicType = HarmonicType(e.intAttribute(_xType));
    }
    if (e.hasAttribute(_xBase)) {
        setBasePitch(e.intAttribute(_xBase));
    }
    if (e.hasAttribute(_xTouch)) {
        setTouchingFret(e.doubleAttribute(_xTouch));
    }
    if (e.hasAttribute(_xAlter)) {
        setPitchAlter(e.intAttribute(_xAlter));
    }
    e.skipCurrentElement();
}

// debugging stuff
#if 0
//---------------------------------------------------------
// toString
//   - used for debugging
//   natural:    "(12)", "(2.3)"
//   artificial: "(3)15"
//   pitch:      "[2]+" or "[2]-"
//---------------------------------------------------------

QString PitchAdjuster::toString(int baseFret) const
{
    QString s;
    switch (_harmonicType) {
    case HarmonicType::Natural:
        s = QString("(%1)").arg(toStringTouchingFret());
        break;
    case HarmonicType::Artificial:
    case HarmonicType::Pinch:
    case HarmonicType::Tap:
    case HarmonicType::Touch:
        s = QString("(%1)%2").arg(baseFret).arg(baseFret + getHarmonicAlter());
        break;
    case HarmonicType::None:
        break;
    }
    int pa = getPitchAlter();
    if (pa > 0) {
        s += QString("[%1]+").arg(pa);
    } else if (pa < 0) {
        s += QString("[%1]-").arg(-pa);
    }
    return s;
}

//---------------------------------------------------------
// test
//---------------------------------------------------------

void PitchAdjuster::test() const
{
    QString s;
    int alter;
    int base;
    int sound;
    double touch;
    int bugs = 0;
    int OPENG = 55;  // G3

    PitchAdjuster pa;

    // 12th fret
    pa.clear();
    pa.setNaturalHarmonic(OPENG, OPENG + 12);
    s = pa.toString();
    alter = pa.getAlter();
    base = pa.getBasePitch();
    sound = pa.getSoundingPitch();
    touch = pa.getTouchingPitch();
    if (s != "(12)" || alter != 0 || base != OPENG || sound != OPENG + 12 || int(touch) != OPENG + 12) {
        bugs++;
    }

    // 12th fret
    pa.clear();
    pa.setNaturalHarmonic(OPENG, QString("67"));
    s = pa.toString();
    alter = pa.getAlter();
    base = pa.getBasePitch();
    sound = pa.getSoundingPitch();
    touch = pa.getTouchingPitch();
    if (s != "(12)" || alter != 0 || base != OPENG || sound != OPENG + 12 || int(touch) != OPENG + 12) {
        bugs++;
    }

    // 7th fret
    pa.clear();
    pa.setNaturalHarmonic(OPENG, OPENG + 7);
    s = pa.toString();
    alter = pa.getAlter();
    base = pa.getBasePitch();
    sound = pa.getSoundingPitch();
    touch = pa.getTouchingPitch();
    if (s != "(7)" || alter != 12 || base != OPENG || sound != OPENG + 19 || int(touch) != OPENG + 7) {
        bugs++;
    }

    // 19th fret (same as 7th)
    pa.clear();
    pa.setNaturalHarmonic(OPENG, OPENG + 19);
    s = pa.toString();
    base = pa.getBasePitch();
    alter = pa.getAlter();
    sound = pa.getSoundingPitch();
    touch = pa.getTouchingPitch();
    if (s != "(19)" || alter != 0 || base != OPENG || sound != OPENG + 19 || int(touch) != OPENG + 19) {
        bugs++;
    }

    // 5th fret
    pa.clear();
    pa.setNaturalHarmonic(OPENG, OPENG + 5);
    s = pa.toString();
    alter = pa.getAlter();
    base = pa.getBasePitch();
    sound = pa.getSoundingPitch();
    touch = pa.getTouchingPitch();
    if (s != "(5)" || alter != 19 || base != OPENG || sound != OPENG + 24 || int(touch) != OPENG + 5) {
        bugs++;
    }

    // 24th fret (same as 5th)
    pa.clear();
    pa.setNaturalHarmonic(OPENG, OPENG + 24);
    s = pa.toString();
    alter = pa.getAlter();
    base = pa.getBasePitch();
    sound = pa.getSoundingPitch();
    touch = pa.getTouchingPitch();
    if (s != "(24)" || alter != 0 || base != OPENG || sound != OPENG + 24 || int(touch) != OPENG + 24) {
        bugs++;
    }

    // no harmonic, 2 semitones up
    pa.clear();
    pa.setPitchAlter(2);
    s = pa.toString();
    alter = pa.getAlter();
    if (s != "[2]+" || alter != 2) {
        bugs++;
    }

    // no harmonic, 2 semitones down
    pa.clear();
    pa.setPitchAlter(-2);
    s = pa.toString();
    alter = pa.getAlter();
    if (s != "[2]-" || alter != -2) {
        bugs++;
    }

    // 12th fret harmonic + 3
    pa.clear();
    pa.setArtificialHarmonic(OPENG + 3, OPENG + 15);
    s = pa.toString(3);
    alter = pa.getAlter();
    base = pa.getBasePitch();
    sound = pa.getSoundingPitch();
    touch = pa.getTouchingPitch();
    if (s != "(3)15" || alter != 12 || base != OPENG + 3 || sound != OPENG + 12 + 3 || int(touch) != OPENG + 15) {
        bugs++;
    }

    // 2.7 fret
    pa.clear();
    pa.setNaturalHarmonic(OPENG, (double)OPENG + 2.7);
    s = pa.toString();
    alter = pa.getAlter();
    base = pa.getBasePitch();
    sound = pa.getSoundingPitch();
    touch = pa.getTouchingPitch();
    if (s != "(2.7)" || alter != 32 || base != OPENG || sound != OPENG + 32 + 2 || int(touch) != OPENG + 2) {
        bugs++;
    }

    if (bugs > 0) {
        qDebug() << "PitchAdjuster::test() failed.\n";
    }
}

#endif
} // namespace
