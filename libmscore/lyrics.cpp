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
      setTextStyleType(TEXT_STYLE_LYRIC1);
      _no          = 0;
      _ticks       = 0;
      _syllabic    = SINGLE;
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
      xml.stag("Lyrics");
      if (_no)
            xml.tag("no", _no);
      if (_syllabic != SINGLE) {
            static const char* sl[] = {
                  "single", "begin", "end", "middle"
                  };
            xml.tag("syllabic", sl[_syllabic]);
            }
      if (_ticks)
            xml.tag("ticks", _ticks);
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
                        _syllabic = SINGLE;
                  else if (val == "begin")
                        _syllabic = BEGIN;
                  else if (val == "end")
                        _syllabic = END;
                  else if (val == "middle")
                        _syllabic = MIDDLE;
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
      if (el->type() == LINE)
            _separator.append((Line*)el);
      else if (el->type() == TEXT)
            _verseNumber = static_cast<Text*>(el);
      else
            qDebug("Lyrics::add: unknown element %s", el->name());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Lyrics::remove(Element* el)
      {
      if (el->type() == LINE)
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
      Text::layout1();
      if (!parent()) // palette & clone trick
          return;

      const QList<Lyrics*>* ll = &(chordRest()->lyricsList());

      qreal lh = lineSpacing() * score()->styleD(ST_lyricsLineHeight);
      int line = ll->indexOf(this);
      qreal y  = lh * line + point(score()->styleS(ST_lyricsDistance));
      qreal x  = 0.0;
      //
      // left align if syllable has a number or is a melisma
      //
      if (_ticks == 0 && (textStyle().align() & ALIGN_HCENTER) && !_verseNumber)
            x += symWidth(SymId::noteheadBlack) * .5;
      else if (_ticks || ((textStyle().align() & ALIGN_HCENTER) && _verseNumber))
            x += width() * .5;
      rxpos() += x;
      rypos() += y;
      if (_verseNumber) {
            _verseNumber->layout();
            _verseNumber->setPos(-x, 0.0);
            }
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
//   paste
//---------------------------------------------------------

void Lyrics::paste()
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

      insertText(sl[0]);
      layout();
      score()->setLayoutAll(true);
      score()->end();
      sl.removeFirst();
      txt = sl.join(" ");

      QApplication::clipboard()->setText(txt, mode);
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

bool Lyrics::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      return e->type() == TEXT;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Lyrics::drop(const DropData& data)
      {
      Text* e = static_cast<Text*>(data.element);
      if (!(e->type() == TEXT && e->textStyle().name() == "Lyrics Verse")) {
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
      if (type() == LYRICS) {
            if( (_no & 1) && textStyleType() == TEXT_STYLE_LYRIC1)
                  setTextStyleType(TEXT_STYLE_LYRIC2);
            if( !(_no & 1) && textStyleType() == TEXT_STYLE_LYRIC2)
                  setTextStyleType(TEXT_STYLE_LYRIC1);
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
      switch(propertyId) {
            default:
                  return Text::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Lyrics::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
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
      switch(id) {
            default: return Text::propertyDefault(id);
            }
      }


}

