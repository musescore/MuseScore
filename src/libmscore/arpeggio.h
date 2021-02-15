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

#ifndef __ARPEGGIO_H__
#define __ARPEGGIO_H__

#include "element.h"

namespace Ms {
class Chord;

enum class ArpeggioType : char {
    NORMAL, UP, DOWN, BRACKET, UP_STRAIGHT, DOWN_STRAIGHT
};

//---------------------------------------------------------
//   @@ Arpeggio
//---------------------------------------------------------

class Arpeggio final : public Element
{
    ArpeggioType _arpeggioType;
    qreal _userLen1;
    qreal _userLen2;
    qreal _height;
    int _span;                // spanning staves
    std::vector<SymId> symbols;
    bool _playArpeggio;

    qreal _stretch;

    bool _hidden = false;   // set in layout, will skip draw if true

    void symbolLine(SymId start, SymId fill);
    void symbolLine2(SymId end, SymId fill);

    void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;
    QVector<QLineF> dragAnchorLines() const override;
    QVector<QLineF> gripAnchorLines(Grip) const override;
    void startEdit(EditData&) override;

    static const std::array<const char*, 6> arpeggioTypeNames;

public:
    Arpeggio(Score* s);

    Arpeggio* clone() const override { return new Arpeggio(*this); }
    ElementType type() const override { return ElementType::ARPEGGIO; }

    ArpeggioType arpeggioType() const { return _arpeggioType; }
    void setArpeggioType(ArpeggioType v) { _arpeggioType = v; }
    QString arpeggioTypeName() { return qApp->translate("Palette", arpeggioTypeNames[int(_arpeggioType)]); }

    Chord* chord() const { return (Chord*)parent(); }

    bool acceptDrop(EditData&) const override;
    Element* drop(EditData&) override;
    void layout() override;
    void draw(QPainter*) const override;
    bool isEditable() const override { return true; }
    void editDrag(EditData&) override;
    bool edit(EditData&) override;

    void read(XmlReader& e) override;
    void write(XmlWriter& xml) const override;
    void reset() override;

    int span() const { return _span; }
    void setSpan(int val) { _span = val; }
    void setHeight(qreal);

    qreal userLen1() const { return _userLen1; }
    qreal userLen2() const { return _userLen2; }
    void setUserLen1(qreal v) { _userLen1 = v; }
    void setUserLen2(qreal v) { _userLen2 = v; }

    bool playArpeggio() { return _playArpeggio; }
    void setPlayArpeggio(bool p) { _playArpeggio = p; }

    qreal Stretch() const { return _stretch; }
    void setStretch(qreal val) { _stretch = val; }

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid propertyId) const override;
    Pid propertyId(const QStringRef& xmlName) const override;

    // TODO: add a grip for moving the entire arpeggio
    EditBehavior normalModeEditBehavior() const override { return EditBehavior::Edit; }
    int gripsCount() const override { return 2; }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<QPointF> gripsPositions(const EditData& = EditData()) const override;
};
}     // namespace Ms
#endif
