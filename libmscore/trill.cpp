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

#include "trill.h"
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


//---------------------------------------------------------
//   trillTable
//    must be in sync with Trill::Type
//---------------------------------------------------------

const TrillTableItem trillTable[] = {
      { Trill::Type::TRILL_LINE,      "trill",      QT_TRANSLATE_NOOP("trillType", "Trill line")          },
      { Trill::Type::UPPRALL_LINE,    "upprall",    QT_TRANSLATE_NOOP("trillType", "Upprall line")        },
      { Trill::Type::DOWNPRALL_LINE,  "downprall",  QT_TRANSLATE_NOOP("trillType", "Downprall line")      },
      { Trill::Type::PRALLPRALL_LINE, "prallprall", QT_TRANSLATE_NOOP("trillType", "Prallprall line")     }
      };

int trillTableSize() {
      return sizeof(trillTable)/sizeof(TrillTableItem);
      }

//---------------------------------------------------------
//   trillStyle
//---------------------------------------------------------

static const ElementStyle trillStyle {
      { Sid::trillPlacement, Pid::PLACEMENT },
      };

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TrillSegment::draw(QPainter* painter) const
      {
      painter->setPen(spanner()->curColor());
      drawSymbols(_symbols, painter);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void TrillSegment::add(Element* e)
      {
      e->setParent(this);
      if (e->type() == ElementType::ACCIDENTAL) {
            // accidental is part of trill
            trill()->setAccidental(toAccidental(e));
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TrillSegment::remove(Element* e)
      {
      if (trill()->accidental() == e) {
            // accidental is part of trill
            trill()->setAccidental(0);
            }
      }

//---------------------------------------------------------
//   computeVOffset
//---------------------------------------------------------

qreal TrillSegment::computeVOffset(SymId reference, SymId symbol, qreal mag)
      {
      // Compute vertical offset for centering symbol on reference
      ScoreFont* f = score()->scoreFont();
      qreal diff   = f->bbox(reference, mag).height() - f->bbox(symbol, mag).height();
      qreal offset = f->bbox(reference, mag).bottom() - f->bbox(symbol, mag).bottom() - diff / 2;
      return offset;
      }

//---------------------------------------------------------
//   symbolOffsetPair
//---------------------------------------------------------

std::pair<SymId, QPointF> TrillSegment::symbolOffsetPair(SymId id, qreal xOffset, qreal yOffset)
      {
      return std::pair<SymId, QPointF>(id, QPointF(xOffset, yOffset));
      }

//---------------------------------------------------------
//   appendSymbol
//---------------------------------------------------------

void TrillSegment::appendSymbol(SymId id, qreal xOffset, qreal yOffset)
      {
      _symbols.push_back(symbolOffsetPair(id, xOffset, yOffset));
      }

//---------------------------------------------------------
//   fillSymbols
//---------------------------------------------------------

void TrillSegment::fillSymbols(SymId fill, qreal x1, qreal x2, qreal mag, qreal startXOffset)
      {
      qreal w = x2 - x1;
      ScoreFont* f = score()->scoreFont();

      int n = lrint((w - startXOffset) / f->advance(fill, mag));

      // Shift first fill symbol
      if (n > 0)
            appendSymbol(fill, startXOffset);

      for (int i = 1; i < n; ++i)
            appendSymbol(fill);
      }

//---------------------------------------------------------
//   symbolLine
//---------------------------------------------------------

void TrillSegment::symbolLine(SymId fill)
      {
      qreal mag = magS();
      ScoreFont* f = score()->scoreFont();

      _symbols.clear();
      fillSymbols(fill, 0, pos2().x(), mag);
      QRectF r(f->bbox(symbols(), mag));
      setbbox(r);
      }

void TrillSegment::symbolLine(SymId start, SymId fill, std::unique_ptr<std::pair<SymId, SymId>> enclosure)
      {
      qreal mag = magS();
      ScoreFont* f = score()->scoreFont();
      qreal SPACE_AFTER_SYMBOL   = spatium() * (0.0 + score()->styleS(Sid::trillSpaceAfterSymbol).val());
      qreal SPACE_AFTER_CONTINUE = spatium() * (0.4 + score()->styleS(Sid::trillSpaceAfterContinue).val());

      _symbols.clear();
      if (enclosure != nullptr) {
            appendSymbol(enclosure.get()->first, 0, computeVOffset(start, enclosure.get()->first, mag));
            appendSymbol(start, spatium() * score()->styleS(Sid::trillSpaceAfterOpen).val());
            appendSymbol(enclosure.get()->second, spatium() * score()->styleS(Sid::trillSpaceBeforeClose).val(), computeVOffset(start, enclosure.get()->second, mag));
            }
      else {
            appendSymbol(start);
            }
      fillSymbols(fill,
                  f->width(symbols(), mag),
                  pos2().x(),
                  mag,
                  // SPACE_AFTER_SYMBOL/SPACE_AFTER_CONTINUE only applies to regular trill lines, not e.g. pralls
                  (trill()->trillType() != Trill::Type::TRILL_LINE? 0 : (enclosure != nullptr ? SPACE_AFTER_CONTINUE : SPACE_AFTER_SYMBOL)));
      QRectF r(f->bbox(symbols(), mag));
      setbbox(r);
      }

void TrillSegment::symbolLine(SymId start, SymId fill, SymId end)
      {
      qreal mag = magS();
      ScoreFont* f = score()->scoreFont();
      qreal SPACE_AFTER_SYMBOL = spatium() * (0.0 + score()->styleS(Sid::trillSpaceAfterSymbol).val());

      _symbols.clear();
      appendSymbol(start);
      fillSymbols(fill,
                  f->width(symbols(), mag),
                  pos2().x() - f->advance(end, mag),
                  mag,
                  // SPACE_AFTER_SYMBOL/SPACE_AFTER_CONTINUE only applies to regular trill lines, not e.g. pralls
                  (trill()->trillType() != Trill::Type::TRILL_LINE? 0 : SPACE_AFTER_SYMBOL));
      appendSymbol(end);
      QRectF r(f->bbox(symbols(), mag));
      setbbox(r);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TrillSegment::layout()
      {
      if (staff())
            setMag(staff()->mag(tick()));
      if (isSingleType() || isBeginType()) {
            Accidental* a = trill()->accidental();
            if (a) {
                  a->layout();
                  a->setMag(a->mag() * .6);
                  qreal _spatium = spatium();
                  a->setPos(_spatium * 1.3, -2.2 * _spatium);
                  a->setParent(this);
                  }
            switch (trill()->trillType()) {
                  case Trill::Type::TRILL_LINE:
                        symbolLine(SymId::ornamentTrill, SymId::wiggleTrill);
                        break;
                  case Trill::Type::PRALLPRALL_LINE:
                        symbolLine(SymId::wiggleTrill);
                        break;
                  case Trill::Type::UPPRALL_LINE:
                              symbolLine(SymId::ornamentBottomLeftConcaveStroke,
                                 SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
                        break;
                  case Trill::Type::DOWNPRALL_LINE:
                              symbolLine(SymId::ornamentLeftVerticalStroke,
                                 SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
                        break;
                  }
            }
      else if (isMiddleType() || isEndType()) {
            switch (trill()->trillType()) {
                  case Trill::Type::TRILL_LINE: {
                        int continueSymbol = score()->styleI(Sid::trillContinue);
                        if (continueSymbol == int(TrillContinue::PARENTHESES)) {
                              symbolLine(SymId::ornamentTrill,
                                         SymId::wiggleTrill,
                                         std::unique_ptr<std::pair<SymId, SymId>>(new std::pair<SymId, SymId>(SymId::accidentalParensLeft,
                                                                                                              SymId::accidentalParensRight)));
                        }
                        else if (continueSymbol == int(TrillContinue::BRACKETS)) {
                              symbolLine(SymId::ornamentTrill,
                                         SymId::wiggleTrill,
                                         std::unique_ptr<std::pair<SymId, SymId>>(new std::pair<SymId, SymId>(SymId::accidentalBracketLeft,
                                                                                                              SymId::accidentalBracketRight)));
                        }
                        else if (continueSymbol == int(TrillContinue::SYMBOL)) {
                              symbolLine(SymId::ornamentTrill, SymId::wiggleTrill);
                              }
                        else {
                              symbolLine(SymId::wiggleTrill);
                              }
                        break;
                        }
                  case Trill::Type::PRALLPRALL_LINE:
                        symbolLine(SymId::wiggleTrill);
                        break;
                  case Trill::Type::UPPRALL_LINE:
                              symbolLine(SymId::ornamentZigZagLineNoRightEnd,
                                 SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
                        break;
                  case Trill::Type::DOWNPRALL_LINE:
                              symbolLine(SymId::ornamentZigZagLineNoRightEnd,
                                 SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
                        break;
                  }
      }
      else
            symbolLine(SymId::wiggleTrill);

      autoplaceSpannerSegment(spatium() * 1.0);
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape TrillSegment::shape() const
      {
      return Shape(bbox());
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool TrillSegment::acceptDrop(EditData& data) const
      {
      if (data.dropElement->isAccidental())
            return true;
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* TrillSegment::drop(EditData& data)
      {
      Element* e = data.dropElement;
      switch (e->type()) {
            case ElementType::ACCIDENTAL:
                  e->setParent(trill());
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
//   propertyDelegate
//---------------------------------------------------------

Element* TrillSegment::propertyDelegate(Pid pid)
      {
      if (pid == Pid::TRILL_TYPE || pid == Pid::ORNAMENT_STYLE || pid == Pid::PLACEMENT || pid == Pid::PLAY)
            return spanner();
      return LineSegment::propertyDelegate(pid);
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void TrillSegment::scanElements(void* data, void (*func)(void*, Element*), bool /*all*/)
      {
      func(data, this);
      if (isSingleType() || isBeginType()) {
            Accidental* a = trill()->accidental();
            if (a)
                  func(data, a);
            }
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid TrillSegment::getPropertyStyle(Pid pid) const
      {
      if (pid == Pid::OFFSET)
            return spanner()->placeAbove() ? Sid::trillPosAbove : Sid::trillPosBelow;
      return LineSegment::getPropertyStyle(pid);
      }

Sid Trill::getPropertyStyle(Pid pid) const
      {
      if (pid == Pid::OFFSET)
            return placeAbove() ? Sid::trillPosAbove : Sid::trillPosBelow;
      return SLine::getPropertyStyle(pid);
      }

//---------------------------------------------------------
//   Trill
//---------------------------------------------------------

Trill::Trill(Score* s)
  : SLine(s)
      {
      _trillType     = Type::TRILL_LINE;
      _accidental    = 0;
      _ornamentStyle = MScore::OrnamentStyle::DEFAULT;
      setPlayArticulation(true);
      initElementStyle(&trillStyle);
      resetProperty(Pid::OFFSET);
      }

Trill::~Trill()
      {
      delete _accidental;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Trill::add(Element* e)
      {
      if (e->type() == ElementType::ACCIDENTAL) {
            e->setParent(this);
            _accidental = toAccidental(e);
            }
      else
            SLine::add(e);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Trill::remove(Element* e)
      {
      if (e == _accidental)
            _accidental = 0;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Trill::layout()
      {
      SLine::layout();
      if (score() == gscore)
            return;
      if (spannerSegments().empty())
            return;
      TrillSegment* ls = toTrillSegment(frontSegment());
      if (spannerSegments().empty())
            qDebug("Trill: no segments");
      if (_accidental)
            _accidental->setParent(ls);
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

static const ElementStyle trillSegmentStyle {
      { Sid::trillPosAbove, Pid::OFFSET },
      };

LineSegment* Trill::createLineSegment()
      {
      TrillSegment* seg = new TrillSegment(this, score());
      seg->setTrack(track());
      seg->setColor(color());
      seg->initElementStyle(&trillSegmentStyle);
      return seg;
      }

//---------------------------------------------------------
//   Trill::write
//---------------------------------------------------------

void Trill::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this);
      xml.tag("subtype", trillTypeName());
      writeProperty(xml, Pid::PLAY);
      writeProperty(xml, Pid::ORNAMENT_STYLE);
      writeProperty(xml, Pid::PLACEMENT);
      SLine::writeProperties(xml);
      if (_accidental)
            _accidental->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Trill::read
//---------------------------------------------------------

void Trill::read(XmlReader& e)
      {
      eraseSpannerSegments();

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setTrillType(e.readElementText());
            else if (tag == "Accidental") {
                  _accidental = new Accidental(score());
                  _accidental->read(e);
                  _accidental->setParent(this);
                  }
            else if ( tag == "ornamentStyle")
                  readProperty(e, Pid::ORNAMENT_STYLE);
            else if ( tag == "play")
                  setPlayArticulation(e.readBool());
            else if (!SLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   setTrillType
//---------------------------------------------------------

void Trill::setTrillType(const QString& s)
      {
      if (s == "0") {
            _trillType = Type::TRILL_LINE;
            return;
            }
      if (s == "pure") {
            _trillType = Type::PRALLPRALL_LINE; // obsolete, compatibility only
            return;
            }
      for (TrillTableItem i : trillTable) {
            if (s.compare(i.name) == 0) {
                  _trillType = i.type;
                  return;
                  }
            }
      qDebug("Trill::setSubtype: unknown <%s>", qPrintable(s));
      }

//---------------------------------------------------------
//   trillTypeName
//---------------------------------------------------------

QString Trill::trillTypeName() const
      {
      for (TrillTableItem i : trillTable) {
            if (i.type == trillType())
                  return i.name;
            }
      qDebug("unknown Trill subtype %d", int(trillType()));
            return "?";
      }

//---------------------------------------------------------
//   trillTypeName
//---------------------------------------------------------

QString Trill::trillTypeUserName() const
      {
      return qApp->translate("trillType", trillTable[static_cast<int>(trillType())].userName.toUtf8().constData());
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Trill::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      if (_accidental)
            _accidental->scanElements(data, func, all);
      func(data, this);       // ?
      SLine::scanElements(data, func, all);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Trill::getProperty(Pid propertyId) const
      {
      switch(propertyId) {
            case Pid::TRILL_TYPE:
                  return int(trillType());
            case Pid::ORNAMENT_STYLE:
                  return int(ornamentStyle());
            case Pid::PLAY:
                  return bool(playArticulation());
            default:
                  break;
            }
      return SLine::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Trill::setProperty(Pid propertyId, const QVariant& val)
      {
      switch(propertyId) {
            case Pid::TRILL_TYPE:
                  setTrillType(Type(val.toInt()));
                  break;
            case Pid::PLAY:
                  setPlayArticulation(val.toBool());
                  break;
            case Pid::ORNAMENT_STYLE:
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

QVariant Trill::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::TRILL_TYPE:
                  return 0;
            case Pid::ORNAMENT_STYLE:
                  //return int(score()->style()->ornamentStyle(_ornamentStyle));
                  return int(MScore::OrnamentStyle::DEFAULT);
            case Pid::PLAY:
                  return true;
            case Pid::PLACEMENT:
                  return score()->styleV(Sid::trillPlacement);

            default:
                  return SLine::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Trill::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(trillTypeUserName());
      }
}

