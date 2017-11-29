//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "vibrato.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "sym.h"
#include "score.h"
#include "accidental.h"
#include "segment.h"
#include "staff.h"

namespace Ms {


// must be in sync with Vibrato::Type
const VibratoTableItem vibratoTable[] = {
      { Vibrato::Type::TRILL_LINE,      "trill",      QT_TRANSLATE_NOOP("trillType", "Vibrato line")          },
      { Vibrato::Type::UPPRALL_LINE,    "upprall",    QT_TRANSLATE_NOOP("trillType", "Upprall line")        },
      { Vibrato::Type::DOWNPRALL_LINE,  "downprall",  QT_TRANSLATE_NOOP("trillType", "Downprall line")      },
      { Vibrato::Type::PRALLPRALL_LINE, "prallprall", QT_TRANSLATE_NOOP("trillType", "Prallprall line")     },
      { Vibrato::Type::GUITAR_VIBRATO,         "guitarVibrato",         QT_TRANSLATE_NOOP("trillType", "Guitar vibrato")          },
      { Vibrato::Type::GUITAR_VIBRATO_WIDE,    "guitarVibratoWide",     QT_TRANSLATE_NOOP("trillType", "Guitar vibrato wide")     },
      { Vibrato::Type::VIBRATO_SAWTOOTH_NARROW,  "vibratoSawtoothNarrow", QT_TRANSLATE_NOOP("trillType", "Vibrato sawtooth narrow") },
      { Vibrato::Type::VIBRATO_SAWTOOTH,         "vibratoSawtooth",       QT_TRANSLATE_NOOP("trillType", "Vibrato sawtooth")        },
      { Vibrato::Type::VIBRATO_SAWTOOTH_WIDE,    "vibratoSawtoothWide",   QT_TRANSLATE_NOOP("trillType", "tremolo sawtooth wide")   }
      };

int vibratoTableSize() {
      return sizeof(vibratoTable)/sizeof(VibratoTableItem);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void VibratoSegment::draw(QPainter* painter) const
      {
      painter->setPen(spanner()->curColor());
      drawSymbols(_symbols, painter);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void VibratoSegment::add(Element* e)
      {
      e->setParent(this);
      if (e->type() == ElementType::ACCIDENTAL) {
            // accidental is part of trill
            vibrato()->setAccidental(static_cast<Accidental*>(e));
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void VibratoSegment::remove(Element* e)
      {
      if (vibrato()->accidental() == e) {
            // accidental is part of trill
            vibrato()->setAccidental(0);
            }
      }

//---------------------------------------------------------
//   symbolLine
//---------------------------------------------------------

void VibratoSegment::symbolLine(SymId start, SymId fill)
      {
      qreal x1 = 0;
      qreal x2 = pos2().x();
      qreal w   = x2 - x1;
      qreal mag = magS();
      ScoreFont* f = score()->scoreFont();

      _symbols.clear();
      _symbols.push_back(start);
      qreal w1 = f->advance(start, mag);
      qreal w2 = f->advance(fill, mag);
      int n    = lrint((w - w1) / w2);
      for (int i = 0; i < n; ++i)
           _symbols.push_back(fill);
      QRectF r(f->bbox(_symbols, mag));
      setbbox(r);
      }

void VibratoSegment::symbolLine(SymId start, SymId fill, SymId end)
      {
      qreal x1 = 0;
      qreal x2 = pos2().x();
      qreal w   = x2 - x1;
      qreal mag = magS();
      ScoreFont* f = score()->scoreFont();

      _symbols.clear();
      _symbols.push_back(start);
      qreal w1 = f->bbox(start, mag).width();
      qreal w2 = f->width(fill, mag);
      qreal w3 = f->width(end, mag);
      int n    = lrint((w - w1 - w3) / w2);
      for (int i = 0; i < n; ++i)
           _symbols.push_back(fill);
      _symbols.push_back(end);
      QRectF r(f->bbox(_symbols, mag));
      setbbox(r);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void VibratoSegment::layout()
      {
      if (autoplace())
            setUserOff(QPointF());

      if (staff())
            setMag(staff()->mag(tick()));
      if (isSingleType() || isBeginType()) {
            Accidental* a = vibrato()->accidental();
            if (a) {
                  a->layout();
                  a->setMag(a->mag() * .6);
                  qreal _spatium = spatium();
                  a->setPos(_spatium * 1.3, -2.2 * _spatium);
                  a->adjustReadPos();
                  a->setParent(this);
                  }
            switch (vibrato()->vibratoType()) {
                  case Vibrato::Type::TRILL_LINE:
                        symbolLine(SymId::ornamentTrill, SymId::wiggleVibrato);
                        break;
                  case Vibrato::Type::PRALLPRALL_LINE:
                        symbolLine(SymId::wiggleVibrato, SymId::wiggleVibrato);
                        break;
                  case Vibrato::Type::UPPRALL_LINE:
                              symbolLine(SymId::ornamentBottomLeftConcaveStroke,
                                 SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
                        break;
                  case Vibrato::Type::DOWNPRALL_LINE:
                              symbolLine(SymId::ornamentLeftVerticalStroke,
                                 SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
                        break;
                  case Vibrato::Type::GUITAR_VIBRATO:
                        symbolLine(SymId::guitarVibratoStroke, SymId::guitarVibratoStroke);
                        break;
                  case Vibrato::Type::GUITAR_VIBRATO_WIDE:
                        symbolLine(SymId::guitarWideVibratoStroke, SymId::guitarWideVibratoStroke);
                        break;
                  case Vibrato::Type::VIBRATO_SAWTOOTH_NARROW:
                        symbolLine(SymId::wiggleSawtoothNarrow, SymId::wiggleSawtoothNarrow);
                        break;
                  case Vibrato::Type::VIBRATO_SAWTOOTH:
                        symbolLine(SymId::wiggleSawtooth, SymId::wiggleSawtooth);
                        break;
                  case Vibrato::Type::VIBRATO_SAWTOOTH_WIDE:
                        symbolLine(SymId::wiggleSawtoothWide, SymId::wiggleSawtoothWide);
                        break;
                  }
            }
      else
            symbolLine(SymId::wiggleVibrato, SymId::wiggleVibrato);

      if (parent()) {
            qreal yo = score()->styleP(vibrato()->placeBelow() ? StyleIdx::trillPosBelow : StyleIdx::trillPosAbove);
            rypos() = yo;
            if (autoplace()) {
                  qreal minDistance = spatium();
                  Shape s1 = shape().translated(pos());
                  if (vibrato()->placeAbove()) {
                        qreal d  = system()->topDistance(staffIdx(), s1);
                        if (d > -minDistance)
                              rUserYoffset() = -d - minDistance;
                        }
                  else {
                        qreal d  = system()->bottomDistance(staffIdx(), s1);
                        if (d > -minDistance)
                              rUserYoffset() = d + minDistance;
                        }
                  }
            else
                  adjustReadPos();
            }
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape VibratoSegment::shape() const
      {
      return Shape(bbox());
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool VibratoSegment::acceptDrop(EditData& data) const
      {
      if (data.element->isAccidental())
            return true;
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* VibratoSegment::drop(EditData& data)
      {
      Element* e = data.element;
      switch (e->type()) {
            case ElementType::ACCIDENTAL:
                  e->setParent(vibrato());
                  score()->undoAddElement(e);
                  break;

            default:
                  delete e;
                  e = 0;
                  break;
            }
      return e;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant VibratoSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::TRILL_TYPE:
            case P_ID::ORNAMENT_STYLE:
            case P_ID::PLACEMENT:
            case P_ID::PLAY:
                  return vibrato()->getProperty(id);
            default:
                  return LineSegment::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool VibratoSegment::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::TRILL_TYPE:
            case P_ID::ORNAMENT_STYLE:
            case P_ID::PLACEMENT:
            case P_ID::PLAY:
                  return vibrato()->setProperty(id, v);
            default:
                  return LineSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant VibratoSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::TRILL_TYPE:
            case P_ID::ORNAMENT_STYLE:
            case P_ID::PLACEMENT:
            case P_ID::PLAY:
                  return vibrato()->propertyDefault(id);
            default:
                  return LineSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void VibratoSegment::scanElements(void* data, void (*func)(void*, Element*), bool /*all*/)
      {
      func(data, this);
      if (isSingleType() || isBeginType()) {
            Accidental* a = vibrato()->accidental();
            if (a)
                  func(data, a);
            }
      }

//---------------------------------------------------------
//   Vibrato
//---------------------------------------------------------

Vibrato::Vibrato(Score* s)
  : SLine(s)
      {
      _vibratoType = Type::GUITAR_VIBRATO;
      _accidental = 0;
      _ornamentStyle    = MScore::OrnamentStyle::DEFAULT;
      setPlayArticulation(true);
      setPlacement(Element::Placement::ABOVE);
      }

Vibrato::~Vibrato()
      {
      delete _accidental;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Vibrato::add(Element* e)
      {
      if (e->type() == ElementType::ACCIDENTAL) {
            e->setParent(this);
            _accidental = static_cast<Accidental*>(e);
            }
      else
            SLine::add(e);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Vibrato::remove(Element* e)
      {
      if (e == _accidental)
            _accidental = 0;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Vibrato::layout()
      {
      SLine::layout();
      if (score() == gscore)
            return;
      if (spannerSegments().empty())
            return;
      VibratoSegment* ls = static_cast<VibratoSegment*>(frontSegment());
#if 0
// this is now handled differently, in SLine::linePos
      //
      // special case:
      // if end segment is first chord/rest segment in measure,
      // shorten trill line so it ends at end of previous measure
      //
      qreal _spatium = spatium();
      Segment* seg1  = startSegment();
      Segment* seg2  = endSegment();
      if (seg1
         && seg2
         && (seg1->system() == seg2->system())
         && (spannerSegments().size() == 1)
         && (seg2->tick() == seg2->measure()->tick())
         ) {
            qreal x1   = seg2->pagePos().x();
            Measure* m = seg2->measure()->prevMeasure();
            if (m) {
                  Segment* s2      = m->last();
                  qreal x2         = s2->pagePos().x();
                  qreal dx         = x1 - x2 + _spatium * .3;
                  ls->setPos2(ls->ipos2() + QPointF(-dx, 0.0));
                  ls->layout();
                  }
            }
#endif
      if (spannerSegments().empty())
            qDebug("Vibrato: no segments");
      if (_accidental)
            _accidental->setParent(ls);
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Vibrato::createLineSegment()
      {
      VibratoSegment* seg = new VibratoSegment(score());
      seg->setTrack(track());
      seg->setColor(color());
      return seg;
      }

//---------------------------------------------------------
//   Vibrato::write
//---------------------------------------------------------

void Vibrato::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
      xml.tag("subtype", vibratoTypeName());
      writeProperty(xml, P_ID::PLAY);
      writeProperty(xml, P_ID::ORNAMENT_STYLE);
      SLine::writeProperties(xml);
      if (_accidental)
            _accidental->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Vibrato::read
//---------------------------------------------------------

void Vibrato::read(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();

      e.addSpanner(e.intAttribute("id", -1), this);
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setVibratoType(e.readElementText());
            else if (tag == "Accidental") {
                  _accidental = new Accidental(score());
                  _accidental->read(e);
                  _accidental->setParent(this);
                  }
            else if ( tag == "ornamentStyle")
                  setProperty(P_ID::ORNAMENT_STYLE, Ms::getProperty(P_ID::ORNAMENT_STYLE, e));
            else if ( tag == "play")
                  setPlayArticulation(e.readBool());
            else if (!SLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   setVibratoType
//---------------------------------------------------------

void Vibrato::setVibratoType(const QString& s)
      {
      if (s == "0") {
            _vibratoType = Type::TRILL_LINE;
            return;
            }
      if (s == "pure") {
            _vibratoType = Type::PRALLPRALL_LINE; // obsolete, compatibility only
            return;
            }
      for (VibratoTableItem i : vibratoTable) {
            if (s.compare(i.name) == 0) {
                  _vibratoType = i.type;
                  return;
                  }
            }
      qDebug("Vibrato::setSubtype: unknown <%s>", qPrintable(s));
      }

//---------------------------------------------------------
//   vibratoTypeName
//---------------------------------------------------------

QString Vibrato::vibratoTypeName() const
      {
      for (VibratoTableItem i : vibratoTable) {
            if (i.type == vibratoType())
                  return i.name;
            }
      qDebug("unknown Vibrato subtype %d", int(vibratoType()));
            return "?";
      }

//---------------------------------------------------------
//   vibratoTypeName
//---------------------------------------------------------

QString Vibrato::vibratoTypeUserName() const
      {
      return qApp->translate("vibratoType", vibratoTable[static_cast<int>(vibratoType())].userName.toUtf8().constData());
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Vibrato::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      if (_accidental)
            _accidental->scanElements(data, func, all);
      func(data, this);       // ?
      SLine::scanElements(data, func, all);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Vibrato::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::TRILL_TYPE:
                  return int(vibratoType());
            case P_ID::ORNAMENT_STYLE:
                  return int(ornamentStyle());
            case P_ID::PLAY:
                  return bool(playArticulation());
            default:
                  break;
            }
      return SLine::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Vibrato::setProperty(P_ID propertyId, const QVariant& val)
      {
      switch(propertyId) {
            case P_ID::TRILL_TYPE:
                  setVibratoType(Type(val.toInt()));
                  break;
            case P_ID::PLAY:
                  setPlayArticulation(val.toBool());
                  break;
            case P_ID::ORNAMENT_STYLE:
                  setOrnamentStyle(MScore::OrnamentStyle(val.toInt()));
                  break;
            default:
                  if (!SLine::setProperty(propertyId, val))
                        return false;
                  break;
            }
      score()->setLayoutAll();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Vibrato::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::TRILL_TYPE:
                  return 0;
            case P_ID::ORNAMENT_STYLE:
                  //return int(score()->style()->ornamentStyle(_ornamentStyle));
                  return int(MScore::OrnamentStyle::DEFAULT);
            case P_ID::PLAY:
                  return true;
            case P_ID::PLACEMENT:
                  return int(Element::Placement::ABOVE);
            default:
                  return SLine::propertyDefault(propertyId);
            }
      return QVariant();
      }

//---------------------------------------------------------
//   undoSetVibratoType
//---------------------------------------------------------

void Vibrato::undoSetVibratoType(Type val)
      {
      undoChangeProperty(P_ID::TRILL_TYPE, int(val));
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Vibrato::setYoff(qreal val)
      {
      rUserYoffset() += val * spatium() - score()->styleP(StyleIdx::trillPosAbove);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Vibrato::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(vibratoTypeUserName());
      }
}

