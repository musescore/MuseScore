//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2015-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "systemdivider.h"
#include "xml.h"
#include "measure.h"
#include "system.h"

namespace Ms {

//---------------------------------------------------------
//   SystemDivider
//---------------------------------------------------------

SystemDivider::SystemDivider(Score* s) : Symbol(s)
      {
      setSystemFlag(true);
      // default value, but not valid until setDividerType()
      _dividerType = SystemDivider::Type::LEFT;
      _sym = SymId::systemDivider;
      }

//---------------------------------------------------------
//   SystemDivider
//---------------------------------------------------------

SystemDivider::SystemDivider(const SystemDivider& sd) : Symbol(sd)
      {
      _dividerType = sd._dividerType;
      }

//---------------------------------------------------------
//   setDividerType
//---------------------------------------------------------

void SystemDivider::setDividerType(SystemDivider::Type v)
      {
      ScoreFont* sf = score()->scoreFont();
       _dividerType = v;
       if (v == SystemDivider::Type::LEFT) {
             SymId symLeft = Sym::name2id(score()->styleSt(StyleIdx::dividerLeftSym));
             if (!symIsValid(symLeft))
                   sf = sf->fallbackFont();
             setSym(symLeft, sf);
             setXoff(score()->styleD(StyleIdx::dividerLeftX));
             setYoff(score()->styleD(StyleIdx::dividerLeftY));
             setAlign(AlignmentFlags::LEFT | AlignmentFlags::VCENTER);
             }
       else {
             SymId symRight = Sym::name2id(score()->styleSt(StyleIdx::dividerRightSym));
             if (!symIsValid(symRight))
                   sf = sf->fallbackFont();
             setSym(symRight, sf);
             setXoff(score()->styleD(StyleIdx::dividerRightX));
             setYoff(score()->styleD(StyleIdx::dividerRightY));
             setAlign(AlignmentFlags::RIGHT | AlignmentFlags::VCENTER);
             }
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF SystemDivider::drag(EditData* ed)
      {
      setGenerated(false);
      return Symbol::drag(ed);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void SystemDivider::write(Xml& xml) const
      {
      if (dividerType() == SystemDivider::Type::LEFT)
            xml.stag(QString("SystemDivider type=\"left\""));
      else
            xml.stag(QString("SystemDivider type=\"right\""));
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SystemDivider::read(XmlReader& e)
      {
      Align a = align() & AlignmentFlags::VMASK;
      ScoreFont* sf = score()->scoreFont();
      if (e.attribute("type") == "left") {
            _dividerType = SystemDivider::Type::LEFT;
            SymId sym = Sym::name2id(score()->styleSt(StyleIdx::dividerLeftSym));
            if (!symIsValid(sym))
                  sf = sf->fallbackFont();
            setSym(sym, sf);
            setAlign(a | AlignmentFlags::LEFT);
            setXoff(score()->styleB(StyleIdx::dividerLeftX));
            setYoff(score()->styleB(StyleIdx::dividerLeftY));
            }
      else {
            _dividerType = SystemDivider::Type::RIGHT;
            SymId sym = Sym::name2id(score()->styleSt(StyleIdx::dividerRightSym));
            if (!symIsValid(sym))
                  sf = sf->fallbackFont();
            setSym(sym, sf);
            setAlign(a | AlignmentFlags::RIGHT);
            setXoff(score()->styleB(StyleIdx::dividerRightX));
            setYoff(score()->styleB(StyleIdx::dividerRightY));
            }
      Symbol::read(e);
      }
}  // namespace Ms


