//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TREMOLO_H__
#define __TREMOLO_H__

#include <QPainterPath>

#include "durationtype.h"
#include "symbol.h"

namespace Ms {
class Chord;

// Tremolo subtypes:
enum class TremoloType : signed char {
    INVALID_TREMOLO = -1,
    R8 = 0, R16, R32, R64, BUZZ_ROLL,    // one note tremolo (repeat)
    C8, C16, C32, C64       // two note tremolo (change)
};

// only applicable to minim two-note tremolos in non-TAB staves
enum class TremoloStyle : signed char {
    DEFAULT = 0, TRADITIONAL, TRADITIONAL_ALTERNATE
};

//---------------------------------------------------------
//   @@ Tremolo
//---------------------------------------------------------

class Tremolo final : public Element
{
    TremoloType _tremoloType { TremoloType::R8 };
    Chord* _chord1 { nullptr };
    Chord* _chord2 { nullptr };
    TDuration _durationType;
    QPainterPath path;

    int _lines;         // derived from _subtype
    TremoloStyle _style { TremoloStyle::DEFAULT };

    QPainterPath basePath() const;
    void computeShape();
    void layoutOneNoteTremolo(qreal x, qreal y, qreal spatium);
    void layoutTwoNotesTremolo(qreal x, qreal y, qreal h, qreal spatium);

public:
    Tremolo(Score*);
    Tremolo(const Tremolo&);
    Tremolo& operator=(const Tremolo&) = delete;
    Tremolo* clone() const override { return new Tremolo(*this); }
    ElementType type() const override { return ElementType::TREMOLO; }
    int subtype() const override { return static_cast<int>(_tremoloType); }
    QString subtypeName() const override;

    void scanElements(void* data, void (* func)(void*, Element*), bool all=true) override;

    QString tremoloTypeName() const;
    void setTremoloType(const QString& s);
    static TremoloType name2Type(const QString& s);
    static QString type2name(TremoloType t);

    Chord* chord() const { return toChord(parent()); }

    void setTremoloType(TremoloType t);
    TremoloType tremoloType() const { return _tremoloType; }

    qreal minHeight() const;

    qreal mag() const override;
    void draw(QPainter*) const override;
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

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid propertyId) const override;
    Pid propertyId(const QStringRef& xmlName) const override;
    QString propertyUserValue(Pid) const override;
};
}     // namespace Ms
#endif
