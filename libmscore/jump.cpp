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
//   jumpStyle
//---------------------------------------------------------

static const ElementStyle jumpStyle {
      { Sid::repeatRightPlacement,               Pid::PLACEMENT              },
      };

//---------------------------------------------------------
//   JumpTypeTable
//---------------------------------------------------------

const JumpTypeTable jumpTypeTable[] = {
      { Jump::Type::DC,         "D.C.",         "start", "end",  "",      QT_TRANSLATE_NOOP("jumpType", "Da Capo")        },
      { Jump::Type::DC_AL_FINE, "D.C. al Fine", "start", "fine", "" ,     QT_TRANSLATE_NOOP("jumpType", "Da Capo al Fine")},
      { Jump::Type::DC_AL_CODA, "D.C. al Coda", "start", "coda", "codab", QT_TRANSLATE_NOOP("jumpType", "Da Capo al Coda")},
      { Jump::Type::DS_AL_CODA, "D.S. al Coda", "segno", "coda", "codab", QT_TRANSLATE_NOOP("jumpType", "D.S. al Coda")   },
      { Jump::Type::DS_AL_FINE, "D.S. al Fine", "segno", "fine", "",      QT_TRANSLATE_NOOP("jumpType", "D.S. al Fine")   },
      { Jump::Type::DS,         "D.S.",         "segno", "end",  "",      QT_TRANSLATE_NOOP("jumpType", "D.S.")           }
      };

int jumpTypeTableSize()
      {
      return sizeof(jumpTypeTable)/sizeof(JumpTypeTable);
      }

//---------------------------------------------------------
//   Jump
//---------------------------------------------------------

Jump::Jump(Score* s)
   : TextBase(s, Tid::REPEAT_RIGHT, ElementFlag::MOVABLE | ElementFlag::SYSTEM)
      {
      initElementStyle(&jumpStyle);
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
                  initTid(Tid::REPEAT_RIGHT);
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
      return QObject::tr("Custom");
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Jump::layout()
      {
      setPos(QPointF(0.0, score()->styleP(Sid::jumpPosAbove)));
      TextBase::layout1();

      if (parent() && autoplace()) {
            setUserOff(QPointF());
            int si             = staffIdx();
            qreal minDistance  = 0.5 * spatium(); // score()->styleP(Sid::tempoMinDistance);
            Shape& s1          = measure()->staffShape(si);
            Shape s2           = shape().translated(pos());
            if (placeAbove()) {
                  qreal d = s2.minVerticalDistance(s1);
                  if (d > -minDistance) {
                        qreal yd       = -d - minDistance;
                        rUserYoffset() = yd;
                        s2.translate(QPointF(0.0, yd));
                        }
                  }
            else {
                  qreal d = s1.minVerticalDistance(s2);
                  if (d > -minDistance) {
                        qreal yd       = d + minDistance;
                        rUserYoffset() = yd;
                        s2.translate(QPointF(0.0, yd));
                        }
                  }
            s1.add(s2);
            }
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
            else if (!TextBase::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Jump::write(XmlWriter& xml) const
      {
      xml.stag(name());
      TextBase::writeProperties(xml);
      xml.tag("jumpTo", _jumpTo);
      xml.tag("playUntil", _playUntil);
      xml.tag("continueAt", _continueAt);
      writeProperty(xml, Pid::PLAY_REPEATS);
      xml.etag();
      }

//---------------------------------------------------------
//   undoSetJumpTo
//---------------------------------------------------------

void Jump::undoSetJumpTo(const QString& s)
      {
      undoChangeProperty(Pid::JUMP_TO, s);
      }

//---------------------------------------------------------
//   undoSetPlayUntil
//---------------------------------------------------------

void Jump::undoSetPlayUntil(const QString& s)
      {
      undoChangeProperty(Pid::PLAY_UNTIL, s);
      }

//---------------------------------------------------------
//   undoSetContinueAt
//---------------------------------------------------------

void Jump::undoSetContinueAt(const QString& s)
      {
      undoChangeProperty(Pid::CONTINUE_AT, s);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Jump::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::JUMP_TO:
                  return jumpTo();
            case Pid::PLAY_UNTIL:
                  return playUntil();
            case Pid::CONTINUE_AT:
                  return continueAt();
            case Pid::PLAY_REPEATS:
                  return playRepeats();
            default:
                  break;
            }
      return TextBase::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Jump::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::JUMP_TO:
                  setJumpTo(v.toString());
                  break;
            case Pid::PLAY_UNTIL:
                  setPlayUntil(v.toString());
                  break;
            case Pid::CONTINUE_AT:
                  setContinueAt(v.toString());
                  break;
            case Pid::PLAY_REPEATS:
                  setPlayRepeats(v.toInt());
                  break;
            default:
                  if (!TextBase::setProperty(propertyId, v))
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

QVariant Jump::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::JUMP_TO:
            case Pid::PLAY_UNTIL:
            case Pid::CONTINUE_AT:
                  return QString("");
            case Pid::PLAY_REPEATS:
                  return false;
            case Pid::PLACEMENT:
                  return int(Placement::ABOVE);
            default:
                  break;
            }
      return TextBase::propertyDefault(propertyId);
      }

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

Element* Jump::nextSegmentElement()
      {
      Segment* seg = measure()->last();
      return seg->firstElement(staffIdx());
      }

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

Element* Jump::prevSegmentElement()
      {
      return nextSegmentElement();
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Jump::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(this->jumpTypeUserName());
      }

}

