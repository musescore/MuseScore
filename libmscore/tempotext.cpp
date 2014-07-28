//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "tempotext.h"
#include "tempo.h"
#include "system.h"
#include "measure.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

TempoText::TempoText(Score* s)
   : Text(s)
      {
      _tempo      = 2.0;      // propertyDefault(P_TEMPO).toDouble();
      _followText = false;
      setPlacement(Element::Placement::ABOVE);
      setTextStyleType(TextStyleType::TEMPO);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TempoText::write(Xml& xml) const
      {
      xml.stag("Tempo");
      xml.tag("tempo", _tempo);
      if (_followText)
            xml.tag("followText", _followText);
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TempoText::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "tempo")
                  _tempo = e.readDouble();
            else if (tag == "followText")
                  _followText = e.readInt();
            else if (!Text::readProperties(e))
                  e.unknown();
            }
      if (score()->mscVersion() < 119) {
            //
            // Reset text in old version to
            // style.
            //
//TODO            if (textStyle() != TextStyleType::INVALID) {
//                  setStyled(true);
//                  styleChanged();
//                  }
            }
      }

//---------------------------------------------------------
//   TempoPattern
//---------------------------------------------------------

struct TempoPattern {
      const char* pattern;
      qreal f;
      TDuration d;
      TempoPattern(const char* s, qreal v, TDuration::DurationType val, int dots = 0) : pattern(s), f(v), d(val) { d.setDots(dots); }
      };

// note: findTempoDuration requires the longer patterns to be before the shorter patterns in tp

static const TempoPattern tp[] = {
      TempoPattern("<sym>noteHalfUp</sym>\\s*<sym>textAugmentationDot</sym>",    1.5/30.0,  TDuration::DurationType::V_HALF, 1),    // dotted 1/2
      TempoPattern("<sym>noteHalfUp</sym><sym>space</sym><sym>textAugmentationDot</sym>",    1.5/30.0,  TDuration::DurationType::V_HALF, 1),    // dotted 1/2
      TempoPattern("<sym>noteQuarterUp</sym>\\s*<sym>textAugmentationDot</sym>", 1.5/60.0,  TDuration::DurationType::V_QUARTER, 1), // dotted 1/4
      TempoPattern("<sym>noteQuarterUp</sym><sym>space</sym><sym>textAugmentationDot</sym>", 1.5/60.0,  TDuration::DurationType::V_QUARTER, 1), // dotted 1/4
      TempoPattern("<sym>note8thUp</sym>\\s*<sym>textAugmentationDot</sym>",     1.5/120.0, TDuration::DurationType::V_EIGHT, 1),   // dotted 1/8
      TempoPattern("<sym>note8thUp</sym><sym>space</sym><sym>textAugmentationDot</sym>",     1.5/120.0, TDuration::DurationType::V_EIGHT, 1),   // dotted 1/8
      TempoPattern("<sym>noteHalfUp</sym>",                                      1.0/30.0,  TDuration::DurationType::V_HALF),       // 1/2
      TempoPattern("<sym>noteQuarterUp</sym>",                                   1.0/60.0,  TDuration::DurationType::V_QUARTER),    // 1/4
      TempoPattern("<sym>note8thUp</sym>",                                       1.0/120.0, TDuration::DurationType::V_EIGHT),      // 1/8
      };
      
//---------------------------------------------------------
//   findTempoDuration
//    find the duration part (note + dot) of a tempo text in string s
//    return the match position or -1 if not found
//    set len to the match length and dur to the duration value
//---------------------------------------------------------

int TempoText::findTempoDuration(const QString& s, int& len, TDuration& dur)
      {
      len = 0;
      dur = TDuration();

      for (unsigned i = 0; i < sizeof(tp)/sizeof(*tp); ++i) {
            QRegExp re(tp[i].pattern);
            int pos = re.indexIn(s);
            if (pos != -1) {
                  len = re.matchedLength();
                  dur = tp[i].d;
                  return pos;
                  }
            }

      return -1;
      }
      
//---------------------------------------------------------
//   duration2tempoTextString
//    find the tempoText string representation for duration
//---------------------------------------------------------

QString TempoText::duration2tempoTextString(const TDuration dur)
      {
      for (unsigned i = 0; i < sizeof(tp)/sizeof(*tp); ++i) {
            if (tp[i].d == dur) {
                  QString res = tp[i].pattern;
                  res.remove("\\s*");
                  return res;
                  }
            }
      return "";
      }

//---------------------------------------------------------
//   textChanged
//    text may have changed
//---------------------------------------------------------

void TempoText::textChanged()
      {
      if (!_followText)
            return;
      QString s = text();

      for (unsigned i = 0; i < sizeof(tp)/sizeof(*tp); ++i) {
            QRegExp re(QString(tp[i].pattern)+"\\s*=\\s*(\\d+)");      // 1/4
            if (re.indexIn(s) != -1) {
                  QStringList sl = re.capturedTexts();
                  if (sl.size() == 2) {
                        qreal nt = qreal(sl[1].toInt()) * tp[i].f;
                        if (nt != _tempo) {
                              _tempo = qreal(sl[1].toInt()) * tp[i].f;
                              if(segment())
                                    score()->setTempo(segment(), _tempo);
                              score()->setPlaylistDirty(true);
                              }
                        break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   undoSetTempo
//---------------------------------------------------------

void TempoText::undoSetTempo(qreal v)
      {
      score()->undoChangeProperty(this, P_ID::TEMPO, v);
      }

//---------------------------------------------------------
//   undoSetFollowText
//---------------------------------------------------------

void TempoText::undoSetFollowText(bool v)
      {
      score()->undoChangeProperty(this, P_ID::TEMPO_FOLLOW_TEXT, v);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TempoText::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::TEMPO:             return _tempo;
            case P_ID::TEMPO_FOLLOW_TEXT: return _followText;
            default:
                  return Text::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TempoText::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_ID::TEMPO:
                  _tempo = v.toDouble();
                  score()->setTempo(segment(), _tempo);
                  break;
            case P_ID::TEMPO_FOLLOW_TEXT:
                  _followText = v.toBool();
                  break;
            default:
                  if (!Text::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TempoText::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_ID::TEMPO:             return 120;
            case P_ID::TEMPO_FOLLOW_TEXT: return false;
            case P_ID::PLACEMENT:         return int(Element::Placement::ABOVE);
            default:                  return Text::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TempoText::layout()
      {
      Text::layout();
      if (placement() == Element::Placement::BELOW) {
            rypos() = -rypos() + 4 * spatium();
            // rUserYoffset() *= -1;
            // text height ?
            }
      }

}

