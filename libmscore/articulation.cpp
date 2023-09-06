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
//   articulationStyle
//---------------------------------------------------------

static const ElementStyle articulationStyle {
      { Sid::articulationMinDistance, Pid::MIN_DISTANCE },
//      { Sid::articulationOffset, Pid::OFFSET },
      { Sid::articulationAnchorDefault, Pid::ARTICULATION_ANCHOR },
      };

//---------------------------------------------------------
//   Articulation
//---------------------------------------------------------

Articulation::Articulation(Score* s)
   : Element(s, ElementFlag::MOVABLE)
      {
      _symId         = SymId::noSym;
      _anchor        = ArticulationAnchor::TOP_STAFF;
      _direction     = Direction::AUTO;
      _up            = true;
      _ornamentStyle = MScore::OrnamentStyle::DEFAULT;
      setPlayArticulation(true);
      initElementStyle(&articulationStyle);
      }

Articulation::Articulation(SymId id, Score* s)
   : Articulation(s)
      {
      setSymId(id);
      }

//---------------------------------------------------------
//   setSymId
//---------------------------------------------------------

void Articulation::setSymId(SymId id)
      {
      _symId  = id;
      _anchor = ArticulationAnchor(propertyDefault(Pid::ARTICULATION_ANCHOR).toInt());
      }

//---------------------------------------------------------
//   subtype
//---------------------------------------------------------

int Articulation::subtype() const
      {
      QString s = Sym::id2name(_symId);
      if (s.endsWith("Below"))
            return int(Sym::name2id(s.left(s.size() - 5) + "Above"));
      else if (s.endsWith("Turned"))
            return int(Sym::name2id(s.left(s.size() - 6)));
      else
            return int(_symId);
      }

//---------------------------------------------------------
//   setUp
//---------------------------------------------------------

void Articulation::setUp(bool val)
      {
      _up = val;
      bool dup = _direction == Direction::AUTO ? val : _direction == Direction::UP;
      QString s = Sym::id2name(_symId);
      if (s.endsWith(!dup ? "Above" : "Below")) {
            QString s2 = s.left(s.size() - 5) + (dup ? "Above" : "Below");
            _symId = Sym::name2id(s2);
            }
      else if (s.endsWith("Turned")) {
            QString s2 = dup ? s.left(s.size() - 6) : s;
            _symId = Sym::name2id(s2);
            }
      else if (!dup) {
            QString s2 = s + "Turned";
            SymId sym = Sym::name2id(s2);
            if (sym != SymId::noSym)
                  _symId = sym;
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Articulation::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      }

extern SymId oldArticulationNames2SymId(const QString&);

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Articulation::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "subtype") {
            QString s = e.readElementText();
            SymId id = Sym::name2id(s);
            if (id == SymId::noSym)
                  id = oldArticulationNames2SymId(s);       // compatibility hack for "old" 3.0 scores
            if (id == SymId::noSym || s == "ornamentMordentInverted") // SMuFL < 1.30
                  id = SymId::ornamentMordent;

            QString programVersion = masterScore()->mscoreVersion();
            if (!programVersion.isEmpty() && programVersion < "3.6") {
                  if (id == SymId::noSym || s == "ornamentMordent") // SMuFL < 1.30 and MuseScore < 3.6
                        id = SymId::ornamentShortTrill;
                  }
            setSymId(id);
            }
      else if (tag == "channel") {
            _channelName = e.attribute("name");
            e.readNext();
            }
      else if (readProperty(tag, e, Pid::ARTICULATION_ANCHOR))
            ;
      else if (tag == "direction")
            readProperty(e, Pid::DIRECTION);
      else if ( tag == "ornamentStyle")
            readProperty(e, Pid::ORNAMENT_STYLE);
      else if ( tag == "play")
            setPlayArticulation(e.readBool());
      else if (tag == "offset") {
            if (score()->mscVersion() > 114)
                  Element::readProperties(e);
            else
                  e.skipCurrentElement(); // ignore manual layout in older scores
            }
      else if (Element::readProperties(e))
            ;
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Articulation::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this);
      if (!_channelName.isEmpty())
            xml.tagE(QString("channel name=\"%1\"").arg(_channelName));
      writeProperty(xml, Pid::DIRECTION);
      xml.tag("subtype", Sym::id2name(_symId));
      writeProperty(xml, Pid::PLAY);
      writeProperty(xml, Pid::ORNAMENT_STYLE);
      for (const StyledProperty& spp : *styledProperties())
            writeProperty(xml, spp.pid);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   userName
