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

#include "lyrics.h"
#include "xml.h"
#include "system.h"
#include "measure.h"
#include "score.h"
#include "sym.h"
#include "segment.h"

namespace Ms {

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

Lyrics::Lyrics(Score* s)
   : Text(s)
      {
      setTextStyleType(TextStyleType::LYRIC1);
      _no          = 0;
      _ticks       = 0;
      _syllabic    = Syllabic::SINGLE;
      _verseNumber = 0;
      }

Lyrics::Lyrics(const Lyrics& l)
   : Text(l)
      {
      _no  = l._no;
      _ticks = l._ticks;
      _syllabic = l._syllabic;
      if (l._verseNumber)
            _verseNumber = new Text(*l._verseNumber);
      else
            _verseNumber = 0;
      QList<Line*> _separator;
      foreach(Line* l, l._separator)
            _separator.append(new Line(*l));
      }

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

Lyrics::~Lyrics()
      {
      delete _verseNumber;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Lyrics::scanElements(void* data, void (*func)(void*, Element*), bool)
      {
      if (_verseNumber)
            func(data, _verseNumber);
      func(data, this);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Lyrics::write(Xml& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag("Lyrics");
      if (_no)
            xml.tag("no", _no);
      if (_syllabic != Syllabic::SINGLE) {
            static const char* sl[] = {
                  "single", "begin", "end", "middle"
                  };
            xml.tag("syllabic", sl[int(_syllabic)]);
            }
      writeProperty(xml, P_ID::LYRIC_TICKS);

      Text::writeProperties(xml);
      if (_verseNumber) {
            xml.stag("Number");
            _verseNumber->writeProperties(xml);
            xml.etag();
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Lyrics::read(XmlReader& e)
      {
      int   iEndTick = 0;           // used for backward compatibility

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "no")
                  _no = e.readInt();
            else if (tag == "syllabic") {
                  QString val(e.readElementText());
                  if (val == "single")
                        _syllabic = Syllabic::SINGLE;
                  else if (val == "begin")
                        _syllabic = Syllabic::BEGIN;
                  else if (val == "end")
                        _syllabic = Syllabic::END;
                  else if (val == "middle")
                        _syllabic = Syllabic::MIDDLE;
                  else
                        qDebug("bad syllabic property");
                  }
            else if (tag == "endTick") {          // obsolete
                  // store <endTick> tag value until a <ticks> tag has been read
                  // which positions this lyrics element in the score
                  iEndTick = e.readInt();
                  }
            else if (tag == "ticks")
                  _ticks = e.readInt();
            else if (tag == "Number") {
                  _verseNumber = new Text(score());
                  _verseNumber->read(e);
                  _verseNumber->setParent(this);
                  }
            else if (!Text::readProperties(e))
                  e.unknown();
            }
      // if any endTick, make it relative to current tick
      if (iEndTick) {
            _ticks = iEndTick - e.tick();
            // qDebug("Lyrics::endTick: %d  ticks %d", iEndTick, _ticks);
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Lyrics::add(Element* el)
      {
      el->setParent(this);
      if (el->type() == Element::Type::LINE)
            _separator.append((Line*)el);
      else if (el->type() == Element::Type::TEXT)
            _verseNumber = static_cast<Text*>(el);
      else
            qDebug("Lyrics::add: unknown element %s", el->name());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Lyrics::remove(Element* el)
      {
      if (el->type() == Element::Type::LINE)
            _separator.removeAll((Line*)el);
      else if (el == _verseNumber)
            _verseNumber = 0;
      else
            qDebug("Lyrics::remove: unknown element %s", el->name());
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Lyrics::draw(QPainter* painter) const
      {
      Text::draw(painter);
      painter->setPen(curColor());
      foreach(const Line* l, _separator) {
            painter->translate(l->pos());
            l->draw(painter);
            painter->translate(-(l->pos()));
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Lyrics::layout()
      {
      // setPos(_textStyle.offset(spatium()));
      layout1();
      QPointF rp(readPos());
      if (!rp.isNull()) {
            if (score()->mscVersion() <= 114) {
                  rp.ry() += lineSpacing() + 2;
                  rp.rx() += bbox().width() * .5;
                  }
            setUserOff(rp - ipos());
            setReadPos(QPointF());
            }
      }

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void Lyrics::layout1()
      {
      setPos(textStyle().offset(spatium()));
      Text::layout1();
      if (!parent()) // palette & clone trick
          return;

      const QList<Lyrics*>* ll = &(chordRest()->lyricsList());

      qreal lh = lineSpacing() * score()->styleD(StyleIdx::lyricsLineHeight);
      int line = ll->indexOf(this);
      qreal y  = lh * line + point(score()->styleS(StyleIdx::lyricsDistance));
      qreal x  = 0.0;

      //
      // left align if syllable has a number or is a melisma or is first syllable of hyphenated word
      //
      ChordRest* cr = chordRest();
      qreal maxWidth;
      if (cr->type() == Element::Type::CHORD)
            maxWidth = static_cast<Chord*>(cr)->maxHeadWidth();
      else
            maxWidth = cr->width();       // TODO: exclude ledger line for multivoice rest?
      qreal nominalWidth = symWidth(SymId::noteheadBlack);
      bool hyphenatedMelisma = false;
      if (_syllabic == Syllabic::BEGIN || _syllabic == Syllabic::MIDDLE) {
            // hyphenated syllables representing melismas need to be left aligned
            // detecting this means finding next CR on same track and checking for existence of lyric in same verse
            Segment* s = cr->segment()->next1();
            ChordRest* ncr = s ? s->nextChordRest(cr->track()) : nullptr;
            if (ncr && !ncr->lyrics(_no))
                  hyphenatedMelisma = true;
            }
      if (_ticks == 0 && !hyphenatedMelisma && (textStyle().align() & AlignmentFlags::HCENTER) && !_verseNumber)
            x +=  nominalWidth * .5 - cr->x();
      else if (_ticks || hyphenatedMelisma || ((textStyle().align() & AlignmentFlags::HCENTER) && _verseNumber))
            x += (width() + nominalWidth - maxWidth) * .5 - cr->x();
      rxpos() += x;
      rypos() += y;
      if (_verseNumber) {
            _verseNumber->layout();
            _verseNumber->setPos(-x, 0.0);
            }
      }

//---------------------------------------------------------
//   paste
//---------------------------------------------------------

void Lyrics::paste(MuseScoreView* scoreview)
      {
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
      QClipboard::Mode mode = QClipboard::Clipboard;
#else
      QClipboard::Mode mode = QClipboard::Selection;
#endif
      QString txt = QApplication::clipboard()->text(mode);
      QStringList sl = txt.split(QRegExp("\\s+"), QString::SkipEmptyParts);
      if (sl.isEmpty())
            return;

      QStringList hyph = sl[0].split("-");
      bool minus = false;
      if(hyph.length() > 1) {
            insertText(hyph[0]);
            hyph.removeFirst();
            sl[0] =  hyph.join("-");
            minus = true;
            }
      else if (sl.length() > 1 && sl[1] == "-") {
            insertText(sl[0]);
            sl.removeFirst();
            sl.removeFirst();
            minus = true;
            }
      else {
            insertText(sl[0]);
            sl.removeFirst();
            }

      layout();
      score()->setLayoutAll(true);
      score()->end();
      txt = sl.join(" ");

      QApplication::clipboard()->setText(txt, mode);
      if (minus)
            scoreview->lyricsMinus();
      else
            scoreview->lyricsTab(false, false, true);
      }

//---------------------------------------------------------
//   endTick
//---------------------------------------------------------

int Lyrics::endTick() const
      {
      return segment()->tick() + ticks();
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Lyrics::acceptDrop(const DropData& data) const
      {
      return data.element->type() == Element::Type::TEXT;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Lyrics::drop(const DropData& data)
      {
      Text* e = static_cast<Text*>(data.element);
      if (!(e->type() == Element::Type::TEXT && e->textStyle().name() == "Lyrics Verse")) {
            delete e;
            return 0;
            }
      e->setParent(this);
      score()->undoAddElement(e);
      return e;
      }

//---------------------------------------------------------
//   setNo
//---------------------------------------------------------

void Lyrics::setNo(int n)
      {
      _no = n;
      // adjust beween LYRICS1 and LYRICS2 only; keep other styles as they are
      // (_no is 0-based, so odd _no means even line and viceversa)
      if (type() == Element::Type::LYRICS) {
            if( (_no & 1) && textStyleType() == TextStyleType::LYRIC1)
                  setTextStyleType(TextStyleType::LYRIC2);
            if( !(_no & 1) && textStyleType() == TextStyleType::LYRIC2)
                  setTextStyleType(TextStyleType::LYRIC1);
            }
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Lyrics::endEdit()
      {
      Text::endEdit();
      score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Lyrics::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::SYLLABIC:
                  return int(_syllabic);
            case P_ID::LYRIC_TICKS:
                  return _ticks;
            default:
                  return Text::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Lyrics::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::SYLLABIC:
                  _syllabic = Syllabic(v.toInt());
                  break;
            case P_ID::LYRIC_TICKS:
                  _ticks = v.toInt();
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

QVariant Lyrics::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::SYLLABIC:
                  return int(Syllabic::SINGLE);
            case P_ID::LYRIC_TICKS:
                  return 0;
            default: return Text::propertyDefault(id);
            }
      }


}

