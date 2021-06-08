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

#ifndef __MEASUREREPEAT_H__
#define __MEASUREREPEAT_H__

#include "rest.h"
#include "text.h"
#include "utils.h"

namespace Ms {
class Score;
class Segment;

//---------------------------------------------------------
//   @@ MeasureRepeat
//---------------------------------------------------------

class MeasureRepeat final : public Rest
{
public:
    MeasureRepeat(Score*);
    MeasureRepeat& operator=(const MeasureRepeat&) = delete;

    MeasureRepeat* clone() const override { return new MeasureRepeat(*this); }
    Element* linkedClone() override { return Element::linkedClone(); }
    ElementType type() const override { return ElementType::MEASURE_REPEAT; }

    void setNumMeasures(int n) { m_numMeasures = n; }
    int numMeasures() const { return m_numMeasures; }
    void setSymId(SymId id) { m_symId = id; }
    SymId symId() const { return m_symId; }
    void setNumberSym(int n) { m_numberSym = toTimeSigString(QString::number(n)); }
    std::vector<SymId> numberSym() const { return m_numberSym; }
    void setNumberPos(qreal d) { m_numberPos = d; }
    qreal numberPos() const { return m_numberPos; }

    Measure* firstMeasureOfGroup() const { return measure()->firstOfMeasureRepeatGroup(staffIdx()); }

    void draw(mu::draw::Painter*) const override;
    void layout() override;
    Fraction ticks() const override;
    Fraction actualTicks() const { return Rest::ticks(); }

    void read(XmlReader&) override;
    void write(XmlWriter& xml) const override;

    QVariant propertyDefault(Pid) const override;
    bool setProperty(Pid, const QVariant&) override;
    QVariant getProperty(Pid) const override;

    mu::RectF numberRect() const override;
    Shape shape() const override;

    QString accessibleInfo() const override;

    bool placeMultiple() const override { return numMeasures() == 1; }     // prevent overlapping additions with range selection

private:
    Sid getPropertyStyle(Pid) const override;
    int m_numMeasures;
    std::vector<SymId> m_numberSym;
    qreal m_numberPos;
    SymId m_symId;
};
}     // namespace Ms
#endif
