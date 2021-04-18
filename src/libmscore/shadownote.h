//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SHADOWNOTE_H__
#define __SHADOWNOTE_H__

#include "element.h"
#include "durationtype.h"
#include "staff.h"

class QPointF;
class QRectF;
namespace Ms {
//---------------------------------------------------------
//   ShadowNote
//---------------------------------------------------------

/**
 Graphic representation of a shadow note,
 which shows the note insert position in note entry mode.
*/

class ShadowNote final : public Element
{
    Fraction m_tick;
    int m_lineIndex;
    SymId m_noteheadSymbol;
    TDuration m_duration;
    bool m_isRest;

    qreal m_segmentSkylineTopY = 0;
    qreal m_segmentSkylineBottomY = 0;

public:
    ShadowNote(Score*);

    ShadowNote* clone() const override { return new ShadowNote(*this); }
    ElementType type() const override { return ElementType::SHADOW_NOTE; }

    bool isValid() const;

    Fraction tick() const override { return m_tick; }
    void setTick(Fraction t) { m_tick = t; }

    int lineIndex() const { return m_lineIndex; }
    void setLineIndex(int n) { m_lineIndex = n; }

    void setState(SymId noteSymbol, TDuration duration, bool isRest, qreal segmentSkylineTopY, qreal segmentSkylineBottomY);

    void layout() override;

    void draw(mu::draw::Painter*) const override;
    void drawArticulations(mu::draw::Painter* painter) const;
    void drawMarcato(mu::draw::Painter* painter, const SymId& articulation, QRectF& boundRect) const;
    void drawArticulation(mu::draw::Painter* painter, const SymId& articulation, QRectF& boundRect) const;

    bool computeUp() const;
    SymId noteheadSymbol() const { return m_noteheadSymbol; }
    SymId getNoteFlag() const;
};
} // namespace Ms
#endif
