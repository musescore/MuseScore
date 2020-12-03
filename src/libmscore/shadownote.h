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
    int _line;
    SymId _notehead;
    TDuration _duration;
    int _voice;
    bool _rest;

public:
    ShadowNote(Score*);

    ShadowNote* clone() const override { return new ShadowNote(*this); }
    ElementType type() const override { return ElementType::SHADOW_NOTE; }

    void layout() override;
    int line() const { return _line; }
    void setLine(int n) { _line = n; }

    void draw(QPainter*) const override;

    void setState(SymId noteSymbol, int voice, TDuration duration, bool rest = false);

    SymId getNoteFlag() const;
    bool computeUp() const;

    SymId notehead() const { return _notehead; }
    bool isValid() const;
};
}     // namespace Ms
#endif
