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

#include "articulation.h"
#include "score.h"
#include "chordrest.h"
#include "system.h"
#include "measure.h"
#include "staff.h"
#include "stafftype.h"
#include "undo.h"
#include "page.h"
#include "barline.h"
#include "sym.h"

namespace Ms {

//---------------------------------------------------------
//   Articulation::articulationList
//---------------------------------------------------------

ArticulationInfo Articulation::articulationList[ARTICULATIONS] = {
      { SymId::fermataAbove, SymId::fermataBelow,
            "fermata", QT_TRANSLATE_NOOP("articulation", "fermata"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::fermataShortAbove, SymId::fermataShortBelow,
            "shortfermata", QT_TRANSLATE_NOOP("articulation", "short fermata"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::fermataLongAbove, SymId::fermataLongBelow,
            "longfermata", QT_TRANSLATE_NOOP("articulation", "long fermata"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::fermataVeryLongAbove, SymId::fermataVeryLongBelow,
            "verylongfermata", QT_TRANSLATE_NOOP("articulation", "very long fermata"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::stringsThumbPosition, SymId::stringsThumbPosition,
            "thumb", QT_TRANSLATE_NOOP("articulation", "thumb"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::articAccent,   SymId::articAccent,
            "sforzato", QT_TRANSLATE_NOOP("articulation", "sforzato"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
// <> not available in smufl?
//      { esprSym, esprSym             ,
//            "espressivo", QT_TRANSLATE_NOOP("articulation", "espressivo"),
//            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
//            },
      { SymId::articStaccato, SymId::articStaccato,
            "staccato", QT_TRANSLATE_NOOP("articulation", "staccato"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::articStaccatissimoAbove,   SymId::articStaccatissimoBelow,
            "staccatissimo", QT_TRANSLATE_NOOP("articulation", "staccatissimo"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::articTenuto, SymId::articTenuto,
            "tenuto", QT_TRANSLATE_NOOP("articulation", "tenuto"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::articTenutoSlurAbove, SymId::articTenutoSlurBelow,
            "portato", QT_TRANSLATE_NOOP("articulation", "portato"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::articMarcatoAbove, SymId::articMarcatoBelow,
            "marcato", QT_TRANSLATE_NOOP("articulation", "marcato"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::brassMuteOpen, SymId::brassMuteOpen,
            "ouvert", QT_TRANSLATE_NOOP("articulation", "ouvert"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::brassMuteClosed, SymId::brassMuteClosed,
            "plusstop", QT_TRANSLATE_NOOP("articulation", "stopped/pizzicato left hand"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::stringsUpBow, SymId::stringsUpBow,
            "upbow", QT_TRANSLATE_NOOP("articulation", "up bow"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::stringsDownBow, SymId::stringsDownBow,
            "downbow", QT_TRANSLATE_NOOP("articulation", "down bow"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::ornamentTurnInverted, SymId::ornamentTurnInverted,
            "reverseturn", QT_TRANSLATE_NOOP("articulation", "reverse turn"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::ornamentTurn, SymId::ornamentTurn,
            "turn", QT_TRANSLATE_NOOP("articulation", "turn"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::ornamentTrill, SymId::ornamentTrill,
            "trill", QT_TRANSLATE_NOOP("articulation", "trill"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::ornamentMordent, SymId::ornamentMordent,
            "prall", QT_TRANSLATE_NOOP("articulation", "prall"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::ornamentMordentInverted, SymId::ornamentMordentInverted,
            "mordent", QT_TRANSLATE_NOOP("articulation", "mordent"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::ornamentTremblement, SymId::ornamentTremblement,
            "prallprall", QT_TRANSLATE_NOOP("articulation", "prall prall"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::ornamentPrallMordent, SymId::ornamentPrallMordent,
            "prallmordent", QT_TRANSLATE_NOOP("articulation", "prall mordent"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::ornamentUpPrall, SymId::ornamentUpPrall,
            "upprall", QT_TRANSLATE_NOOP("articulation", "up prall"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ SymId::ornamentDownPrall, SymId::ornamentDownPrall,
            "downprall", QT_TRANSLATE_NOOP("articulation", "down prall"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ SymId::ornamentUpMordent, SymId::ornamentUpMordent,
            "upmordent", QT_TRANSLATE_NOOP("articulation", "up mordent"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ SymId::ornamentDownMordent, SymId::ornamentDownMordent,
            "downmordent", QT_TRANSLATE_NOOP("articulation", "down mordent"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ SymId::ornamentPrallDown, SymId::ornamentPrallDown,
            "pralldown", QT_TRANSLATE_NOOP("articulation", "prall down"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ SymId::ornamentPrallUp, SymId::ornamentPrallUp,
            "prallup", QT_TRANSLATE_NOOP("articulation", "prall up"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ SymId::ornamentLinePrall, SymId::ornamentLinePrall,
            "lineprall", QT_TRANSLATE_NOOP("articulation", "line prall"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ SymId::ornamentPrecompSlide, SymId::ornamentPrecompSlide,
            "schleifer", QT_TRANSLATE_NOOP("articulation", "schleifer"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { SymId::pluckedSnapPizzicatoAbove, SymId::pluckedSnapPizzicatoAbove,
            "snappizzicato", QT_TRANSLATE_NOOP("articulation", "snap pizzicato"),
            1.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },

#if 0
      { letterTSym, letterTSym,
            "tapping", QT_TRANSLATE_NOOP("articulation", "tapping"),
            1.0, ARTICULATION_SHOW_IN_TABLATURE
            },
      { letterSSym, letterSSym,
            "slapping", QT_TRANSLATE_NOOP("articulation", "slapping"),
            1.0, ARTICULATION_SHOW_IN_TABLATURE
            },
      { letterPSym, letterPSym,
            "popping", QT_TRANSLATE_NOOP("articulation", "popping"),
            1.0, ARTICULATION_SHOW_IN_TABLATURE
            },
#endif
	};

//---------------------------------------------------------
//   Articulation
//---------------------------------------------------------

Articulation::Articulation(Score* s)
   : Element(s)
      {
      _direction = MScore::AUTO;
      _up = true;
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      setArticulationType(Articulation_Fermata);
       }

//---------------------------------------------------------
//   setArticulationType
//---------------------------------------------------------

void Articulation::setArticulationType(ArticulationType idx)
      {
      _articulationType = idx;
      _anchor           = score()->style()->articulationAnchor(_articulationType);
      anchorStyle       = PropertyStyle::STYLED;
      _timeStretch      = articulationList[articulationType()].timeStretch;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Articulation::read(XmlReader& e)
      {
      setArticulationType(Articulation_Fermata);    // default // backward compatibility (no type = ufermata in 1.2)
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setSubtype(e.readElementText());
            else if (tag == "channel") {
                  _channelName = e.attribute("name");
                  e.readNext();
                  }
            else if (tag == "anchor") {
                  _anchor = ArticulationAnchor(e.readInt());
                  anchorStyle = PropertyStyle::UNSTYLED;
                  }
            else if (tag == "direction") {
                  setProperty(P_DIRECTION, Ms::getProperty(P_DIRECTION, e));
                  }
            else if (tag == "timeStretch")
                  _timeStretch = e.readDouble();
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Articulation::write(Xml& xml) const
      {
      xml.stag("Articulation");
      if (!_channelName.isEmpty())
            xml.tagE(QString("channel name=\"%1\"").arg(_channelName));
      writeProperty(xml, P_DIRECTION);
      xml.tag("subtype", subtypeName());
      if (_timeStretch != 1.0)
            xml.tag("timeStretch", _timeStretch);
      Element::writeProperties(xml);
      if (anchorStyle == PropertyStyle::UNSTYLED)
            xml.tag("anchor", int(_anchor));
      xml.etag();
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

QString Articulation::subtypeName() const
      {
      return articulationList[articulationType()].name;
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Articulation::setSubtype(const QString& s)
      {
      if (s.isEmpty()) {
            qDebug("Articulation::setSubtype: empty subtype");
            setArticulationType(Articulation_Fermata);            // something to debug...
            return;
            }

      if (s[0].isDigit()) {         // for backward compatibility
            setArticulationType(ArticulationType(s.toInt()));
            return;
            }
      int st;
      for (st = 0; st < ARTICULATIONS; ++st) {
            if (articulationList[st].name == s)
                  break;
            }
      if (st == ARTICULATIONS) {
            struct {
                  const char* name;
                  bool up;
                  ArticulationType type;
                  } al[] = {
                  { "umarcato",         true,  Articulation_Marcato },
                  { "dmarcato",         false, Articulation_Marcato },
                  { "ufermata",         true,  Articulation_Fermata },
                  { "dfermata",         false, Articulation_Fermata },
                  { "ushortfermata",    true,  Articulation_Shortfermata },
                  { "dshortfermata",    false, Articulation_Shortfermata },
                  { "ulongfermata",     true,  Articulation_Longfermata },
                  { "dlongfermata",     false, Articulation_Longfermata },
                  { "uverylongfermata", true,  Articulation_Verylongfermata },
                  { "dverylongfermata", false, Articulation_Verylongfermata },
                  // watch out, bug in 1.2 uportato and dportato are reversed
                  { "dportato",         true,  Articulation_Portato },
                  { "uportato",         false, Articulation_Portato },
                  { "ustaccatissimo",   true,  Articulation_Staccatissimo },
                  { "dstaccatissimo",   false, Articulation_Staccatissimo }
                  };

            int i;
            int n = sizeof(al) / sizeof(*al);
            for (i = 0; i < n; ++i) {
                  if (s == al[i].name) {
                        _up = al[i].up;
                        _direction = (_up ? MScore::UP : MScore::DOWN);
                        st  = int(al[i].type);
                        break;
                        }
                  }
            if (i == n) {
                  st = 0;
                  qDebug("Articulation: unknown <%s>", qPrintable(s));
                  }
            }
      setArticulationType(ArticulationType(st));
      }

//---------------------------------------------------------
//   idx2name
//---------------------------------------------------------

QString Articulation::idx2name(int idx)
      {
      return articulationList[idx].name;
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF Articulation::pagePos() const
      {
      if (parent() == 0)
            return pos();
      return parent()->pagePos() + pos();
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF Articulation::canvasPos() const
      {
      if (parent() == 0)
            return pos();
      return parent()->canvasPos() + pos();
      }

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void Articulation::draw(QPainter* painter) const
      {
      SymId sym = _up ? articulationList[articulationType()].upSym : articulationList[articulationType()].downSym;
      int flags = articulationList[articulationType()].flags;
      if (staff()) {
            if (staff()->staffGroup() == TAB_STAFF_GROUP) {
                  if (!(flags & ARTICULATION_SHOW_IN_TABLATURE))
                        return;
                  }
            else {
                  if (!(flags & ARTICULATION_SHOW_IN_PITCHED_STAFF))
                        return;
                  }
            }
      painter->setPen(curColor());
      drawSymbol(sym, painter, QPointF(-0.5 * width(), _up ? 0.0 : height()));
      }

//---------------------------------------------------------
//   chordRest
//---------------------------------------------------------

ChordRest* Articulation::chordRest() const
      {
      if (parent() && parent()->isChordRest())
            return static_cast<ChordRest*>(parent());
      return 0;
      }

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

QString Articulation::subtypeUserName() const
      {
      return articulationList[articulationType()].description;
      }

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void Articulation::layout()
      {
      SymId sym = _up ? articulationList[articulationType()].upSym : articulationList[articulationType()].downSym;
      QRectF b(symBbox(sym));
      setbbox(b.translated(-0.5 * b.width(), _up ? 0.0 : b.height()));
      }

//---------------------------------------------------------
//   setDirection
//---------------------------------------------------------

void Articulation::setDirection(MScore::Direction d)
      {
      _direction = d;
      if (d != MScore::AUTO)
            _up = (d == MScore::UP);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Articulation::reset()
      {
      if (_direction != MScore::AUTO)
            score()->undoChangeProperty(this, P_DIRECTION, int(MScore::AUTO));
      ArticulationAnchor a = score()->style()->articulationAnchor(articulationType());
      if (_anchor != a)
            score()->undoChangeProperty(this, P_ARTICULATION_ANCHOR, int(a));
      Element::reset();
      if (chordRest())
            chordRest()->layoutArticulations();
      score()->addRefresh(canvasBoundingRect());
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Articulation::dragAnchor() const
      {
      return QLineF(canvasPos(), parent()->canvasPos());
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Articulation::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_DIRECTION:           return int(direction());
            case P_ARTICULATION_ANCHOR: return int(anchor());
            case P_TIME_STRETCH:        return timeStretch();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Articulation::setProperty(P_ID propertyId, const QVariant& v)
      {
      score()->addRefresh(canvasBoundingRect());
      switch (propertyId) {
            case P_DIRECTION:
                  setDirection(MScore::Direction(v.toInt()));
                  break;
            case P_ARTICULATION_ANCHOR:
                  anchorStyle = PropertyStyle::UNSTYLED;
                  setAnchor(ArticulationAnchor(v.toInt()));
                  break;
            case P_TIME_STRETCH:
                  setTimeStretch(v.toDouble());
                  score()->fixTicks();
                  break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      score()->addRefresh(canvasBoundingRect());

      // layout:
      if (chordRest())
            chordRest()->layoutArticulations();
      else if (parent() && parent()->type() == BAR_LINE)
            static_cast<BarLine*>(parent())->layout();

      score()->addRefresh(canvasBoundingRect());
      score()->setLayoutAll(false);       //DEBUG
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Articulation::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_DIRECTION:
                  return int(MScore::AUTO);

            case P_ARTICULATION_ANCHOR:
                  return int(score()->style()->articulationAnchor(_articulationType));

            case P_TIME_STRETCH:
                  return articulationList[articulationType()].timeStretch;

            default:
                  break;
            }
      return Element::propertyDefault(propertyId);
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyStyle Articulation::propertyStyle(P_ID id) const
      {
      switch (id) {
            case P_DIRECTION:
            case P_TIME_STRETCH:
                  return PropertyStyle::NOSTYLE;

            case P_ARTICULATION_ANCHOR:
                  return anchorStyle;

            default:
                  break;
            }
      return Element::propertyStyle(id);
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Articulation::resetProperty(P_ID id)
      {
      switch (id) {
            case P_DIRECTION:
            case P_TIME_STRETCH:
                  return;

            case P_ARTICULATION_ANCHOR:
                  setProperty(id, propertyDefault(id));
                  anchorStyle = PropertyStyle::STYLED;
                  return;

            default:
                  break;
            }
      Element::resetProperty(id);
      }

//---------------------------------------------------------
//   styleChanged
//    reset all styled values to actual style
//---------------------------------------------------------

void Articulation::styleChanged()
      {
      if (anchorStyle == PropertyStyle::STYLED)
            _anchor = score()->style()->articulationAnchor(_articulationType);
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Articulation::mag() const
      {
      return parent() ? parent()->mag() * score()->styleD(ST_articulationMag): 1.0;
      }
}