//---------------------------------------------------------

QString Articulation::userName() const
      {
      return Sym::id2userName(symId());
      }

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void Articulation::draw(QPainter* painter) const
      {
#if 0 //TODO
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
#endif
      painter->setPen(curColor());
      drawSymbol(_symId, painter, QPointF(-0.5 * width(), 0.0));
      }

//---------------------------------------------------------
//   chordRest
//---------------------------------------------------------

ChordRest* Articulation::chordRest() const
      {
      if (parent() && parent()->isChordRest())
            return toChordRest(parent());
      return 0;
      }

Segment* Articulation::segment() const
      {
      ChordRest* cr = chordRest();
      if (!cr)
            return 0;

      Segment* s = 0;
      if (cr->isGrace()) {
            if (cr->parent())
                  s = toSegment(cr->parent()->parent());
            }
      else
            s = toSegment(cr->parent());

      return s;
      }

Measure* Articulation::measure() const
      {
      Segment* s = segment();
      return toMeasure(s ? s->parent() : 0);
      }

System* Articulation::system() const
      {
      Measure* m = measure();
      return toSystem(m ? m->parent() : 0);
      }

Page* Articulation::page() const
      {
      System* s = system();
      return toPage(s ? s->parent() : 0);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Articulation::layout()
      {
      QRectF b(symBbox(_symId));
      setbbox(b.translated(-0.5 * b.width(), 0.0));
      }

//---------------------------------------------------------
//   layoutCloseToNote
//    Needed to figure out the layout policy regarding
//    distance to the note and placement in relation to
//    slur.
//---------------------------------------------------------

bool Articulation::layoutCloseToNote() const
      {
      return (isStaccato() || isTenuto()) && !isDouble();
      }

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

QVector<QLineF> Articulation::dragAnchorLines() const
      {
      QVector<QLineF> result;
      result << QLineF(canvasPos(), parent()->canvasPos());
      return result;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Articulation::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::SYMBOL:              return QVariant::fromValue(_symId);
            case Pid::DIRECTION:           return QVariant::fromValue<Direction>(direction());
            case Pid::ARTICULATION_ANCHOR: return int(anchor());
            case Pid::ORNAMENT_STYLE:      return int(ornamentStyle());
            case Pid::PLAY:                return bool(playArticulation());
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Articulation::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::SYMBOL:
                  setSymId(v.value<SymId>());
                  break;
            case Pid::DIRECTION:
                  setDirection(v.value<Direction>());
                  break;
            case Pid::ARTICULATION_ANCHOR:
                  setAnchor(ArticulationAnchor(v.toInt()));
                  break;
            case Pid::PLAY:
                  setPlayArticulation(v.toBool());
                  break;
            case Pid::ORNAMENT_STYLE:
                  setOrnamentStyle(MScore::OrnamentStyle(v.toInt()));
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

QVariant Articulation::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::DIRECTION:
                  return QVariant::fromValue<Direction>(Direction::AUTO);

            case Pid::ORNAMENT_STYLE:
                  //return int(score()->style()->ornamentStyle(_ornamentStyle));
                  return int(MScore::OrnamentStyle::DEFAULT);

            case Pid::PLAY:
                  return true;

            default:
                  break;
            }
      return Element::propertyDefault(propertyId);
      }

//---------------------------------------------------------
//   anchorGroup
//---------------------------------------------------------

