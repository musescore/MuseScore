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

#ifndef __TRILL_H__
#define __TRILL_H__

#include "line.h"

namespace Ms {
class Trill;
class Accidental;

//---------------------------------------------------------
//   @@ TrillSegment
//---------------------------------------------------------

class TrillSegment final : public LineSegment
{
    SymIdList _symbols;

    void symbolLine(SymId start, SymId fill);
    void symbolLine(SymId start, SymId fill, SymId end);
    Sid getPropertyStyle(Pid) const override;

public:
    TrillSegment(Trill* sp, System* parent);
    TrillSegment(System* parent);

    Trill* trill() const { return (Trill*)spanner(); }

    TrillSegment* clone() const override { return new TrillSegment(*this); }
    void draw(mu::draw::Painter*) const override;
    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;
    void layout() override;

    EngravingItem* propertyDelegate(Pid) override;

    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;
    Shape shape() const override;

    SymIdList symbols() const { return _symbols; }
    void setSymbols(const SymIdList& s) { _symbols = s; }
};

//---------------------------------------------------------
//   @@ Trill
//   @P trillType  enum (Trill.DOWNPRALL_LINE, .PRALLPRALL_LINE, .PURE_LINE, .TRILL_LINE, .UPPRALL_LINE)
//---------------------------------------------------------

class Trill final : public SLine
{
    Sid getPropertyStyle(Pid) const override;

public:
    enum class Type : char {
        TRILL_LINE, UPPRALL_LINE, DOWNPRALL_LINE, PRALLPRALL_LINE,
    };

private:
    Type _trillType;
    Accidental* _accidental;
    OrnamentStyle _ornamentStyle;   // for use in ornaments such as trill
    bool _playArticulation;

public:
    Trill(EngravingItem* parent);
    Trill(const Trill& t);
    ~Trill();

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObject* scanChild(int idx) const override;
    int scanChildCount() const override;

    Trill* clone() const override { return new Trill(*this); }

    void layout() override;
    LineSegment* createLineSegment(System* parent) override;
    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;
    void write(XmlWriter&) const override;
    void read(XmlReader&) override;

    void setTrillType(const QString& s);
    void setTrillType(Type tt) { _trillType = tt; }
    Type trillType() const { return _trillType; }
    void setOrnamentStyle(OrnamentStyle val) { _ornamentStyle = val; }
    OrnamentStyle ornamentStyle() const { return _ornamentStyle; }
    void setPlayArticulation(bool val) { _playArticulation = val; }
    bool playArticulation() const { return _playArticulation; }
    static QString type2name(Trill::Type t);
    QString trillTypeName() const;
    QString trillTypeUserName() const;
    Accidental* accidental() const { return _accidental; }
    void setAccidental(Accidental* a) { _accidental = a; }

    Segment* segment() const { return (Segment*)explicitParent(); }

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid) const override;
    Pid propertyId(const QStringRef& xmlName) const override;

    QString accessibleInfo() const override;
};

struct TrillTableItem {
    Trill::Type type;
    const char* name;
    QString userName;
};

extern const std::vector<TrillTableItem> trillTable;
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Trill::Type);

#endif
