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

#define PT    ArticulationShowIn::PITCHED_STAFF | ArticulationShowIn::TABLATURE
#define TR(a) QT_TRANSLATE_NOOP("articulation", a)

ArticulationInfo Articulation::articulationList[int(ArticulationType::ARTICULATIONS)] = {
      { SymId::fermataAbove,              SymId::fermataBelow,              "fermata",                   1.0, PT },
      { SymId::fermataShortAbove,         SymId::fermataShortBelow,         "shortfermata",              1.0, PT },
      { SymId::fermataLongAbove,          SymId::fermataLongBelow,          "longfermata",               1.0, PT },
      { SymId::fermataVeryLongAbove,      SymId::fermataVeryLongBelow,      "verylongfermata",           1.0, PT },
      { SymId::articAccentAbove,          SymId::articAccentBelow,          "sforzato",                  1.0, PT },
      { SymId::articStaccatoAbove,        SymId::articStaccatoBelow,        "staccato",                  1.0, PT },
      { SymId::articStaccatissimoAbove,   SymId::articStaccatissimoBelow,   "staccatissimo",             1.0, PT },
      { SymId::articTenutoAbove,          SymId::articTenutoBelow,          "tenuto",                    1.0, PT },
      { SymId::articTenutoStaccatoAbove,  SymId::articTenutoStaccatoBelow,  "portato",                   1.0, PT },
      { SymId::articMarcatoAbove,         SymId::articMarcatoBelow,         "marcato",                   1.0, PT },
      { SymId::guitarFadeIn,              SymId::guitarFadeIn,              "fadein",                    1.0, PT },
      { SymId::guitarFadeOut,             SymId::guitarFadeOut,             "fadeout",                   1.0, PT },
      { SymId::guitarVolumeSwell,         SymId::guitarVolumeSwell,         "volumeswell",               1.0, PT },
      { SymId::wiggleSawtooth,            SymId::wiggleSawtooth,            "wigglesawtooth",            1.0, PT },
      { SymId::wiggleSawtoothWide,        SymId::wiggleSawtoothWide,        "wigglesawtoothwide",        1.0, PT },
      { SymId::wiggleVibratoLargeFaster,  SymId::wiggleVibratoLargeFaster,  "wigglevibratolargefaster",  1.0, PT },
      { SymId::wiggleVibratoLargeSlowest, SymId::wiggleVibratoLargeSlowest, "wigglevibratolargeslowest", 1.0, PT },
      { SymId::brassMuteOpen,             SymId::brassMuteOpen,             "ouvert",                    1.0, PT },
      { SymId::brassMuteClosed,           SymId::brassMuteClosed,           "plusstop",                  1.0, PT },
      { SymId::stringsUpBow,              SymId::stringsUpBow,              "upbow",                     1.0, PT },
      { SymId::stringsDownBow,            SymId::stringsDownBow,            "downbow",                   1.0, PT },
      { SymId::ornamentTurnInverted,      SymId::ornamentTurnInverted,      "reverseturn",               1.0, PT },
      { SymId::ornamentTurn,              SymId::ornamentTurn,              "turn",                      1.0, PT },
      { SymId::ornamentTrill,             SymId::ornamentTrill,             "trill",                     1.0, PT },
      { SymId::ornamentMordent,           SymId::ornamentMordent,           "prall",                     1.0, PT },
      { SymId::ornamentMordentInverted,   SymId::ornamentMordentInverted,   "mordent",                   1.0, PT },
      { SymId::ornamentTremblement,       SymId::ornamentTremblement,       "prallprall",                1.0, PT },
      { SymId::ornamentPrallMordent,      SymId::ornamentPrallMordent,      "prallmordent",              1.0, PT },
      { SymId::ornamentUpPrall,           SymId::ornamentUpPrall,           "upprall",                   1.0, PT },
      { SymId::ornamentDownPrall,         SymId::ornamentDownPrall,         "downprall",                 1.0, PT },
      { SymId::ornamentUpMordent,         SymId::ornamentUpMordent,         "upmordent",                 1.0, PT },
      { SymId::ornamentDownMordent,       SymId::ornamentDownMordent,       "downmordent",               1.0, PT },
      { SymId::ornamentPrallDown,         SymId::ornamentPrallDown,         "pralldown",                 1.0, PT },
      { SymId::ornamentPrallUp,           SymId::ornamentPrallUp,           "prallup",                   1.0, PT },
      { SymId::ornamentLinePrall,         SymId::ornamentLinePrall,         "lineprall",                 1.0, PT },
      { SymId::ornamentPrecompSlide,      SymId::ornamentPrecompSlide,      "schleifer",                 1.0, PT },
      { SymId::pluckedSnapPizzicatoAbove, SymId::pluckedSnapPizzicatoBelow, "snappizzicato",             1.0, PT },

      // Fingerings

      { SymId::stringsThumbPosition,      SymId::stringsThumbPosition,      "thumb",                     1.0, PT },
      { SymId::luteFingeringRHThumb,      SymId::luteFingeringRHThumb,      "lutefingeringthumb",        1.0, ArticulationShowIn::TABLATURE },
      { SymId::luteFingeringRHFirst,      SymId::luteFingeringRHFirst,      "lutefingering1st",          1.0, ArticulationShowIn::TABLATURE },
      { SymId::luteFingeringRHSecond,     SymId::luteFingeringRHSecond,     "lutefingering2nd",          1.0, ArticulationShowIn::TABLATURE },
      { SymId::luteFingeringRHThird,      SymId::luteFingeringRHThird,      "lutefingering3rd",          1.0, ArticulationShowIn::TABLATURE },
      };