Articulation::AnchorGroup Articulation::anchorGroup(SymId symId)
      {
      switch (symId) {
            case SymId::articAccentAbove:
            case SymId::articAccentBelow:
            case SymId::articStaccatoAbove:
            case SymId::articStaccatoBelow:
            case SymId::articStaccatissimoAbove:
            case SymId::articStaccatissimoBelow:
            case SymId::articTenutoAbove:
            case SymId::articTenutoBelow:
            case SymId::articTenutoStaccatoAbove:
            case SymId::articTenutoStaccatoBelow:
            case SymId::articMarcatoAbove:
            case SymId::articMarcatoBelow:

            case SymId::articAccentStaccatoAbove:
            case SymId::articAccentStaccatoBelow:
            case SymId::articLaissezVibrerAbove:
            case SymId::articLaissezVibrerBelow:
            case SymId::articMarcatoStaccatoAbove:
            case SymId::articMarcatoStaccatoBelow:
            case SymId::articMarcatoTenutoAbove:
            case SymId::articMarcatoTenutoBelow:
            case SymId::articStaccatissimoStrokeAbove:
            case SymId::articStaccatissimoStrokeBelow:
            case SymId::articStaccatissimoWedgeAbove:
            case SymId::articStaccatissimoWedgeBelow:
            case SymId::articStressAbove:
            case SymId::articStressBelow:
            case SymId::articTenutoAccentAbove:
            case SymId::articTenutoAccentBelow:
            case SymId::articUnstressAbove:
            case SymId::articUnstressBelow:

            case SymId::articSoftAccentAbove:
            case SymId::articSoftAccentBelow:
            case SymId::articSoftAccentStaccatoAbove:
            case SymId::articSoftAccentStaccatoBelow:
            case SymId::articSoftAccentTenutoAbove:
            case SymId::articSoftAccentTenutoBelow:
            case SymId::articSoftAccentTenutoStaccatoAbove:
            case SymId::articSoftAccentTenutoStaccatoBelow:

            case SymId::guitarFadeIn:
            case SymId::guitarFadeOut:
            case SymId::guitarVolumeSwell:
            case SymId::wiggleSawtooth:
            case SymId::wiggleSawtoothWide:
            case SymId::wiggleVibratoLargeFaster:
            case SymId::wiggleVibratoLargeSlowest:
                  return AnchorGroup::ARTICULATION;

            case SymId::luteFingeringRHThumb:
            case SymId::luteFingeringRHFirst:
            case SymId::luteFingeringRHSecond:
            case SymId::luteFingeringRHThird:
                  return AnchorGroup::LUTE_FINGERING;

            case SymId::tremoloDivisiDots2:
            case SymId::tremoloDivisiDots3:
            case SymId::tremoloDivisiDots4:
            case SymId::tremoloDivisiDots6:
                  return AnchorGroup::ARTICULATION;

            default:
                  break;
            }
      return AnchorGroup::OTHER;
      }

//---------------------------------------------------------
//   symId2ArticulationName
//---------------------------------------------------------

