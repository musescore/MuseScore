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

#ifndef __TREMOLO_H__
#define __TREMOLO_H__

#include "durationtype.h"
#include "symbol.h"
#include "infrastructure/draw/painterpath.h"

namespace Ms {
class Chord;

// Tremolo subtypes:
enum class TremoloType : signed char {
    INVALID_TREMOLO = -1,
    R8 = 0, R16, R32, R64, BUZZ_ROLL,    // one note tremolo (repeat)
    C8, C16, C32, C64       // two note tremolo (change)
};

// only applicable to minim two-note tremolo in non-TAB staves
enum class TremoloStyle : signed char {
    DEFAULT = 0, TRADITIONAL, TRADITIONAL_ALTERNATE
};

//---------------------------------------------------------
//   @@ Tremolo
//---------------------------------------------------------

class Tremolo final : public EngravingItem
{
    TremoloType _tremoloType { TremoloType::R8 };
    Chord* _chord1 { nullptr };
    Chord* _chord2 { nullptr };
    TDuration _durationType;
    mu::PainterPath path;

    int _lines;         // derived from _subtype
    TremoloStyle _style { TremoloStyle::DEFAULT };

    friend class mu::engraving::Factory;
    Tremolo(Chord* parent);
    Tremolo(const Tremolo&);

    mu::PainterPath basePath() const;
    void computeShape();
    void layoutOneNoteTremolo(qreal x, qreal y, qreal h, qreal spatium);
    void layoutTwoNotesTremolo(qreal x, qreal y, qreal h, qreal spatium);

public:

    Tremolo& operator=(const Tremolo&) = delete;
    Tremolo* clone() const override { return new Tremolo(*this); }

    Chord* chord() const { return toChord(explicitParent()); }
    void setParent(Chord* ch);

    int subtype() const override { return static_cast<int>(_tremoloType); }
    QString subtypeName() const override;

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    QString tremoloTypeName() const;
    void setTremoloType(const QString& s);
    static TremoloType name2Type(const QString& s);
    static QString type2name(TremoloType t);

    void setTremoloType(TremoloType t);
    TremoloType tremoloType() const { return _tremoloType; }

    qreal minHeight() const;

    qreal chordMag() const;
    qreal mag() const override;
    void draw(mu::draw::Painter*) const override;
    void layout() override;
    void layout2();
    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;

    Chord* chord1() const { return _chord1; }
    Chord* chord2() const { return _chord2; }

    TDuration durationType() const { return _durationType; }
    void setDurationType(TDuration d) { _durationType = d; }

    void setChords(Chord* c1, Chord* c2)
    {
        _chord1 = c1;
        _chord2 = c2;
    }

    Fraction tremoloLen() const;
    bool isBuzzRoll() const { return _tremoloType == TremoloType::BUZZ_ROLL; }
    bool twoNotes() const { return _tremoloType >= TremoloType::C8; }    // is it a two note tremolo?
    int lines() const { return _lines; }

    bool placeMidStem() const;

    bool crossStaffBeamBetween() const;

    void spatiumChanged(qreal oldValue, qreal newValue) override;
    void localSpatiumChanged(qreal oldValue, qreal newValue) override;
    void styleChanged() override;

    QString accessibleInfo() const override;

    TremoloStyle style() const { return _style; }
    void setStyle(TremoloStyle v) { _style = v; }

    bool customStyleApplicable() const;

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid propertyId) const override;
    Pid propertyId(const QStringRef& xmlName) const override;
};
}     // namespace Ms
#endif
