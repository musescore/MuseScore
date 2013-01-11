//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: articulation.cpp 5604 2012-05-04 15:29:13Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
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

//---------------------------------------------------------
//   Articulation::articulationList
//---------------------------------------------------------

ArticulationInfo Articulation::articulationList[ARTICULATIONS] = {
      { ufermataSym, dfermataSym,
            "fermata", QT_TRANSLATE_NOOP("articulation", "fermata"),
            2.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { ushortfermataSym, dshortfermataSym,
            "shortfermata", QT_TRANSLATE_NOOP("articulation", "shortfermata"),
            1.5, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { ulongfermataSym, dlongfermataSym,
            "longfermata", QT_TRANSLATE_NOOP("articulation", "longfermata"),
            3.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { uverylongfermataSym, dverylongfermataSym,
            "verylongfermata", QT_TRANSLATE_NOOP("articulation", "verylongfermata"),
            4.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { thumbSym, thumbSym,
            "thumb", QT_TRANSLATE_NOOP("articulation", "thumb"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { sforzatoaccentSym,   sforzatoaccentSym,
            "sforzato", QT_TRANSLATE_NOOP("articulation", "sforzato"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { esprSym, esprSym             ,
            "espressivo", QT_TRANSLATE_NOOP("articulation", "espressivo"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { staccatoSym, staccatoSym,
            "staccato", QT_TRANSLATE_NOOP("articulation", "staccato"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF
            },
      { ustaccatissimoSym,   dstaccatissimoSym,
            "staccatissimo", QT_TRANSLATE_NOOP("articulation", "staccatissimo"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF
            },
      { tenutoSym, tenutoSym,
            "tenuto", QT_TRANSLATE_NOOP("articulation", "tenuto"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { dportatoSym, uportatoSym,
            "portato", QT_TRANSLATE_NOOP("articulation", "portato"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { umarcatoSym, dmarcatoSym,
            "marcato", QT_TRANSLATE_NOOP("articulation", "marcato"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { ouvertSym, ouvertSym,
            "ouvert", QT_TRANSLATE_NOOP("articulation", "ouvert"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { plusstopSym, plusstopSym,
            "plusstop", QT_TRANSLATE_NOOP("articulation", "plusstop"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { upbowSym, upbowSym,
            "upbow", QT_TRANSLATE_NOOP("articulation", "upbow"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { downbowSym, downbowSym,
            "downbow", QT_TRANSLATE_NOOP("articulation", "downbow"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { reverseturnSym, reverseturnSym,
            "reverseturn", QT_TRANSLATE_NOOP("articulation", "reverseturn"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { turnSym, turnSym,
            "turn", QT_TRANSLATE_NOOP("articulation", "turn"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { trillSym, trillSym,
            "trill", QT_TRANSLATE_NOOP("articulation", "trill"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { prallSym, prallSym,
            "prall", QT_TRANSLATE_NOOP("articulation", "prall"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { mordentSym, mordentSym,
            "mordent", QT_TRANSLATE_NOOP("articulation", "mordent"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { prallprallSym, prallprallSym,
            "prallprall", QT_TRANSLATE_NOOP("articulation", "prallprall"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { prallmordentSym, prallmordentSym,
            "prallmordent", QT_TRANSLATE_NOOP("articulation", "prallmordent"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { upprallSym, upprallSym,
            "upprall", QT_TRANSLATE_NOOP("articulation", "upprall"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ downprallSym, downprallSym,
            "downprall", QT_TRANSLATE_NOOP("articulation", "downprall"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ upmordentSym, upmordentSym,
            "upmordent", QT_TRANSLATE_NOOP("articulation", "upmordent"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ downmordentSym, downmordentSym,
            "downmordent", QT_TRANSLATE_NOOP("articulation", "downmordent"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ pralldownSym, pralldownSym,
            "pralldown", QT_TRANSLATE_NOOP("articulation", "pralldown"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ prallupSym, prallupSym,
            "prallup", QT_TRANSLATE_NOOP("articulation", "prallup"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ lineprallSym, lineprallSym,
            "lineprall", QT_TRANSLATE_NOOP("articulation", "lineprall"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ schleiferSym, schleiferSym,
            "schleifer", QT_TRANSLATE_NOOP("articulation", "schleifer"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { snappizzicatoSym, snappizzicatoSym,
            "snappizzicato", QT_TRANSLATE_NOOP("articulation", "snappizzicato"),
            0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { letterTSym, letterTSym,
            "tapping", QT_TRANSLATE_NOOP("articulation", "tapping"),
            0., ARTICULATION_SHOW_IN_TABLATURE
            },
      { letterSSym, letterSSym,
            "slapping", QT_TRANSLATE_NOOP("articulation", "slapping"),
            0., ARTICULATION_SHOW_IN_TABLATURE
            },
      { letterPSym, letterPSym,
            "popping", QT_TRANSLATE_NOOP("articulation", "popping"),
            0., ARTICULATION_SHOW_IN_TABLATURE
            },
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
      setSubtype(Articulation_Fermata);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Articulation::setSubtype(ArticulationType idx)
      {
      _subtype = idx;
      _anchor = score()->style()->articulationAnchor(idx);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Articulation::read(XmlReader& e)
      {
      setSubtype(Articulation_Fermata);    // default // backward compatibility (no type = ufermata in 1.2)
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setSubtype(e.readElementText());
            else if (tag == "channel") {
                  _channelName = e.attribute("name");
                  e.readNext();
                  }
            else if (tag == "anchor")
                  _anchor = ArticulationAnchor(e.readInt());
            else if (tag == "direction") {
                  MScore::Direction dir = MScore::AUTO;
                  QString val = e.readElementText();
                  if (val == "up")
                        dir = MScore::UP;
                  else if (val == "down")
                        dir = MScore::DOWN;
                  else if (val == "auto")
                        dir = MScore::AUTO;
                  else
                        e.unknown();
                  setDirection(dir);
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
      xml.stag("Articulation");
      if (!_channelName.isEmpty())
            xml.tagE(QString("channel name=\"%1\"").arg(_channelName));
      switch(_direction) {
            case MScore::UP:
                  xml.tag("direction", QVariant("up"));
                  break;
            case MScore::DOWN:
                  xml.tag("direction", QVariant("down"));
                  break;
            case MScore::AUTO:
                  break;
            }
      xml.tag("subtype", subtypeName());
      Element::writeProperties(xml);
      if (_anchor != score()->style()->articulationAnchor(subtype()))
            xml.tag("anchor", int(_anchor));
      xml.etag();
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

QString Articulation::subtypeName() const
      {
      return articulationList[subtype()].name;
      }

//---------------------------------------------------------
//   timeStretch
//---------------------------------------------------------

qreal Articulation::timeStretch() const
      {
      return articulationList[subtype()].timeStretch;
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Articulation::setSubtype(const QString& s)
      {
      if (s[0].isDigit()) {         // for backward compatibility
            setSubtype(ArticulationType(s.toInt()));
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
                  qDebug("Articulation: unknown <%s>\n", qPrintable(s));
                  }
            }
      setSubtype(ArticulationType(st));
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
      SymId sym = _up ? articulationList[subtype()].upSym : articulationList[subtype()].downSym;
      int flags = articulationList[subtype()].flags;
      if (staff()) {
            if (staff()->staffGroup() == TAB_STAFF) {
                  if (!(flags & ARTICULATION_SHOW_IN_TABLATURE))
                        return;
                  }
            else {
                  if (!(flags & ARTICULATION_SHOW_IN_PITCHED_STAFF))
                        return;
                  }
            }
      painter->setPen(curColor());
      symbols[score()->symIdx()][sym].draw(painter, magS());
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
//   articulationType
//---------------------------------------------------------

ArticulationType Articulation::articulationType() const
      {
      return ArticulationType(subtype());
      }

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

QString Articulation::subtypeUserName() const
      {
      return articulationList[subtype()].description;
      }

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void Articulation::layout()
      {
      SymId sym = _up ? articulationList[subtype()].upSym : articulationList[subtype()].downSym;
      setbbox(score()->sym(sym).bbox(magS()));
      }

//---------------------------------------------------------
//   setDirection
//---------------------------------------------------------

void Articulation::setDirection(MScore::Direction d)
      {
      _direction = d;
      if (d != MScore::AUTO)
            _up = (d == MScore::UP);
//      qDebug("setDirection %p %d %d\n", this, _up, int(d));
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Articulation::reset()
      {
      if (_direction != MScore::AUTO)
            score()->undoChangeProperty(this, P_DIRECTION, int(MScore::AUTO));
      ArticulationAnchor a = score()->style()->articulationAnchor(subtype());
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
      switch(propertyId) {
            case P_DIRECTION:           setDirection(MScore::Direction(v.toInt())); break;
            case P_ARTICULATION_ANCHOR: setAnchor(ArticulationAnchor(v.toInt())); break;
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