const char* Articulation::symId2ArticulationName(SymId symId)
      {
      switch (symId) {
            case SymId::articStaccatissimoAbove:
            case SymId::articStaccatissimoBelow:
            case SymId::articStaccatissimoStrokeAbove:
            case SymId::articStaccatissimoStrokeBelow:
            case SymId::articStaccatissimoWedgeAbove:
            case SymId::articStaccatissimoWedgeBelow:
                  return "staccatissimo";

            case SymId::articStaccatoAbove:
            case SymId::articStaccatoBelow:
            case SymId::tremoloDivisiDots2:
            case SymId::tremoloDivisiDots3:
            case SymId::tremoloDivisiDots4:
            case SymId::tremoloDivisiDots6:
                  return "staccato";

            case SymId::articAccentStaccatoAbove:
            case SymId::articAccentStaccatoBelow:
                  return "sforzatoStaccato";

            case SymId::articMarcatoStaccatoAbove:
            case SymId::articMarcatoStaccatoBelow:
                  return "marcatoStaccato";

            case SymId::articTenutoStaccatoAbove:
            case SymId::articTenutoStaccatoBelow:
                  return "portato";

            case SymId::articMarcatoTenutoAbove:
            case SymId::articMarcatoTenutoBelow:
                  return "marcatoTenuto";

            case SymId::articTenutoAbove:
            case SymId::articTenutoBelow:
                  return "tenuto";

            case SymId::articMarcatoAbove:
            case SymId::articMarcatoBelow:
                  return "marcato";

            case SymId::articAccentAbove:
            case SymId::articAccentBelow:
                  return "sforzato";

            case SymId::brassMuteOpen:
                  return "open";

            case SymId::brassMuteClosed:
                  return "closed";

            case SymId::stringsHarmonic:
                  return "harmonic";

            case SymId::ornamentMordent:
                  return "mordent";

            default:
                  return "---";
            }
      }

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid Articulation::propertyId(const QStringRef& xmlName) const
      {
      if (xmlName == "subtype")
            return Pid::SYMBOL;
      return Element::propertyId(xmlName);
      }

//---------------------------------------------------------
//   articulationName
//---------------------------------------------------------

