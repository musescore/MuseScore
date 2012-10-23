//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: lyrics.cpp 5655 2012-05-21 12:33:32Z lasconic $
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

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

Lyrics::Lyrics(Score* s)
   : Text(s)
      {
      setTextStyle(s->textStyle(TEXT_STYLE_LYRIC1));
      _no          = 0;
      _ticks       = 0;
      _syllabic    = SINGLE;
      _verseNumber = 0;
      }

Lyrics::Lyrics(const Lyrics& l)
   : Text(l)
      {
      _no  = l._no;
      if (styled())
            setTextStyle(l.textStyle());
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

void Lyrics::read(const QDomElement& de)
      {
      int   iEndTick = 0;           // used for backward compatibility

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "no")
                  _no = val.toInt();
            else if (tag == "syllabic") {
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
                  iEndTick = val.toInt();
                  }
            else if (tag == "ticks")
                  _ticks = val.toInt();
            else if (tag == "Number") {
                  _verseNumber = new Text(score());
                  _verseNumber->read(e);
                  _verseNumber->setParent(this);
                  }
            else if (!Text::readProperties(e))
                  domError(e);
            }
      // if any endTick, make it relative to current tick
      if (iEndTick) {
            _ticks = iEndTick - score()->curTick;
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
//   pagePos
//---------------------------------------------------------

QPointF Lyrics::pagePos() const
      {
      System* system = measure()->system();
      qreal yp = y();
      if (system)
	      yp = yp + system->staff(staffIdx())->y() + system->y();
      return QPointF(pageX(), yp);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Lyrics::layout()
      {
      if (styled())
            setTextStyle(score()->textStyle((_no % 2) ? TEXT_STYLE_LYRIC2 : TEXT_STYLE_LYRIC1));
      Text::layout();
      qreal lh = lineSpacing() * score()->styleD(ST_lyricsLineHeight);
      if(!parent()) // palette & clone trick
          return;
      System* sys = measure()->system();
      if (sys == 0) {
            qDebug("lyrics layout: no system!");
            abort();
            }
      const QList<Lyrics*>* ll = &(chordRest()->lyricsList());

      int line = ll->indexOf(this);
      qreal y  = lh * line + point(score()->styleS(ST_lyricsDistance));
      qreal x  = 0.0;
      //
      // left align if syllable has a number or is a melisma
      //
      if (_ticks == 0 && (textStyle().align() & ALIGN_HCENTER) && !_verseNumber)
            x += symbols[score()->symIdx()][quartheadSym].width(magS()) * .5;
      else if (_ticks || ((textStyle().align() & ALIGN_HCENTER) && _verseNumber))
            x += width() * .5;
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

void Lyrics::paste()
      {
#if defined(Q_WS_MAC) || defined(__MINGW32__)
      QClipboard::Mode mode = QClipboard::Clipboard;
#else
      QClipboard::Mode mode = QClipboard::Selection;
#endif
      QString txt = QApplication::clipboard()->text(mode);
      QStringList sl = txt.split(QRegExp("\\s+"), QString::SkipEmptyParts);
      if (sl.isEmpty())
            return;

      cursor()->insertText(sl[0]);
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
      if (type() == LYRICS)
            setTextStyle(score()->textStyle((_no % 2) ? TEXT_STYLE_LYRIC2 : TEXT_STYLE_LYRIC1));
      }

