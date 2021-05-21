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

#ifndef __PITCHADJUSTER_H__
#define __PITCHADJUSTER_H__

namespace Ms {
class XmlReader;
class XmlWriter;

enum class HarmonicType : unsigned char {
    None, Natural, Artificial, Pinch, Tap, Touch
};

class PitchAdjuster
{
    HarmonicType _harmonicType;
    unsigned char _basePitch;
    unsigned char _touchingFret;
    unsigned char _alter;
public:
    PitchAdjuster();
    void clear();

    HarmonicType getHarmonicType() const;
    int getBasePitch() const;
    int getTouchingPitch() const;
    int getSoundingPitch() const;
    int getAlter() const;

    bool setNaturalHarmonic(int, int);
    bool setNaturalHarmonic(int, double);
    bool setNaturalHarmonic(int, const QString&);
    bool setArtificialHarmonic(int, int, HarmonicType t = HarmonicType::Artificial);

    void setBasePitch(int);
    void setTouchingFret(int);
    void setTouchingFret(double);
    void setPitchAlter(int);

    void write(XmlWriter&) const;
    void read(XmlReader&);
protected:
    int computeHarmonicFret() const;
    double getTouchingFret() const;
    QString toStringTouchingFret() const;
    int getHarmonicAlter() const;
    int getPitchAlter() const;

#if 0
    QString toString(int baseFret = 0) const;
    void test() const;
#endif
};
}   // namespace
#endif
