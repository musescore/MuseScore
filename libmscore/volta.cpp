//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: volta.cpp 5610 2012-05-08 09:56:00Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "volta.h"
#include "style.h"
#include "xml.h"
#include "score.h"
#include "text.h"

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void VoltaSegment::layout()
      {
      TextLineSegment::layout1();
      if (parent())     // for palette
            rypos() += score()->styleS(ST_voltaY).val() * spatium();
      adjustReadPos();
      }

//---------------------------------------------------------
//   Volta
//---------------------------------------------------------

Volta::Volta(Score* s)
   : TextLine(s)
      {
      _subtype = VOLTA_OPEN;
      setBeginText("1.", s->textStyle(TEXT_STYLE_VOLTA));

      setBeginTextPlace(PLACE_BELOW);
      setContinueTextPlace(PLACE_BELOW);

      setBeginHook(true);
      Spatium hook(s->styleS(ST_voltaHook));
      setBeginHookHeight(hook);
      setEndHookHeight(hook);
      setAnchor(ANCHOR_MEASURE);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Volta::setSubtype(VoltaType val)
      {
      _subtype = val;
      switch(val) {
            case VOLTA_OPEN:
                  setEndHook(false);
                  break;
            case VOLTA_CLOSED:
                  setEndHook(true);
                  break;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Volta::layout()
      {
      setLineWidth(score()->styleS(ST_voltaLineWidth));
      Spatium hook(score()->styleS(ST_voltaHook));
      setBeginHookHeight(hook);
      setEndHookHeight(hook);
      TextLine::layout();
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void Volta::setText(const QString& s)
      {
      setBeginText(s, score()->textStyle(TEXT_STYLE_VOLTA));
      foreach(SpannerSegment* seg, spannerSegments())
            static_cast<VoltaSegment*>(seg)->clearText();
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString Volta::text() const
      {
      return beginText()->getText();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Volta::read(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();

      setId(e.intAttribute("id", -1));
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setSubtype(VoltaType(e.readInt()));
            else if (tag == "text")            // obsolete
                  setText(e.readElementText());
            else if (tag == "endings") {
                  QString s = e.readElementText();
                  QStringList sl = s.split(",", QString::SkipEmptyParts);
                  _endings.clear();
                  foreach(const QString& l, sl) {
                        int i = l.simplified().toInt();
                        _endings.append(i);
                        }
                  }
            else if (!TextLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Volta::write(Xml& xml) const
      {
      Volta proto(score());
      proto.setSubtype(subtype());

      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
      xml.tag("subtype", _subtype);
      TextLine::writeProperties(xml, &proto);
      QString s;
      foreach(int i, _endings) {
            if (!s.isEmpty())
                  s += ", ";
            s += QString("%1").arg(i);
            }
      xml.tag("endings", s);
      xml.etag();
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Volta::createLineSegment()
      {
      return new VoltaSegment(score());
      }

//---------------------------------------------------------
//   hasEnding
//---------------------------------------------------------

bool Volta::hasEnding(int repeat) const
      {
      foreach(int ending, endings()) {
            if (ending == repeat)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Volta::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_VOLTA_TYPE:
                  return subtype();
            default:
                  break;
            }
      return TextLine::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Volta::setProperty(P_ID propertyId, const QVariant& val)
      {
      score()->addRefresh(pageBoundingRect());
      switch(propertyId) {
            case P_VOLTA_TYPE:
                  setSubtype(VoltaType(val.toInt()));
                  break;
            default:
                  if (!TextLine::setProperty(propertyId, val))
                        return false;
                  break;
            }
      layout();
      score()->addRefresh(pageBoundingRect());
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Volta::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_VOLTA_TYPE:
                  return 0;
            default:
                  return TextLine::propertyDefault(propertyId);
            }
      return QVariant();
      }

//---------------------------------------------------------
//   undoSetSubtype
//---------------------------------------------------------

void Volta::undoSetSubtype(VoltaType val)
      {
      score()->undoChangeProperty(this, P_VOLTA_TYPE, val);
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Volta::setYoff(qreal val)
      {
      rUserYoffset() += (val - score()->styleS(ST_voltaY).val()) * spatium();
      }

