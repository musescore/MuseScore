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
#include "staff.h"
#include "xml.h"

namespace Ms {

#define MIN_TEMPO 5.0/60
#define MAX_TEMPO 999.0/60

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

TempoText::TempoText(Score* s)
   : TextBase(s)
      {
      init(SubStyle::TEMPO);
      _tempo      = 2.0;      // propertyDefault(P_TEMPO).toDouble();
      _followText = false;
      _relative   = 1.0;
      _isRelative = false;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TempoText::write(XmlWriter& xml) const
      {
      xml.stag("Tempo");
      xml.tag("tempo", _tempo);
      if (_followText)
            xml.tag("followText", _followText);
      TextBase::writeProperties(xml);
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
                  setTempo(e.readDouble());
            else if (tag == "followText")
                  _followText = e.readInt();
            else if (!TextBase::readProperties(e))
                  e.unknown();
            }
      // check sanity
      if (xmlText().isEmpty()) {
            setXmlText(QString("<sym>metNoteQuarterUp</sym> = %1").arg(lrint(60 * _tempo)));
            setVisible(false);
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
      TempoPattern("\uECA5\\s*\uECB7\\s*\uECB7", 1.75/60.0,  TDuration::DurationType::V_QUARTER, 2), // double dotted 1/4
      TempoPattern("\uECA5\\s*\uECB7",           1.5/60.0,   TDuration::DurationType::V_QUARTER, 1), // dotted 1/4
      TempoPattern("\uECA5",                     1.0/60.0,   TDuration::DurationType::V_QUARTER),    // 1/4
      TempoPattern("\uECA3\\s*\uECB7\\s*\uECB7", 1.75/30.0,  TDuration::DurationType::V_HALF, 2),    // double dotted 1/2
      TempoPattern("\uECA3\\s*\uECB7",           1.5/30.0,   TDuration::DurationType::V_HALF, 1),    // dotted 1/2
      TempoPattern("\uECA3",                     1.0/30.0,   TDuration::DurationType::V_HALF),       // 1/2
      TempoPattern("\uECA7\\s*\uECB7\\s*\uECB7", 1.75/120.0, TDuration::DurationType::V_EIGHTH, 2),  // double dotted 1/8
      TempoPattern("\uECA7\\s*\uECB7",           1.5/120.0,  TDuration::DurationType::V_EIGHTH, 1),  // dotted 1/8
      TempoPattern("\uECA7",                     1.0/120.0,  TDuration::DurationType::V_EIGHTH),     // 1/8
      TempoPattern("\uECA2\\s*\uECB7",           1.5/15.0,   TDuration::DurationType::V_WHOLE, 1),   // dotted whole
      TempoPattern("\uECA2",                     1.0/15.0,   TDuration::DurationType::V_WHOLE),      // whole
      TempoPattern("\uECA9\\s*\uECB7",           1.5/240.0,  TDuration::DurationType::V_16TH, 1),    // dotted 1/16
      TempoPattern("\uECA9",                     1.0/240.0,  TDuration::DurationType::V_16TH),       // 1/16
      TempoPattern("\uECAB\\s*\uECB7",           1.5/480.0,  TDuration::DurationType::V_32ND, 1),    // dotted 1/32
      TempoPattern("\uECAB",                     1.0/480.0,  TDuration::DurationType::V_32ND),       // 1/32
      TempoPattern("\uECA1",                     1.0/7.5,    TDuration::DurationType::V_BREVE),      // longa
      TempoPattern("\uECA0",                     1.0/7.5,    TDuration::DurationType::V_BREVE),      // double whole
      TempoPattern("\uECAD",                     1.0/960.0,  TDuration::DurationType::V_64TH),       // 1/64
      TempoPattern("\uECAF",                     1.0/1920.0, TDuration::DurationType::V_128TH),      // 1/128
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
      for (const auto& i : tp) {
            QRegExp re(i.pattern);
            int pos = re.indexIn(s);
            if (pos != -1) {
                  len = re.matchedLength();
                  dur = i.d;
                  return pos;
                  }
            }
      return -1;
      }

static const TempoPattern tpSym[] = {
      TempoPattern("<sym>metNoteQuarterUp</sym>\\s*<sym>metAugmentationDot</sym>\\s*<sym>metAugmentationDot</sym>",                         1.75/60.0, TDuration::DurationType::V_QUARTER, 2), // double dotted 1/4
      TempoPattern("<sym>metNoteQuarterUp</sym>\\s*<sym>metAugmentationDot</sym>",          1.5/60.0,  TDuration::DurationType::V_QUARTER, 1), // dotted 1/4
      TempoPattern("<sym>metNoteQuarterUp</sym>",                                           1.0/60.0,  TDuration::DurationType::V_QUARTER),  // 1/4
      TempoPattern("<sym>metNoteHalfUp</sym>\\s*<sym>metAugmentationDot</sym>\\s*<sym>metAugmentationDot</sym>",                         1.75/30.0, TDuration::DurationType::V_HALF, 2), // double dotted 1/2
      TempoPattern("<sym>metNoteHalfUp</sym>\\s*<sym>metAugmentationDot</sym>",             1.5/30.0,  TDuration::DurationType::V_HALF, 1),    // dotted 1/2
      TempoPattern("<sym>metNoteHalfUp</sym>",                                              1.0/30.0,  TDuration::DurationType::V_HALF),     // 1/2
      TempoPattern("<sym>metNote8thUp</sym>\\s*<sym>metAugmentationDot</sym>\\s*<sym>metAugmentationDot</sym>",         1.75/120.0, TDuration::DurationType::V_EIGHTH, 2), // double dotted 1/8
      TempoPattern("<sym>metNote8thUp</sym>\\s*<sym>metAugmentationDot</sym>",              1.5/120.0, TDuration::DurationType::V_EIGHTH, 1),  // dotted 1/8
      TempoPattern("<sym>metNote8thUp</sym>",                                               1.0/120.0, TDuration::DurationType::V_EIGHTH),   // 1/8
      TempoPattern("<sym>metNoteWhole</sym>\\s*<sym>metAugmentationDot</sym>",              1.5/15.0, TDuration::DurationType::V_WHOLE, 1),    // dotted whole
      TempoPattern("<sym>metNoteWhole</sym>",                                               1.0/15.0,  TDuration::DurationType::V_WHOLE),    // whole
      TempoPattern("<sym>metNote16thUp</sym>\\s*<sym>metAugmentationDot</sym>",             1.5/240.0, TDuration::DurationType::V_16TH, 1),  // dotted 1/16
      TempoPattern("<sym>metNote16thUp</sym>",                                              1.0/240.0, TDuration::DurationType::V_16TH),     // 1/16
      TempoPattern("<sym>metNote32ndUp</sym>\\s*<sym>metAugmentationDot</sym>",             1.5/480.0, TDuration::DurationType::V_32ND, 1),  // dotted 1/32
      TempoPattern("<sym>metNote32ndUp</sym>",                                              1.0/480.0, TDuration::DurationType::V_32ND),     // 1/32
      TempoPattern("<sym>metNoteDoubleWholeSquare</sym>",                                   1.0/7.5,   TDuration::DurationType::V_BREVE),    // longa
      TempoPattern("<sym>metNoteDoubleWhole</sym>",                                         1.0/7.5,   TDuration::DurationType::V_BREVE),    // double whole
      TempoPattern("<sym>metNote64thUp</sym>",                                              1.0/960.0, TDuration::DurationType::V_64TH),     // 1/64
      TempoPattern("<sym>metNote128thUp</sym>",                                             1.0/1920.0,TDuration::DurationType::V_128TH),    // 1/128
      };

//---------------------------------------------------------
//   duration2tempoTextString
//    find the tempoText string representation for duration
//---------------------------------------------------------

QString TempoText::duration2tempoTextString(const TDuration dur)
      {
      for (const TempoPattern& pa : tpSym) {
            if (pa.d == dur) {
                  QString res = pa.pattern;
                  res.replace("\\s*", " ");
                  return res;
                  }
            }
      return "";
      }

//---------------------------------------------------------
// updateScore
//---------------------------------------------------------

void TempoText::updateScore()
      {
      if (segment())
            score()->setTempo(segment(), _tempo);
      score()->fixTicks();
      score()->setPlaylistDirty();
      }

//---------------------------------------------------------
// updateRelative
//---------------------------------------------------------

void TempoText::updateRelative()
      {
      qreal tempoBefore = score()->tempo(tick() - 1);
      setTempo(tempoBefore * _relative);
      }

//---------------------------------------------------------
//   textChanged
//    text may have changed
//---------------------------------------------------------

void TempoText::textChanged()
      {
      if (!_followText)
            return;
      // cache regexp, they are costly to create
      static QHash<QString, QRegExp> regexps;
      static QHash<QString, QRegExp> regexps2;
      QString s = plainText();
      s.replace(",", ".");
      s.replace("<sym>space</sym>"," ");
      for (const TempoPattern& pa : tp) {
            QRegExp re;
            if (!regexps.contains(pa.pattern)) {
                  re = QRegExp(QString("%1\\s*=\\s*(\\d+[.]{0,1}\\d*)\\s*").arg(pa.pattern));
                  regexps[pa.pattern] = re;
                  }
            re = regexps.value(pa.pattern);
            if (re.indexIn(s) != -1) {
                  QStringList sl = re.capturedTexts();
                  if (sl.size() == 2) {
                        qreal nt = qreal(sl[1].toDouble()) * pa.f;
                        if (nt != _tempo) {
                              setTempo(qreal(sl[1].toDouble()) * pa.f);
                              _relative = 1.0;
                              _isRelative = false;
                              updateScore();
                              }
                        break;
                        }
                  }
            else {
                 for (const TempoPattern& pa2 : tp) {
                       QString key = QString("%1_%2").arg(pa.pattern).arg(pa2.pattern);
                       QRegExp re2;
                       if (!regexps2.contains(key)) {
                             re2 = QRegExp(QString("%1\\s*=\\s*%2\\s*").arg(pa.pattern).arg(pa2.pattern));
                             regexps2[key] = re2;
                             }
                       re2 = regexps2.value(key);
                       if (re2.indexIn(s) != -1) {
                             _relative = pa2.f / pa.f;
                             _isRelative = true;
                             updateRelative();
                             updateScore();
                             return;
                             }
                       }
                  }
            }
      }

//---------------------------------------------------------
//   setTempo
//---------------------------------------------------------

void TempoText::setTempo(qreal v)
      {
      if (v < MIN_TEMPO)
            v = MIN_TEMPO;
      else if (v > MAX_TEMPO)
            v = MAX_TEMPO;
      _tempo = v;
      }

//---------------------------------------------------------
//   undoSetTempo
//---------------------------------------------------------

void TempoText::undoSetTempo(qreal v)
      {
      undoChangeProperty(P_ID::TEMPO, v);
      }

//---------------------------------------------------------
//   undoSetFollowText
//---------------------------------------------------------

void TempoText::undoSetFollowText(bool v)
      {
      undoChangeProperty(P_ID::TEMPO_FOLLOW_TEXT, v);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TempoText::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::TEMPO:
                  return _tempo;
            case P_ID::TEMPO_FOLLOW_TEXT:
                  return _followText;
            default:
                  return TextBase::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TempoText::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::TEMPO:
                  setTempo(v.toDouble());
                  score()->setTempo(segment(), _tempo);
                  score()->fixTicks();
                  break;
            case P_ID::TEMPO_FOLLOW_TEXT:
                  _followText = v.toBool();
                  break;
            default:
                  if (!TextBase::setProperty(propertyId, v))
                        return false;
                  break;
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TempoText::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_ID::SUB_STYLE:
                  return int(SubStyle::TEMPO);
            case P_ID::TEMPO:
                  return 2.0;
            case P_ID::TEMPO_FOLLOW_TEXT:
                  return false;
            case P_ID::PLACEMENT:
                  return int(Placement::ABOVE);
            default:
                  return TextBase::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   layout
//    called after Measure->stretchMeasure()
//---------------------------------------------------------

void TempoText::layout()
      {
      qreal y;
      if (placeAbove())
            y = score()->styleP(StyleIdx::tempoPosAbove);
      else {
            qreal sh = staff() ? staff()->height() : 0;
            y = score()->styleP(StyleIdx::tempoPosBelow) + sh + lineSpacing();
            }
      setPos(QPointF(0.0, y));
      TextBase::layout1();

      // tempo text on first chordrest of measure should align over time sig if present
      //
      Segment* s = segment();
      if (s && !s->rtick()) {
            Segment* p = segment()->prev(SegmentType::TimeSig);
            if (p) {
                  rxpos() -= s->x() - p->x();
                  Element* e = p->element(staffIdx() * VOICES);
                  if (e)
                        rxpos() += e->x();
                  }
            }
      autoplaceSegmentElement(score()->styleP(StyleIdx::tempoMinDistance));
      }

//---------------------------------------------------------
//   duration2userName
//---------------------------------------------------------

QString TempoText::duration2userName(const TDuration t)
      {
      QString dots;
      switch (t.dots()) {
            case 1: dots = QObject::tr("Dotted %1").arg(t.durationTypeUserName());
                  break;
            case 2: dots = QObject::tr("Double dotted %1").arg(t.durationTypeUserName());
                  break;
            case 3: dots = QObject::tr("Triple dotted %1").arg(t.durationTypeUserName());
                  break;
            case 4: dots = QObject::tr("Quadruple dotted %1").arg(t.durationTypeUserName());
                  break;
            default:
                  dots = t.durationTypeUserName();
                  break;
            }
      return dots;
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString TempoText::accessibleInfo() const
      {
      TDuration t1;
      TDuration t2;
      int len1;
      int len2;
      QString text = plainText();
      QString firstPart = text.split(" = ").first();
      QString secondPart = text.split(" = ").back();
      int x1 = findTempoDuration(firstPart, len1, t1);
      int x2 = -1;
      if (_relative)
            x2 = findTempoDuration(secondPart, len2, t2);

      if (x1 != -1) {
            QString dots1;
            QString dots2;
            dots1 = duration2userName(t1);
            if (x2 != -1) {
                  dots2 = duration2userName(t2);
                  return QString("%1: %2 %3 = %4 %5").arg(Element::accessibleInfo()).arg(dots1).arg(QObject::tr("note")).arg(dots2).arg(QObject::tr("note"));
                  }
            else
                  return QString("%1: %2 %3 = %4").arg(Element::accessibleInfo()).arg(dots1).arg(QObject::tr("note")).arg(secondPart);
            }
      else
            return TextBase::accessibleInfo();
      }

}

