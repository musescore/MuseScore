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

#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include "bsymbol.h"

#include "infrastructure/draw/font.h"

namespace Ms {
class Segment;
class ScoreFont;

//---------------------------------------------------------
//   @@ Symbol
///    Symbol constructed from builtin symbol.
//
//   @P symbol       string       the SMuFL name of the symbol
//---------------------------------------------------------

class Symbol : public BSymbol
{
protected:
    SymId _sym;
    const ScoreFont* _scoreFont = nullptr;

public:
    Symbol(const ElementType& type, EngravingItem* parent, ElementFlags f = ElementFlag::MOVABLE);
    Symbol(EngravingItem* parent, ElementFlags f = ElementFlag::MOVABLE);
    Symbol(const Symbol&);

    Symbol& operator=(const Symbol&) = delete;

    Symbol* clone() const override { return new Symbol(*this); }

    void setSym(SymId s, const ScoreFont* sf = nullptr) { _sym  = s; _scoreFont = sf; }
    SymId sym() const { return _sym; }
    QString symName() const;

    QString accessibleInfo() const override;

    void draw(mu::draw::Painter*) const override;
    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;
    void layout() override;

    mu::engraving::PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const mu::engraving::PropertyValue&) override;

    qreal baseLine() const override { return 0.0; }
    virtual Segment* segment() const { return (Segment*)explicitParent(); }
};

//---------------------------------------------------------
//   @@ FSymbol
///    Symbol constructed from a font glyph.
//---------------------------------------------------------

class FSymbol final : public BSymbol
{
    mu::draw::Font _font;
    int _code;

public:
    FSymbol(EngravingItem* parent);
    FSymbol(const FSymbol&);

    FSymbol* clone() const override { return new FSymbol(*this); }

    void draw(mu::draw::Painter*) const override;
    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;
    void layout() override;

    qreal baseLine() const override { return 0.0; }
    Segment* segment() const { return (Segment*)explicitParent(); }
    mu::draw::Font font() const { return _font; }
    int code() const { return _code; }
    void setFont(const mu::draw::Font& f);
    void setCode(int val) { _code = val; }
};
}     // namespace Ms
#endif