#undef PT
#undef TR

//---------------------------------------------------------
//   Articulation
//---------------------------------------------------------

Articulation::Articulation(Score* s)
   : Element(s)
      {
      _direction = Direction_AUTO;
      _up = true;
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
      setArticulationType(ArticulationType::Fermata);
      _ornamentStyle    = MScore::OrnamentStyle::DEFAULT;
      setPlayArticulation(true);
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

      // TODO: layout() can be empty?
      SymId sym = _up ? articulationList[int(_articulationType)].upSym : articulationList[int(_articulationType)].downSym;
      QRectF b(symBbox(sym));
      setbbox(b.translated(-0.5 * b.width(), 0.0));
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
            else if (tag == "direction")
                  readProperty(e, P_ID::DIRECTION);
            else if ( tag == "ornamentStyle")
                  setProperty(P_ID::ORNAMENT_STYLE, Ms::getProperty(P_ID::ORNAMENT_STYLE, e));
            else if ( tag == "play")
                  setPlayArticulation(e.readBool());
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
      if (!xml.canWrite(this))
            return;
      xml.stag("Articulation");
      if (!_channelName.isEmpty())
            xml.tagE(QString("channel name=\"%1\"").arg(_channelName));
      writeProperty(xml, P_ID::DIRECTION);
      xml.tag("subtype", subtypeName());
      writeProperty(xml, P_ID::TIME_STRETCH);
      writeProperty(xml, P_ID::PLAY);
      writeProperty(xml, P_ID::ORNAMENT_STYLE);
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
                  { "fadein",                 true,  ArticulationType::FadeIn },
                  { "fadeout",                true,  ArticulationType::FadeOut },
                  { "volumeswell",            true,  ArticulationType::VolumeSwell },
                  { "wigglesawtooth",         true,  ArticulationType::WiggleSawtooth },
                  { "wigglesawtoothwide",     true,  ArticulationType::WiggleSawtoothWide },
                  { "wigglevibratolargefaster",  true,  ArticulationType::WiggleVibratoLargeFaster },
                  { "wigglevibratolargeslowest", true,  ArticulationType::WiggleVibratoLargeSlowest },
                  { "umarcato",               true,  ArticulationType::Marcato },
                  { "dmarcato",               false, ArticulationType::Marcato },
                  { "ufermata",               true,  ArticulationType::Fermata },
                  { "dfermata",               false, ArticulationType::Fermata },
                  { "ushortfermata",          true,  ArticulationType::Shortfermata },
                  { "dshortfermata",          false, ArticulationType::Shortfermata },
                  { "ulongfermata",           true,  ArticulationType::Longfermata },
                  { "dlongfermata",           false, ArticulationType::Longfermata },
                  { "uverylongfermata",       true,  ArticulationType::Verylongfermata },
                  { "dverylongfermata",       false, ArticulationType::Verylongfermata },
                  // watch out, bug in 1.2 uportato and dportato are reversed
                  { "dportato",               true,  ArticulationType::Portato },
                  { "uportato",               false, ArticulationType::Portato },
                  { "ustaccatissimo",         true,  ArticulationType::Staccatissimo },
                  { "dstaccatissimo",         false, ArticulationType::Staccatissimo }
                  };

            int i;
            int n = sizeof(al) / sizeof(*al);
            for (i = 0; i < n; ++i) {
                  if (s == al[i].name) {
                        _up = al[i].up;
                        _direction = (_up ? Direction_UP : Direction_DOWN);
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
//   Symbol::draw
//---------------------------------------------------------

void Articulation::draw(QPainter* painter) const
      {
      SymId sym = symId();
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
//   symId
//---------------------------------------------------------

SymId Articulation::symId() const
      {
      return _up ? articulationList[int(articulationType())].upSym : articulationList[int(articulationType())].downSym;
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

Segment* Articulation::segment() const
      {
      ChordRest* cr = chordRest();
      if (!cr)
            return nullptr;

      Segment* s = nullptr;
      if (cr->isGrace()) {
            if (cr->parent())
                  s = static_cast<Segment*>(cr->parent()->parent());
            }
      else
            s = static_cast<Segment*>(cr->parent());

      return s;
      }

Measure* Articulation::measure() const
      {
      Segment* s = segment();
      return static_cast<Measure*>(s ? s->parent() : 0);
      }

System* Articulation::system() const
      {
      Measure* m = measure();
      return static_cast<System*>(m ? m->parent() : 0);
      }

Page* Articulation::page() const
      {
      System* s = system();
      return static_cast<Page*>(s ? s->parent() : 0);
      }

//---------------------------------------------------------
//   userName
//---------------------------------------------------------

QString Articulation::userName() const
      {
      return Sym::id2userName(symId());
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

void Articulation::setDirection(Direction d)
      {
      _direction = d;
      if (d != Direction_AUTO)
            _up = (d == Direction_UP);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Articulation::reset()
      {
      if (_direction != Direction_AUTO)
            undoChangeProperty(P_ID::DIRECTION, Direction_AUTO);
      ArticulationAnchor a = score()->style()->articulationAnchor(int(articulationType()));
      if (_anchor != a)
            undoChangeProperty(P_ID::ARTICULATION_ANCHOR, int(a));
      Element::reset();
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
      switch (propertyId) {
            case P_ID::DIRECTION:           return direction();
            case P_ID::ARTICULATION_ANCHOR: return int(anchor());
            case P_ID::TIME_STRETCH:        return timeStretch();
            case P_ID::ORNAMENT_STYLE:      return int(ornamentStyle());
            case P_ID::PLAY:                return bool(playArticulation());
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Articulation::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::DIRECTION:
                  setDirection(v.value<Direction>());
                  break;
            case P_ID::ARTICULATION_ANCHOR:
                  anchorStyle = PropertyStyle::UNSTYLED;
                  setAnchor(ArticulationAnchor(v.toInt()));
                  break;
            case P_ID::PLAY:
                  setPlayArticulation(v.toBool());
                  break;
            case P_ID::ORNAMENT_STYLE:
                  setOrnamentStyle(MScore::OrnamentStyle(v.toInt()));
                  break;
            case P_ID::TIME_STRETCH:
                  setTimeStretch(v.toDouble());
                  score()->fixTicks();
                  break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Articulation::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::DIRECTION:
                  return Direction_AUTO;

            case P_ID::ARTICULATION_ANCHOR:
                  return int(score()->style()->articulationAnchor(int(_articulationType)));

            case P_ID::TIME_STRETCH:
                  return articulationList[int(articulationType())].timeStretch;

            case P_ID::ORNAMENT_STYLE:
                  //return int(score()->style()->ornamentStyle(_ornamentStyle));
                  return int(MScore::OrnamentStyle::DEFAULT);

            case P_ID::PLAY:
                  return true;

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
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx Articulation::getPropertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::ARTICULATION_ANCHOR:
                  return MStyle::articulationAnchorIdx(int(_articulationType));
            default:
                  break;
            }
      return StyleIdx::NOSTYLE;
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Articulation::resetProperty(P_ID id)
      {
      switch (id) {
            case P_ID::DIRECTION:
            case P_ID::TIME_STRETCH:
            case P_ID::ORNAMENT_STYLE:
                  setProperty(id, propertyDefault(id));
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

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Articulation::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(userName());
      }

}



