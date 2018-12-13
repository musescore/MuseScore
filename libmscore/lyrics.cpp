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

#include "chord.h"
#include "score.h"
#include "sym.h"
#include "system.h"
#include "xml.h"
#include "staff.h"
#include "segment.h"
#include "undo.h"
#include "textedit.h"
#include "measure.h"

namespace Ms {

//---------------------------------------------------------
//   lyricsElementStyle
//---------------------------------------------------------

static const ElementStyle lyricsElementStyle {
      { Sid::lyricsPlacement, Pid::PLACEMENT  },
      };

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

Lyrics::Lyrics(Score* s)
   : TextBase(s, Tid::LYRICS_ODD)
      {
      _even       = false;
      initElementStyle(&lyricsElementStyle);
      _no         = 0;
      _ticks      = 0;
      _syllabic   = Syllabic::SINGLE;
      _separator  = 0;
      }

Lyrics::Lyrics(const Lyrics& l)
   : TextBase(l)
      {
      _even      = l._even;
      _no        = l._no;
      _ticks     = l._ticks;
      _syllabic  = l._syllabic;
      _separator = 0;
      }

Lyrics::~Lyrics()
      {
      if (_separator)
            remove(_separator);
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Lyrics::scanElements(void* data, void (*func)(void*, Element*), bool /*all*/)
      {
      func(data, this);
/* DO NOT ADD EITHER THE LYRICSLINE OR THE SEGMENTS: segments are added through the system each belongs to;
      LyricsLine is not needed, as it is internally manged.
      if (_separator)
            _separator->scanElements(data, func, all); */
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Lyrics::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this);
      writeProperty(xml, Pid::VERSE);
      if (_syllabic != Syllabic::SINGLE) {
            static const char* sl[] = {
                  "single", "begin", "end", "middle"
                  };
            xml.tag("syllabic", sl[int(_syllabic)]);
            }
      writeProperty(xml, Pid::LYRIC_TICKS);

      TextBase::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Lyrics::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Lyrics::readProperties(XmlReader& e)
      {
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
      else if (tag == "ticks")
            _ticks = e.readInt();
      else if (readProperty(tag, e, Pid::PLACEMENT))
            ;
      else if (!TextBase::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Lyrics::add(Element* el)
      {
//      el->setParent(this);
//      if (el->type() == ElementType::LINE)
//            _separator.append((Line*)el);           // ignore! Internally managed
//            ;
//      else
            qDebug("Lyrics::add: unknown element %s", el->name());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Lyrics::remove(Element* el)
      {
      if (el->isLyricsLine()) {
            // only if separator still exists and is the right one
            if (_separator && el == _separator) {
                  // Lyrics::remove() and LyricsLine::removeUnmanaged() call each other;
                  // be sure each finds a clean context
                  LyricsLine* separ = _separator;
                  _separator = 0;
                  separ->setParent(0);
                  separ->removeUnmanaged();
//done in undo/redo?                  delete separ;
                  }
            }
      else
            qDebug("Lyrics::remove: unknown element %s", el->name());
      }

//---------------------------------------------------------
//   isMelisma
//---------------------------------------------------------

bool Lyrics::isMelisma() const
      {
      // entered as melisma using underscore?
      if (_ticks > 0)
            return true;

      // hyphenated?
      // if so, it is a melisma only if there is no lyric in same verse on next CR
      if (_syllabic == Syllabic::BEGIN || _syllabic == Syllabic::MIDDLE) {
            // find next CR on same track and check for existence of lyric in same verse
            ChordRest* cr  = chordRest();
            if (cr) {
                  Segment* s     = cr->segment()->next1();
                  ChordRest* ncr = s ? s->nextChordRest(cr->track()) : 0;
                  if (ncr && !ncr->lyrics(_no, placement()))
                        return true;
                  }
            }

      // default - not a melisma
      return false;
      }

//---------------------------------------------------------
//   layout
//    - does not touch vertical position
//---------------------------------------------------------

void Lyrics::layout()
      {
      if (!parent()) { // palette & clone trick
            setPos(QPointF());
            TextBase::layout1();
            return;
            }

      //
      // parse leading verse number and/or punctuation, so we can factor it into layout separately
      //
      bool hasNumber     = false; // _verseNumber;
      qreal centerAdjust = 0.0;
      qreal leftAdjust   = 0.0;

      // find:
      // 1) string of numbers and non-word characters at start of syllable
      // 2) at least one other character (indicating start of actual lyric)
      // 3) string of non-word characters at end of syllable
      //QRegularExpression leadingPattern("(^[\\d\\W]+)([^\\d\\W]+)");

      if (score()->styleB(Sid::lyricsAlignVerseNumber)) {
            QString s = plainText();
            QRegularExpression punctuationPattern("(^[\\d\\W]*)([^\\d\\W].*?)([\\d\\W]*$)", QRegularExpression::UseUnicodePropertiesOption);
            QRegularExpressionMatch punctuationMatch = punctuationPattern.match(s);
            if (punctuationMatch.hasMatch()) {
                  // leading and trailing punctuation
                  QString lp = punctuationMatch.captured(1);
                  QString tp = punctuationMatch.captured(3);
                  // actual lyric
                  //QString actualLyric = punctuationMatch.captured(2);
                  if (!lp.isEmpty() || !tp.isEmpty()) {
//                        qDebug("create leading, trailing <%s> -- <%s><%s>", qPrintable(s), qPrintable(lp), qPrintable(tp));
                        Lyrics leading(*this);
                        leading.setPlainText(lp);
                        leading.layout1();
                        Lyrics trailing(*this);
                        trailing.setPlainText(tp);
                        trailing.layout1();
                        leftAdjust = leading.width();
                        centerAdjust = leading.width() - trailing.width();
                        if (!lp.isEmpty() && lp[0].isDigit())
                              hasNumber = true;
                        }
                  }
            }

      bool styleDidChange = false;
      if ((_no & 1) && !_even) {
            initTid(Tid::LYRICS_EVEN, /* preserveDifferent */ true);
            _even             = true;
            styleDidChange    = true;
            }
      if (!(_no & 1) && _even) {
            initTid(Tid::LYRICS_ODD, /* preserveDifferent */ true);
            _even             = false;
            styleDidChange    = true;
            }

      if (styleDidChange)
            styleChanged();

      if (isMelisma() || hasNumber)
            if (isStyled(Pid::ALIGN)) {
                  setAlign(score()->styleV(Sid::lyricsMelismaAlign).value<Align>());
            }
      QPointF o(propertyDefault(Pid::OFFSET).toPointF());
      rxpos() = o.x();
      qreal x = pos().x();
      TextBase::layout1();

      ChordRest* cr = chordRest();

      if (align() & Align::HCENTER) {
            //
            // center under notehead, not origin
            // however, lyrics that are melismas or have verse numbers will be forced to left alignment
            //
            // center under note head
            qreal nominalWidth = symWidth(SymId::noteheadBlack);
            x += nominalWidth * .5 - cr->x() - centerAdjust * 0.5;
            }
      else if (!(align() & Align::RIGHT)) {
            // even for left aligned syllables, ignore leading verse numbers and/or punctuation
            x -= leftAdjust;
            }

      rxpos() = x;

      if (_ticks > 0 || _syllabic == Syllabic::BEGIN || _syllabic == Syllabic::MIDDLE) {
            if (!_separator) {
                  _separator = new LyricsLine(score());
                  _separator->setTick(cr->tick());
                  score()->addUnmanagedSpanner(_separator);
                  }
            _separator->setParent(this);
            _separator->setTick(cr->tick());
            // HACK separator should have non-zero length to get its layout
            // always triggered. A proper ticks length will be set later on the
            // separator layout.
            _separator->setTicks(1);
            _separator->setTrack(track());
            _separator->setTrack2(track());
            // bbox().setWidth(bbox().width());  // ??
            }
      else {
            if (_separator) {
                  _separator->removeUnmanaged();
                  delete _separator;
                  _separator = 0;
                  }
            }
      }

//---------------------------------------------------------
//   layout2
//    compute vertical position
//---------------------------------------------------------

void Lyrics::layout2(int nAbove)
      {
      qreal lh = lineSpacing() * score()->styleD(Sid::lyricsLineHeight);

      if (placeBelow()) {
            qreal yo = segment()->measure()->system()->staff(staffIdx())->bbox().height();
            rypos()  = lh * (_no - nAbove) + yo;
            rpos()  += styleValue(Pid::OFFSET, Sid::lyricsPosBelow).toPointF();
            }
      else {
            rypos() = -lh * (nAbove - _no - 1);
            rpos() += styleValue(Pid::OFFSET, Sid::lyricsPosAbove).toPointF();
            }
      }

//---------------------------------------------------------
//   paste
//---------------------------------------------------------

void Lyrics::paste(EditData& ed)
      {
      MuseScoreView* scoreview = ed.view;
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
      QClipboard::Mode mode = QClipboard::Clipboard;
#else
      QClipboard::Mode mode = QClipboard::Selection;
#endif
      QString txt = QApplication::clipboard()->text(mode);
      QString regex = QString("[^\\S") + QChar(0xa0) + QChar(0x202F) + "]+";
      QStringList sl = txt.split(QRegExp(regex), QString::SkipEmptyParts);
      if (sl.empty())
            return;

      QStringList hyph = sl[0].split("-");
      bool minus = false;
      bool underscore = false;
      score()->startCmd();

      if(hyph.length() > 1) {
            score()->undo(new InsertText(cursor(ed), hyph[0]), &ed);
            hyph.removeFirst();
            sl[0] =  hyph.join("-");
            minus = true;
            }
      else if (sl.length() > 1 && sl[1] == "-") {
            score()->undo(new InsertText(cursor(ed), sl[0]), &ed);
            sl.removeFirst();
            sl.removeFirst();
            minus = true;
            }
      else if (sl[0].startsWith("_")) {
            sl[0].remove(0, 1);
            if (sl[0].isEmpty())
                  sl.removeFirst();
            underscore = true;
            }
      else if (sl[0].contains("_")) {
            int p = sl[0].indexOf("_");
            score()->undo(new InsertText(cursor(ed), sl[0]), &ed);
            sl[0] = sl[0].mid(p + 1);
            if (sl[0].isEmpty())
                  sl.removeFirst();
            underscore = true;
            }
      else if (sl.length() > 1 && sl[1] == "_") {
            score()->undo(new InsertText(cursor(ed), sl[0]), &ed);
            sl.removeFirst();
            sl.removeFirst();
            underscore = true;
            }
      else {
            score()->undo(new InsertText(cursor(ed), sl[0]), &ed);
            sl.removeFirst();
            }

      score()->endCmd();
      txt = sl.join(" ");

      QApplication::clipboard()->setText(txt, mode);
      if (minus)
            scoreview->lyricsMinus();
      else if (underscore)
            scoreview->lyricsUnderscore();
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

bool Lyrics::acceptDrop(EditData& data) const
      {
      return data.dropElement->isText() || TextBase::acceptDrop(data);
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Lyrics::drop(EditData& data)
      {
      ElementType type = data.dropElement->type();
      if (type == ElementType::SYMBOL || type == ElementType::FSYMBOL) {
            TextBase::drop(data);
            return 0;
            }
      if (!data.dropElement->isText()) {
            delete data.dropElement;
            data.dropElement = 0;
            return 0;
            }
      Text* e = toText(data.dropElement);
      e->setParent(this);
      score()->undoAddElement(e);
      return e;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Lyrics::endEdit(EditData& ed)
      {
      TextBase::endEdit(ed);
      score()->setLayoutAll();
      }

//---------------------------------------------------------
//   removeFromScore
//---------------------------------------------------------

void Lyrics::removeFromScore()
      {
      if (_separator) {
            _separator->removeUnmanaged();
            delete _separator;
            _separator = 0;
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Lyrics::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::SYLLABIC:
                  return int(_syllabic);
            case Pid::LYRIC_TICKS:
                  return _ticks;
            case Pid::VERSE:
                  return _no;
            default:
                  return TextBase::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Lyrics::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::PLACEMENT:
                  setPlacement(Placement(v.toInt()));
                  break;
            case Pid::SYLLABIC:
                  _syllabic = Syllabic(v.toInt());
                  break;
            case Pid::LYRIC_TICKS:
                  _ticks = v.toInt();
                  break;
            case Pid::VERSE:
                  _no = v.toInt();
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

QVariant Lyrics::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::SUB_STYLE:
                  return int((_no & 1) ? Tid::LYRICS_EVEN : Tid::LYRICS_ODD);
            case Pid::PLACEMENT:
                  return score()->styleV(Sid::lyricsPlacement);
            case Pid::SYLLABIC:
                  return int(Syllabic::SINGLE);
            case Pid::LYRIC_TICKS:
            case Pid::VERSE:
                  return 0;
            case Pid::ALIGN:
                  if (isMelisma())
                        return score()->styleV(Sid::lyricsMelismaAlign);
                  // fall through
            default:
                  return TextBase::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   forAllLyrics
//---------------------------------------------------------

void Score::forAllLyrics(std::function<void(Lyrics*)> f)
      {
      for (Segment* s = firstSegment(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
            for (Element* e : s->elist()) {
                  if (e) {
                        for (Lyrics* l : toChordRest(e)->lyrics()) {
                              f(l);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void Lyrics::undoChangeProperty(Pid id, const QVariant& v, PropertyFlags ps)
      {
      if (id == Pid::VERSE && no() != v.toInt()) {
            for (Lyrics* l : chordRest()->lyrics()) {
                  if (l->no() == v.toInt()) {
                        // verse already exists, swap
                        l->TextBase::undoChangeProperty(id, no(), ps);
                        Placement p = l->placement();
                        l->TextBase::undoChangeProperty(Pid::PLACEMENT, int(placement()), ps);
                        TextBase::undoChangeProperty(Pid::PLACEMENT, int(p), ps);
                        break;
                        }
                  }
            TextBase::undoChangeProperty(id, v, ps);
            return;
            }
      if (id == Pid::PLACEMENT) {
            if (Placement(v.toInt()) == Placement::ABOVE) {
                  // change placment of all verse for the same voice upto this one to ABOVE
                  score()->forAllLyrics([this,id,v,ps](Lyrics* l) {
                        if (l->no() <= no() && l->voice() == voice())
                              l->TextBase::undoChangeProperty(id, v, ps);
                        });
                  }
            else {
                  // change placment of all verse for the same voce starting from this one to BELOW
                  score()->forAllLyrics([this,id,v,ps](Lyrics* l) {
                        if (l->no() >= no() && l->voice() == voice())
                              l->TextBase::undoChangeProperty(id, v, ps);
                        });
                  }
            return;
            }

      TextBase::undoChangeProperty(id, v, ps);
      }

}

