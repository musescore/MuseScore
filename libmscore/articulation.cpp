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
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   Articulation::articulationList
//---------------------------------------------------------

ArticulationInfo Articulation::articulationList[int(ArticulationType::ARTICULATIONS)] = {
      { SymId::fermataAbove, SymId::fermataBelow,
            "fermata", QT_TRANSLATE_NOOP("articulation", "fermata"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::fermataShortAbove, SymId::fermataShortBelow,
            "shortfermata", QT_TRANSLATE_NOOP("articulation", "short fermata"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::fermataLongAbove, SymId::fermataLongBelow,
            "longfermata", QT_TRANSLATE_NOOP("articulation", "long fermata"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::fermataVeryLongAbove, SymId::fermataVeryLongBelow,
            "verylongfermata", QT_TRANSLATE_NOOP("articulation", "very long fermata"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::stringsThumbPosition, SymId::stringsThumbPosition,
            "thumb", QT_TRANSLATE_NOOP("articulation", "thumb"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::articAccentAbove,   SymId::articAccentBelow,
            "sforzato", QT_TRANSLATE_NOOP("articulation", "sforzato"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
// <> not available in smufl?
//      { esprSym, esprSym             ,
//            "espressivo", QT_TRANSLATE_NOOP("articulation", "espressivo"),
//            1.0,ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
//            },
      { SymId::articStaccatoAbove, SymId::articStaccatoBelow,
            "staccato", QT_TRANSLATE_NOOP("articulation", "staccato"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::articStaccatissimoAbove,   SymId::articStaccatissimoBelow,
            "staccatissimo", QT_TRANSLATE_NOOP("articulation", "staccatissimo"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::articTenutoAbove, SymId::articTenutoBelow,
            "tenuto", QT_TRANSLATE_NOOP("articulation", "tenuto"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::articTenutoStaccatoAbove, SymId::articTenutoStaccatoBelow,
            "portato", QT_TRANSLATE_NOOP("articulation", "portato"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::articMarcatoAbove, SymId::articMarcatoBelow,
            "marcato", QT_TRANSLATE_NOOP("articulation", "marcato"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::guitarFadeIn, SymId::guitarFadeIn,
            "fadein", QT_TRANSLATE_NOOP("articulation", "fade in"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::guitarFadeOut, SymId::guitarFadeOut,
            "fadeout", QT_TRANSLATE_NOOP("articulation", "fade out"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::brassMuteOpen, SymId::brassMuteOpen,
            "ouvert", QT_TRANSLATE_NOOP("articulation", "ouvert"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::brassMuteClosed, SymId::brassMuteClosed,
            "plusstop", QT_TRANSLATE_NOOP("articulation", "stopped/pizzicato left hand"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::stringsUpBow, SymId::stringsUpBow,
            "upbow", QT_TRANSLATE_NOOP("articulation", "up bow"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::stringsDownBow, SymId::stringsDownBow,
            "downbow", QT_TRANSLATE_NOOP("articulation", "down bow"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::ornamentTurnInverted, SymId::ornamentTurnInverted,
            "reverseturn", QT_TRANSLATE_NOOP("articulation", "reverse turn"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::ornamentTurn, SymId::ornamentTurn,
            "turn", QT_TRANSLATE_NOOP("articulation", "turn"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::ornamentTrill, SymId::ornamentTrill,
            "trill", QT_TRANSLATE_NOOP("articulation", "trill"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::ornamentMordent, SymId::ornamentMordent,
            "prall", QT_TRANSLATE_NOOP("articulation", "prall"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::ornamentMordentInverted, SymId::ornamentMordentInverted,
            "mordent", QT_TRANSLATE_NOOP("articulation", "mordent"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::ornamentTremblement, SymId::ornamentTremblement,
            "prallprall", QT_TRANSLATE_NOOP("articulation", "prall prall"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::ornamentPrallMordent, SymId::ornamentPrallMordent,
            "prallmordent", QT_TRANSLATE_NOOP("articulation", "prall mordent"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::ornamentUpPrall, SymId::ornamentUpPrall,
            "upprall", QT_TRANSLATE_NOOP("articulation", "up prall"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::ornamentDownPrall, SymId::ornamentDownPrall,
            "downprall", QT_TRANSLATE_NOOP("articulation", "down prall"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::ornamentUpMordent, SymId::ornamentUpMordent,
            "upmordent", QT_TRANSLATE_NOOP("articulation", "up mordent"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::ornamentDownMordent, SymId::ornamentDownMordent,
            "downmordent", QT_TRANSLATE_NOOP("articulation", "down mordent"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::ornamentPrallDown, SymId::ornamentPrallDown,
            "pralldown", QT_TRANSLATE_NOOP("articulation", "prall down"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::ornamentPrallUp, SymId::ornamentPrallUp,
            "prallup", QT_TRANSLATE_NOOP("articulation", "prall up"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::ornamentLinePrall, SymId::ornamentLinePrall,
            "lineprall", QT_TRANSLATE_NOOP("articulation", "line prall"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::ornamentPrecompSlide, SymId::ornamentPrecompSlide,
            "schleifer", QT_TRANSLATE_NOOP("articulation", "schleifer"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },
      { SymId::pluckedSnapPizzicatoAbove, SymId::pluckedSnapPizzicatoBelow,
            "snappizzicato", QT_TRANSLATE_NOOP("articulation", "snap pizzicato"),
            1.0, ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
            },

#if 0
      { letterTSym, letterTSym,
            "tapping", QT_TRANSLATE_NOOP("articulation", "tapping"),
            1.0, ArticulationShowIn::TABLATURE
            },
      { letterSSym, letterSSym,
            "slapping", QT_TRANSLATE_NOOP("articulation", "slapping"),
            1.0, ArticulationShowIn::TABLATURE
            },
      { letterPSym, letterPSym,
            "popping", QT_TRANSLATE_NOOP("articulation", "popping"),
            1.0, ArticulationShowIn::TABLATURE
            },
#endif
      };

//---------------------------------------------------------
//   Articulation
//---------------------------------------------------------

Articulation::Articulation(Score* s)
   : Element(s)
      {
      _direction = MScore::Direction::AUTO;
      _up = true;
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
      setArticulationType(ArticulationType::Fermata);
       }

//---------------------------------------------------------
//   setArticulationType
//---------------------------------------------------------

void Articulation::setArticulationType(ArticulationType idx)
      {
      _articulationType = idx;
      _anchor           = score()->style()->articulationAnchor(int(_articulationType));
      anchorStyle       = PropertyStyle::STYLED;
      _timeStretch      = articulationList[int(articulationType())].timeStretch;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Articulation::read(XmlReader& e)
      {
      setArticulationType(ArticulationType::Fermata);    // default // backward compatibility (no type = ufermata in 1.2)
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
                  setProperty(P_ID::DIRECTION, Ms::getProperty(P_ID::DIRECTION, e));
                  }
            else if (tag == "timeStretch")
                  _timeStretch = e.readDouble();
            else if (tag == "offset") {
                  if (score()->mscVersion() > 114)
                        Element::readProperties(e);
                  else
                        e.skipCurrentElement(); // ignore manual layout in older scores
                  }
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Articulation::write(Xml& xml) const
      {
      if (!xml.canWrite(this)) return;
      xml.stag("Articulation");
      if (!_channelName.isEmpty())
            xml.tagE(QString("channel name=\"%1\"").arg(_channelName));
      writeProperty(xml, P_ID::DIRECTION);
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
      return articulationList[int(articulationType())].name;
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Articulation::setSubtype(const QString& s)
      {
      if (s.isEmpty()) {
            qDebug("Articulation::setSubtype: empty subtype");
            setArticulationType(ArticulationType::Fermata);            // something to debug...
            return;
            }

      if (s[0].isDigit()) {         // for backward compatibility
            setArticulationType(ArticulationType(s.toInt()));
            return;
            }
      int st;
      for (st = 0; st < int(ArticulationType::ARTICULATIONS); ++st) {
            if (articulationList[st].name == s)
                  break;
            }
      if (st == int(ArticulationType::ARTICULATIONS)) {
            struct {
                  const char* name;
                  bool up;
                  ArticulationType type;
                  } al[] = {
                  { "fadein",           true,  ArticulationType::FadeIn },
                  { "fadeout",          true,  ArticulationType::FadeOut },
                  { "umarcato",         true,  ArticulationType::Marcato },
                  { "dmarcato",         false, ArticulationType::Marcato },
                  { "ufermata",         true,  ArticulationType::Fermata },
                  { "dfermata",         false, ArticulationType::Fermata },
                  { "ushortfermata",    true,  ArticulationType::Shortfermata },
                  { "dshortfermata",    false, ArticulationType::Shortfermata },
                  { "ulongfermata",     true,  ArticulationType::Longfermata },
                  { "dlongfermata",     false, ArticulationType::Longfermata },
                  { "uverylongfermata", true,  ArticulationType::Verylongfermata },
                  { "dverylongfermata", false, ArticulationType::Verylongfermata },
                  // watch out, bug in 1.2 uportato and dportato are reversed
                  { "dportato",         true,  ArticulationType::Portato },
                  { "uportato",         false, ArticulationType::Portato },
                  { "ustaccatissimo",   true,  ArticulationType::Staccatissimo },
                  { "dstaccatissimo",   false, ArticulationType::Staccatissimo }
                  };

            int i;
            int n = sizeof(al) / sizeof(*al);
            for (i = 0; i < n; ++i) {
                  if (s == al[i].name) {
                        _up = al[i].up;
                        _direction = (_up ? MScore::Direction::UP : MScore::Direction::DOWN);
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
      SymId sym = _up ? articulationList[int(articulationType())].upSym : articulationList[int(articulationType())].downSym;
      ArticulationShowIn flags = articulationList[int(articulationType())].flags;
      if (staff()) {
            if (staff()->staffGroup() == StaffGroup::TAB) {
                  if (!(flags & ArticulationShowIn::TABLATURE))
                        return;
                  }
            else {
                  if (!(flags & ArticulationShowIn::PITCHED_STAFF))
                        return;
                  }
            }
      painter->setPen(curColor());
      drawSymbol(sym, painter, QPointF(-0.5 * width(), 0.0));
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
      return articulationList[int(articulationType())].description;
      }

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void Articulation::layout()
      {
      SymId sym = _up ? articulationList[int(articulationType())].upSym : articulationList[int(articulationType())].downSym;
      QRectF b(symBbox(sym));
      setbbox(b.translated(-0.5 * b.width(), 0.0));
      }

//---------------------------------------------------------
//   setDirection
//---------------------------------------------------------

void Articulation::setDirection(MScore::Direction d)
      {
      _direction = d;
      if (d != MScore::Direction::AUTO)
            _up = (d == MScore::Direction::UP);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Articulation::reset()
      {
      if (_direction != MScore::Direction::AUTO)
            score()->undoChangeProperty(this, P_ID::DIRECTION, int(MScore::Direction::AUTO));
      ArticulationAnchor a = score()->style()->articulationAnchor(int(articulationType()));
      if (_anchor != a)
            score()->undoChangeProperty(this, P_ID::ARTICULATION_ANCHOR, int(a));
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
            case P_ID::DIRECTION:           return int(direction());
            case P_ID::ARTICULATION_ANCHOR: return int(anchor());
            case P_ID::TIME_STRETCH:        return timeStretch();
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
            case P_ID::DIRECTION:
                  setDirection(MScore::Direction(v.toInt()));
                  break;
            case P_ID::ARTICULATION_ANCHOR:
                  anchorStyle = PropertyStyle::UNSTYLED;
                  setAnchor(ArticulationAnchor(v.toInt()));
                  break;
            case P_ID::TIME_STRETCH:
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
      else if (parent() && parent()->type() == Element::Type::BAR_LINE)
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
            case P_ID::DIRECTION:
                  return int(MScore::Direction::AUTO);

            case P_ID::ARTICULATION_ANCHOR:
                  return int(score()->style()->articulationAnchor(int(_articulationType)));

            case P_ID::TIME_STRETCH:
                  return articulationList[int(articulationType())].timeStretch;

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
            case P_ID::DIRECTION:
            case P_ID::TIME_STRETCH:
                  return PropertyStyle::NOSTYLE;

            case P_ID::ARTICULATION_ANCHOR:
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
            case P_ID::DIRECTION:
            case P_ID::TIME_STRETCH:
                  return;

            case P_ID::ARTICULATION_ANCHOR:
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
            _anchor = score()->style()->articulationAnchor(int(_articulationType));
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Articulation::mag() const
      {
      return parent() ? parent()->mag() * score()->styleD(StyleIdx::articulationMag): 1.0;
      }
}



