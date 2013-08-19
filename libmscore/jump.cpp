//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "jump.h"
#include "score.h"

namespace Ms {

//---------------------------------------------------------
//   JumpTypeTable
//---------------------------------------------------------

struct JumpTypeTable {
      JumpType type;
      const char* text;
      const char* jumpTo;
      const char* playUntil;
      const char* continueAt;
      };

static const JumpTypeTable jumpTypeTable[] = {
      { JumpType::DC,         "D.C.",         "start", "end",  "" },
      { JumpType::DC_AL_FINE, "D.C. al Fine", "start", "fine", "" },
      { JumpType::DC_AL_CODA, "D.C. al Coda", "start", "coda", "codab" },
      { JumpType::DS_AL_CODA, "D.S. al Coda", "segno", "coda", "codab" },
      { JumpType::DS_AL_FINE, "D.S. al Fine", "segno", "fine", "" },
      { JumpType::DS,         "D.S.",         "segno", "end",  "" }
      };

//---------------------------------------------------------
//   Jump
//---------------------------------------------------------

Jump::Jump(Score* s)
   : Text(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      setTextStyleType(TEXT_STYLE_REPEAT);
      }

//---------------------------------------------------------
//   setJumpType
//---------------------------------------------------------

void Jump::setJumpType(JumpType t)
      {
      for (const JumpTypeTable& p : jumpTypeTable) {
            if (p.type == t) {
                  setText(p.text);
                  setJumpTo(p.jumpTo);
                  setPlayUntil(p.playUntil);
                  setContinueAt(p.continueAt);
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   jumpType
//---------------------------------------------------------

JumpType Jump::jumpType() const
      {
      for (const JumpTypeTable& t : jumpTypeTable) {
            if (_jumpTo == t.jumpTo && _playUntil == t.playUntil && _continueAt == t.continueAt)
                  return t.type;
            }
      return JumpType::USER;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Jump::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "jumpTo")
                  _jumpTo = e.readElementText();
            else if (tag == "playUntil")
                  _playUntil = e.readElementText();
            else if (tag == "continueAt")
                  _continueAt = e.readElementText();
            else if (!Text::readProperties(e))
                  e.unknown();
            }
//      setTextStyleType(TEXT_STYLE_REPEAT_RIGHT);    // do not reset text style!
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Jump::write(Xml& xml) const
      {
      xml.stag(name());
      Text::writeProperties(xml);
      xml.tag("jumpTo", _jumpTo);
      xml.tag("playUntil", _playUntil);
      xml.tag("continueAt", _continueAt);
      xml.etag();
      }

//---------------------------------------------------------
//   undoSetJumpTo
//---------------------------------------------------------

void Jump::undoSetJumpTo(const QString& s)
      {
      score()->undoChangeProperty(this, P_JUMP_TO, s);
      }

//---------------------------------------------------------
//   undoSetPlayUntil
//---------------------------------------------------------

void Jump::undoSetPlayUntil(const QString& s)
      {
      score()->undoChangeProperty(this, P_PLAY_UNTIL, s);
      }

//---------------------------------------------------------
//   undoSetContinueAt
//---------------------------------------------------------

void Jump::undoSetContinueAt(const QString& s)
      {
      score()->undoChangeProperty(this, P_CONTINUE_AT, s);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Jump::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_JUMP_TO:
                  return jumpTo();
            case P_PLAY_UNTIL:
                  return playUntil();
            case P_CONTINUE_AT:
                  return continueAt();
            default:
                  break;
            }
      return Text::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Jump::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_JUMP_TO:
                  setJumpTo(v.toString());
                  break;
            case P_PLAY_UNTIL:
                  setPlayUntil(v.toString());
                  break;
            case P_CONTINUE_AT:
                  setContinueAt(v.toString());
                  break;
            default:
                  if (!Text::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Jump::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_JUMP_TO:
            case P_PLAY_UNTIL:
            case P_CONTINUE_AT:
                  return QString("");

            default:
                  break;
            }
      return Text::propertyDefault(propertyId);
      }


}

