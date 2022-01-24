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

#ifndef __VIBRATO_H__
#define __VIBRATO_H__

#include "line.h"

namespace Ms {
class Vibrato;
class Accidental;

//---------------------------------------------------------
//   @@ VibratoSegment
//---------------------------------------------------------

class VibratoSegment final : public LineSegment
{
    SymIdList _symbols;

    void symbolLine(SymId start, SymId fill);
    void symbolLine(SymId start, SymId fill, SymId end);
    virtual Sid getPropertyStyle(Pid) const override;

public:
    VibratoSegment(Vibrato* sp, System* parent);

    VibratoSegment* clone() const override { return new VibratoSegment(*this); }

    Vibrato* vibrato() const { return toVibrato(spanner()); }

    void draw(mu::draw::Painter*) const override;
    void layout() override;

    EngravingItem* propertyDelegate(Pid) override;

    Shape shape() const override;
    SymIdList symbols() const { return _symbols; }
    void setSymbols(const SymIdList& s) { _symbols = s; }
};

//---------------------------------------------------------
//   Vibrato
//---------------------------------------------------------

class Vibrato final : public SLine
{
    Sid getPropertyStyle(Pid) const override;

public:
    enum class Type : char {
        GUITAR_VIBRATO, GUITAR_VIBRATO_WIDE, VIBRATO_SAWTOOTH, VIBRATO_SAWTOOTH_WIDE
    };
private:
    Type _vibratoType;
    bool _playArticulation;

public:
    Vibrato(EngravingItem* parent);
    ~Vibrato();

    Vibrato* clone() const override { return new Vibrato(*this); }

    void layout() override;
    LineSegment* createLineSegment(System* parent) override;

    void write(XmlWriter&) const override;
    void read(XmlReader&) override;

    void setVibratoType(const QString& s);
    void undoSetVibratoType(Type val);
    void setVibratoType(Type tt) { _vibratoType = tt; }
    Type vibratoType() const { return _vibratoType; }
    void setPlayArticulation(bool val) { _playArticulation = val; }
    bool playArticulation() const { return _playArticulation; }
    static QString type2name(Vibrato::Type t);
    QString vibratoTypeName() const;
    QString vibratoTypeUserName() const;

    Segment* segment() const { return (Segment*)explicitParent(); }

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid) const override;
    Pid propertyId(const QStringRef& xmlName) const override;
    QString accessibleInfo() const override;
};

//---------------------------------------------------------
//   VibratoTableItem
//---------------------------------------------------------

struct VibratoTableItem {
    Vibrato::Type type;
    const char* name;
    QString userName;
};

extern const std::vector<VibratoTableItem> vibratoTable;
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Vibrato::Type);

#endif
