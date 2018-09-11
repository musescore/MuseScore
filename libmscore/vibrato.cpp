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


//---------------------------------------------------------
//   vibratoTable
//    must be in sync with Vibrato::Type
//---------------------------------------------------------

const VibratoTableItem vibratoTable[] = {
      { Vibrato::Type::GUITAR_VIBRATO,        "guitarVibrato",       QT_TRANSLATE_NOOP("vibratoType", "Guitar vibrato")        },
      { Vibrato::Type::GUITAR_VIBRATO_WIDE,   "guitarVibratoWide",   QT_TRANSLATE_NOOP("vibratoType", "Guitar vibrato wide")   },
      { Vibrato::Type::VIBRATO_SAWTOOTH,      "vibratoSawtooth",     QT_TRANSLATE_NOOP("vibratoType", "Vibrato sawtooth")      },
      { Vibrato::Type::VIBRATO_SAWTOOTH_WIDE, "vibratoSawtoothWide", QT_TRANSLATE_NOOP("vibratoType", "tremolo sawtooth wide") }
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
      if (staff())
            setMag(staff()->mag(tick()));
      if (isSingleType() || isBeginType()) {
            switch (vibrato()->vibratoType()) {
                  case Vibrato::Type::GUITAR_VIBRATO:
                        symbolLine(SymId::guitarVibratoStroke, SymId::guitarVibratoStroke);
                        break;
                  case Vibrato::Type::GUITAR_VIBRATO_WIDE:
                        symbolLine(SymId::guitarWideVibratoStroke, SymId::guitarWideVibratoStroke);
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

      autoplaceSpannerSegment(spatium() * 1.0, Sid::vibratoPosBelow, Sid::vibratoPosAbove);
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape VibratoSegment::shape() const
      {
      return Shape(bbox());
      }

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

Element* VibratoSegment::propertyDelegate(Pid pid)
      {
      if (pid == Pid::VIBRATO_TYPE || pid == Pid::PLACEMENT || pid == Pid::PLAY)
            return spanner();
      return LineSegment::propertyDelegate(pid);
      }

//---------------------------------------------------------
//   Vibrato
//---------------------------------------------------------

Vibrato::Vibrato(Score* s)
  : SLine(s)
      {
      _vibratoType = Type::GUITAR_VIBRATO;
      setPlayArticulation(true);
      setPlacement(Placement::ABOVE);
      }

Vibrato::~Vibrato()
      {
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Vibrato::layout()
      {
      SLine::layout();
      if (score() == gscore)
            return;
      if (spannerSegments().empty()) {
            qDebug("Vibrato: no segments");
            return;
            }
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
      xml.stag(name());
      xml.tag("subtype", vibratoTypeName());
      writeProperty(xml, Pid::PLAY);
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Vibrato::read
//---------------------------------------------------------

void Vibrato::read(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setVibratoType(e.readElementText());
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
//   getProperty
//---------------------------------------------------------

QVariant Vibrato::getProperty(Pid propertyId) const
      {
      switch(propertyId) {
            case Pid::VIBRATO_TYPE:
                  return int(vibratoType());
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

bool Vibrato::setProperty(Pid propertyId, const QVariant& val)
      {
      switch(propertyId) {
            case Pid::VIBRATO_TYPE:
                  setVibratoType(Type(val.toInt()));
                  break;
            case Pid::PLAY:
                  setPlayArticulation(val.toBool());
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

QVariant Vibrato::propertyDefault(Pid propertyId) const
      {
      switch(propertyId) {
            case Pid::VIBRATO_TYPE:
                  return 0;
            case Pid::PLAY:
                  return true;
            case Pid::PLACEMENT:
                  return int(Placement::ABOVE);
            default:
                  return SLine::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   undoSetVibratoType
//---------------------------------------------------------

void Vibrato::undoSetVibratoType(Type val)
      {
      undoChangeProperty(Pid::VIBRATO_TYPE, int(val));
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Vibrato::setYoff(qreal val)
      {
      rUserYoffset() += val * spatium() - score()->styleP(Sid::vibratoPosAbove);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Vibrato::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(vibratoTypeUserName());
      }
}

