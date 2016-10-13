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
#include "xml.h"
#include "measure.h"

namespace Ms {

//---------------------------------------------------------
//   JumpTypeTable
//---------------------------------------------------------

const JumpTypeTable jumpTypeTable[] = {
      { Jump::Type::DC,         TextStyleType::REPEAT_RIGHT, "D.C.",         "start", "end",  "",      QT_TRANSLATE_NOOP("jumpType", "Da Capo")        },
      { Jump::Type::DC_AL_FINE, TextStyleType::REPEAT_RIGHT, "D.C. al Fine", "start", "fine", "" ,     QT_TRANSLATE_NOOP("jumpType", "Da Capo al Fine")},
      { Jump::Type::DC_AL_CODA, TextStyleType::REPEAT_RIGHT, "D.C. al Coda", "start", "coda", "codab", QT_TRANSLATE_NOOP("jumpType", "Da Capo al Coda")},
      { Jump::Type::DS_AL_CODA, TextStyleType::REPEAT_RIGHT, "D.S. al Coda", "segno", "coda", "codab", QT_TRANSLATE_NOOP("jumpType", "D.S. al Coda")   },
      { Jump::Type::DS_AL_FINE, TextStyleType::REPEAT_RIGHT, "D.S. al Fine", "segno", "fine", "",      QT_TRANSLATE_NOOP("jumpType", "D.S. al Fine")   },
      { Jump::Type::DS,         TextStyleType::REPEAT_RIGHT, "D.S.",         "segno", "end",  "",      QT_TRANSLATE_NOOP("jumpType", "D.S.")           }
      };

int jumpTypeTableSize()
      {
      return sizeof(jumpTypeTable)/sizeof(JumpTypeTable);
      }

//---------------------------------------------------------
//   Jump
//---------------------------------------------------------

Jump::Jump(Score* s)
   : Text(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
      setTextStyleType(TextStyleType::REPEAT_RIGHT);
      setLayoutToParentWidth(true);
      _playRepeats = false;
      }

//---------------------------------------------------------
//   setJumpType
//---------------------------------------------------------

void Jump::setJumpType(Type t)
      {
      for (const JumpTypeTable& p : jumpTypeTable) {
            if (p.type == t) {
                  setXmlText(p.text);
                  setJumpTo(p.jumpTo);
                  setPlayUntil(p.playUntil);
                  setContinueAt(p.continueAt);
                  setTextStyleType(p.textStyleType);
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   jumpType
//---------------------------------------------------------

Jump::Type Jump::jumpType() const
      {
      for (const JumpTypeTable& t : jumpTypeTable) {
            if (_jumpTo == t.jumpTo && _playUntil == t.playUntil && _continueAt == t.continueAt)
                  return t.type;
            }
      return Type::USER;
      }

QString Jump::jumpTypeUserName() const
      {
      int idx = static_cast<int>(this->jumpType());
      if (idx < jumpTypeTableSize())
            return qApp->translate("jumpType", jumpTypeTable[idx].userText.toUtf8().constData());
      return tr("Custom");
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
            else if (tag == "playRepeats")
                  _playRepeats = e.readBool();
            else if (!Text::readProperties(e))
                  e.unknown();
            }
//      setTextStyleType(TextStyleType::REPEAT_RIGHT);    // do not reset text style!
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
      writeProperty(xml, P_ID::PLAY_REPEATS);
      xml.etag();
      }

//---------------------------------------------------------
//   undoSetJumpTo
//---------------------------------------------------------

void Jump::undoSetJumpTo(const QString& s)
      {
      undoChangeProperty(P_ID::JUMP_TO, s);
      }

//---------------------------------------------------------
//   undoSetPlayUntil
//---------------------------------------------------------

void Jump::undoSetPlayUntil(const QString& s)
      {
      undoChangeProperty(P_ID::PLAY_UNTIL, s);
      }

//---------------------------------------------------------
//   undoSetContinueAt
//---------------------------------------------------------

void Jump::undoSetContinueAt(const QString& s)
      {
      undoChangeProperty(P_ID::CONTINUE_AT, s);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Jump::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::JUMP_TO:
                  return jumpTo();
            case P_ID::PLAY_UNTIL:
                  return playUntil();
            case P_ID::CONTINUE_AT:
                  return continueAt();
            case P_ID::PLAY_REPEATS:
                  return playRepeats();
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
            case P_ID::JUMP_TO:
                  setJumpTo(v.toString());
                  break;
            case P_ID::PLAY_UNTIL:
                  setPlayUntil(v.toString());
                  break;
            case P_ID::CONTINUE_AT:
                  setContinueAt(v.toString());
                  break;
            case P_ID::PLAY_REPEATS:
                  setPlayRepeats(v.toInt());
                  break;
            default:
                  if (!Text::setProperty(propertyId, v))
                        return false;
                  break;
            }
      triggerLayout();
      score()->setPlaylistDirty();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Jump::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::JUMP_TO:
            case P_ID::PLAY_UNTIL:
            case P_ID::CONTINUE_AT:
                  return QString("");

            case P_ID::PLAY_REPEATS:
                  return false;

            default:
                  break;
            }
      return Text::propertyDefault(propertyId);
      }

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

Element* Jump::nextElement()
      {
      Segment* seg = measure()->last();
      return seg->firstElement(staffIdx());
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* Jump::prevElement()
      {
      return nextElement();
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Jump::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(this->jumpTypeUserName());
      }

}