const char* Articulation::articulationName() const
      {
      return symId2ArticulationName(_symId);
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid Articulation::getPropertyStyle(Pid id) const
      {
      switch (id) {
            case Pid::MIN_DISTANCE:
                  return Element::getPropertyStyle(id);

            case Pid::ARTICULATION_ANCHOR: {
                  switch (anchorGroup(_symId)) {
                        case AnchorGroup::ARTICULATION:
                              return Sid::articulationAnchorDefault;
                        case AnchorGroup::LUTE_FINGERING:
                              return Sid::articulationAnchorLuteFingering;
                        case AnchorGroup::OTHER:
                              return Sid::articulationAnchorOther;
                        }
                  }
                  Q_ASSERT(false); // should never be reached
                  Q_FALLTHROUGH();
            default:
                  return Sid::NOSTYLE;
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Articulation::resetProperty(Pid id)
      {
      switch (id) {
            case Pid::DIRECTION:
            case Pid::ORNAMENT_STYLE:
                  setProperty(id, propertyDefault(id));
                  return;
            case Pid::ARTICULATION_ANCHOR:
                  setProperty(id, propertyDefault(id));
                  return;

            default:
                  break;
            }
      Element::resetProperty(id);
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Articulation::mag() const
      {
      return parent() ? parent()->mag() * score()->styleD(Sid::articulationMag) : 1.0;
      }

bool Articulation::isTenuto() const
      {
      return _symId == SymId::articTenutoAbove   || _symId == SymId::articTenutoBelow;
      }

bool Articulation::isStaccato() const
      {
      return _symId == SymId::articStaccatoAbove        || _symId == SymId::articStaccatoBelow
          || _symId == SymId::articMarcatoStaccatoAbove || _symId == SymId::articMarcatoStaccatoBelow
          || _symId == SymId::articAccentStaccatoAbove || _symId == SymId::articAccentStaccatoBelow
          || _symId == SymId::tremoloDivisiDots2 || _symId == SymId::tremoloDivisiDots3
          || _symId == SymId::tremoloDivisiDots4 || _symId == SymId::tremoloDivisiDots6;
      }

bool Articulation::isAccent() const
      {
      return _symId == SymId::articAccentAbove          || _symId == SymId::articAccentBelow
          || _symId == SymId::articAccentStaccatoAbove  || _symId == SymId::articAccentStaccatoBelow;
      }

bool Articulation::isMarcato() const
      {
      return _symId == SymId::articMarcatoAbove         || _symId == SymId::articMarcatoBelow
          || _symId == SymId::articMarcatoStaccatoAbove || _symId == SymId::articMarcatoStaccatoBelow
          || _symId == SymId::articMarcatoTenutoAbove   || _symId == SymId::articMarcatoTenutoBelow;
      }

bool Articulation::isDouble() const {
      return _symId == SymId::articMarcatoStaccatoAbove || _symId == SymId::articMarcatoStaccatoBelow
          || _symId == SymId::articAccentStaccatoAbove  || _symId == SymId::articAccentStaccatoBelow
          || _symId == SymId::articMarcatoTenutoAbove   || _symId == SymId::articMarcatoTenutoBelow;
      }

//---------------------------------------------------------
//   isLuteFingering
//---------------------------------------------------------

bool Articulation::isLuteFingering() const
      {
      return _symId == SymId::stringsThumbPosition
          || _symId == SymId::luteFingeringRHThumb
          || _symId == SymId::luteFingeringRHFirst
          || _symId == SymId::luteFingeringRHSecond
          || _symId == SymId::luteFingeringRHThird;
      }

//---------------------------------------------------------
//   isOrnament
//---------------------------------------------------------

bool Articulation::isOrnament() const
      {
      return _symId == SymId::ornamentTurn
          || _symId == SymId::ornamentTurnInverted
          || _symId == SymId::ornamentTurnSlash
          || _symId == SymId::ornamentTrill
          || _symId == SymId::brassMuteClosed
          || _symId == SymId::ornamentMordent
          || _symId == SymId::ornamentShortTrill
          || _symId == SymId::ornamentTremblement
          || _symId == SymId::ornamentPrallMordent
          || _symId == SymId::ornamentLinePrall
          || _symId == SymId::ornamentUpPrall
          || _symId == SymId::ornamentUpMordent
          || _symId == SymId::ornamentPrecompMordentUpperPrefix
          || _symId == SymId::ornamentDownMordent
          || _symId == SymId::ornamentPrallUp
          || _symId == SymId::ornamentPrallDown
          || _symId == SymId::ornamentPrecompSlide
          || _symId == SymId::ornamentShake3
          || _symId == SymId::ornamentShakeMuffat1
          || _symId == SymId::ornamentTremblementCouperin
          || _symId == SymId::ornamentPinceCouperin;
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Articulation::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo(), userName());
      }

//---------------------------------------------------------
//   doAutoplace
//    check for collisions
//---------------------------------------------------------

void Articulation::doAutoplace()
      {
      // rebase vertical offset on drag
      qreal rebase = 0.0;
      if (offsetChanged() != OffsetChange::NONE)
            rebase = rebaseOffset();

      if (autoplace() && parent()) {
            Segment* s = segment();
            Measure* m = measure();
            int si     = vStaffIdx();

            qreal sp = score()->spatium();
            qreal md = minDistance().val() * sp;

            SysStaff* ss = m->system()->staff(si);
            QRectF r = bbox().translated(chordRest()->pos() + m->pos() + s->pos() + pos());

            qreal d;
            bool above = up(); // (anchor() == ArticulationAnchor::TOP_STAFF || anchor() == ArticulationAnchor::TOP_CHORD);
            SkylineLine sk(!above);
            if (above) {
                  sk.add(r.x(), r.bottom(), r.width());
                  d = sk.minDistance(ss->skyline().north());
                  }
            else {
                  sk.add(r.x(), r.top(), r.width());
                  d = ss->skyline().south().minDistance(sk);
                  }

            if (d > -md) {
                  qreal yd = d + md;
                  if (above)
                        yd *= -1.0;
                  if (offsetChanged() != OffsetChange::NONE) {
                        // user moved element within the skyline
                        // we may need to adjust minDistance, yd, and/or offset
                        //bool inStaff = placeAbove() ? r.bottom() + rebase > 0.0 : r.top() + rebase < staff()->height();
                        if (rebaseMinDistance(md, yd, sp, rebase, above, true))
                              r.translate(0.0, rebase);
                        }
                  rypos() += yd;
                  r.translate(QPointF(0.0, yd));
                  }
            }
      setOffsetChanged(false);
      }

}
