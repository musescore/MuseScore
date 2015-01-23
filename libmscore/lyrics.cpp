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

namespace Ms {

// some useful values:
static const qreal      HALF                                = 0.5;
static const qreal      TWICE                               = 2.0;

//---------------------------------------------------------
//   searchNextLyrics
//---------------------------------------------------------

static Lyrics* searchNextLyrics(Segment* s, int staffIdx, int verse)
      {
      Lyrics* l = 0;
      while ((s = s->next1(Segment::Type::ChordRest))) {
            int strack = staffIdx * VOICES;
            int etrack = strack + VOICES;
            // search through all tracks of current staff looking for a lyric in specified verse
            for (int track = strack; track < etrack; ++track) {
                  ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                  if (cr && !cr->lyricsList().isEmpty()) {
                        // cr with lyrics found, but does it have a syllable in specified verse?
                        l = cr->lyricsList().value(verse);
                        if (l)
                              break;
                        }
                  }
            if (l)
                  break;
            }
      return l;
      }

//=========================================================
//   Lyrics
//=========================================================

Lyrics::Lyrics(Score* s)
   : Text(s)
      {
      setTextStyleType(TextStyleType::LYRIC1);
      _no         = 0;
      _ticks      = 0;
      _syllabic   = Syllabic::SINGLE;
      _separator  = nullptr;
      }

Lyrics::Lyrics(const Lyrics& l)
   : Text(l)
      {
      _no         = l._no;
      _ticks      = l._ticks;
      _syllabic   = l._syllabic;
#if 0
      // if we copy lines at all, they need to be parented to new lyric
      // but they will be regenerated upon layout anyhow
      for (const Line* line : l._separator) {
            Line* nline = new Line(*line);
            nline->setParent(this);
            _separator.append(nline);
            }
#endif
      _separator = nullptr;
      }

Lyrics::~Lyrics()
      {
      if (_separator != nullptr) {
            _separator->unchain();
            delete _separator;
            }
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Lyrics::scanElements(void* data, void (*func)(void*, Element*), bool /*all*/)
      {
      func(data, this);
/* DO NOT ADD EITHER THE LYRICSLINE OR THE SEGMENTS: segments are added through the system each belongs to;
      LyricsLine is not needed, as it is internally manged.
      if (_separator != nullptr)
            _separator->scanElements(data, func, all); */
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
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Lyrics::read(XmlReader& e)
      {
      int   iEndTick = 0;           // used for backward compatibility
      Text* _verseNumber = 0;

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
            else if (tag == "Number") {                           // obsolete
                  Text* _verseNumber = new Text(score());
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
      if (_verseNumber) {
            // TODO: add text to main text
            }

      delete _verseNumber;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Lyrics::add(Element* el)
      {
      el->setParent(this);
      if (el->type() == Element::Type::LINE)
//            _separator.append((Line*)el);           // ignore! Internally managed
            ;
      else
            qDebug("Lyrics::add: unknown element %s", el->name());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Lyrics::remove(Element* el)
      {
      if (el->type() == Element::Type::LYRICSLINE) {
            _separator->unchain();
            delete _separator;
            _separator = nullptr;
            }
      else
            qDebug("Lyrics::remove: unknown element %s", el->name());
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

/* As the lyrics line is now drawn by the system it belongs to, standard Text drawing is enough.

void Lyrics::draw(QPainter* painter) const
      {
      Text::draw(painter);
      }
*/

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
            ChordRest* cr = chordRest();
            Segment* s = cr->segment()->next1();
            ChordRest* ncr = s ? s->nextChordRest(cr->track()) : nullptr;
            if (ncr && !ncr->lyrics(_no))
                  return true;
            }

      // default - not a melisma
      return false;
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

#if defined (USE_FONT_DASH_METRIC)
      static QString    g_fontFamily      = QString();
      static qreal      g_fontSize        = -1;
      static qreal      g_cachedDashY;
      static qreal      g_cachedDashLength;
   #if defined(USE_FONT_DASH_TICKNESS)
      static qreal      g_cachedDashThickness;
   #endif
#endif

void Lyrics::layout1()
      {
      setPos(textStyle().offset(spatium()));
      Text::layout1();
      if (!parent()) // palette & clone trick
          return;

      ChordRest* cr = chordRest();
      const QList<Lyrics*>* ll = &(cr->lyricsList());

      qreal lh = lineSpacing() * score()->styleD(StyleIdx::lyricsLineHeight);
      int line = ll->indexOf(this);
      qreal y  = lh * line + point(score()->styleS(StyleIdx::lyricsDistance));
      qreal x  = 0.0;

      //
      // parse leading verse number and/or punctuation, so we can factor it into layout separately
      // TODO: provide a way to disable this
      //
      bool hasNumber = false; // _verseNumber;
      qreal adjust = 0.0;
      QString s = plainText(true);
      // find:
      // 1) string of numbers and non-word characters at start of syllable
      // 2) at least one other character (indicating start of actual lyric)
      QRegularExpression leadingPattern("(^[\\d\\W]+)([^\\d\\W]+)");
      QRegularExpressionMatch leadingMatch = leadingPattern.match(s);
      if (leadingMatch.hasMatch()) {
            // leading string
            QString s1 = leadingMatch.captured(1);
            // actual lyric
            //QString s2 = leadingMatch.captured(2);
            Text leading(*this);
            leading.setPlainText(s1);
            leading.layout1();
            adjust = leading.width();
            if (!s1.isEmpty() && s1[0].isDigit())
                  hasNumber = true;
            }

      if (textStyle().align() & AlignmentFlags::HCENTER) {
            //
            // center under notehead, not origin
            // however, lyrics that are melismas or have verse numbers will be forced to left alignment
            // TODO: provide a way to disable the automatic left alignment
            //
            qreal maxWidth;
            if (cr->type() == Element::Type::CHORD)
                  maxWidth = static_cast<Chord*>(cr)->maxHeadWidth();
            else
                  maxWidth = cr->width();       // TODO: exclude ledger line for multivoice rest?
            qreal nominalWidth = symWidth(SymId::noteheadBlack);
            if (!isMelisma() && !hasNumber)     // center under notehead
                  x +=  nominalWidth * .5 - cr->x() - adjust * 0.5;
            else                                // force left alignment
                  x += (width() + nominalWidth - maxWidth) * .5 - cr->x() - adjust;
            }
      else {
            // even for left aligned syllables, ignore leading verse numbers and/or punctuation
            x -= adjust;
            }

      rxpos() += x;
      rypos() += y;

      if (_ticks > 0 || _syllabic == Syllabic::BEGIN || _syllabic == Syllabic::MIDDLE) {
            if (_separator == nullptr) {
                  _separator = new LyricsLine(score());
                  _separator->setTick(cr->tick());
                  score()->addUnmanagedSpanner(_separator);
                  }
            _separator->setParent(this);
            _separator->setTick(cr->tick());
            _separator->setTrack(track());
            _separator->setTrack2(track());
#if defined(USE_FONT_DASH_METRIC)
            // if font parameters different from font cached values, compute new dash values from font metrics
            if (textStyle().family() != g_fontFamily && textStyle().size() != g_fontSize) {
                  QFontMetricsF     fm    = textStyle().fontMetrics(spatium());
                  QRectF            r     = fm.tightBoundingRect("\u2013");   // U+2013 EN DASH
                  g_cachedDashY           = _dashY          = r.y() + (r.height() * HALF);
                  g_cachedDashLength      = _dashLength     = r.width();
   #if defined(USE_FONT_DASH_TICKNESS)
                  g_cachedDashThickness   = _dashThickness  = r.height();
   #endif
                  g_fontFamily            = textStyle().family();
                  g_fontSize              = textStyle().size();
                  }
            // if same font, use cached values
            else {
                  _dashY                  = g_cachedDashY;
                  _dashLength             = g_cachedDashLength;
   #if defined(USE_FONT_DASH_TICKNESS)
                  _dashThickness          = g_cachedDashThickness;
   #endif
                  }
#endif
            }
      else
            if (_separator != nullptr) {
                  _separator->unchain();
                  delete _separator;
                  _separator = nullptr;
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
      return data.element->type() == Element::Type::TEXT || Text::acceptDrop(data);
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Lyrics::drop(const DropData& data)
      {
      Element::Type type = data.element->type();
      if (type == Element::Type::SYMBOL || type == Element::Type::FSYMBOL) {
            Text::drop(data);
            return 0;
            }
      Text* e = static_cast<Text*>(data.element);
      if (!(type == Element::Type::TEXT && e->textStyle().name() == "Lyrics Verse Number")) {
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
//   removeFromScore
//---------------------------------------------------------

void Lyrics::removeFromScore()
      {
      if (_separator) {
            _separator->unchain();
            delete _separator;
            _separator = nullptr;
            }
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

//=========================================================
//   LyricsLine
//=========================================================

LyricsLine::LyricsLine(Score* s)
  : SLine(s)
      {
      setFlags(0);

      setGenerated(true);           // no need to save it, as it can be re-generated
      setDiagonal(false);
      setLineWidth(Spatium(Lyrics::LYRICS_DASH_DEFAULT_LINE_THICKNESS));
      setAnchor(Spanner::Anchor::SEGMENT);
      _nextLyrics = nullptr;
      }

LyricsLine::LyricsLine(const LyricsLine& g)
   : SLine(g)
      {
      _nextLyrics = nullptr;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void LyricsLine::layout()
      {
      bool tempMelismaTicks = (lyrics()->ticks() == Lyrics::TEMP_MELISMA_TICKS);
      if (lyrics()->ticks() > 0) {              // melisma
            setLineWidth(Spatium(Lyrics::MELISMA_DEFAULT_LINE_THICKNESS));
            // if lyrics has a temporary one-chord melisma, set to 0 ticks (just its own chord)
            if (tempMelismaTicks)
                  lyrics()->setTicks(0);
            // Lyrics::_ticks points to the beginning of the last spanned segment,
            // but the line shall include it:
            // include the duration of this last segment in the melisma duration
            Segment* lyricsSegment = lyrics()->segment();
            int lyricsStartTick = lyricsSegment->tick();
            int lyricsEndTick = lyrics()->endTick();
            int lyricsTrack = lyrics()->track();
            // find segment with tick >= endTick
            Segment* s = lyricsSegment;
            while (s && s->tick() < lyricsEndTick)
                  s = s->nextCR();
            if (!s) {
                  // user probably deleted measures at end of score, leaving this melisma too long
                  // set s to last segment and reset lyricsEndTick to trigger FIXUP code below
                  s = score()->lastSegment();
                  lyricsEndTick = -1;
                  }
            Element* se = s->element(lyricsTrack);
            // everything is OK if we have reached a chord at right tick on right track
            if (s->tick() == lyricsEndTick && se && se->type() == Element::Type::CHORD) {
                  // advance to next CR, or last segment if no next CR
                  s = s->nextCR();
                  if (!s)
                        s = score()->lastSegment();
                  }
            else {
                  // FIXUP - lyrics tick count not valid
                  // this happens if edits to score have removed the original end segment
                  // so let's fix it here
                  // s is already pointing to segment past endTick (or to last segment)
                  // we should shorten the lyrics tick count to make this work
                  Segment* ns = s;
                  Segment* ps = s->prev1(Segment::Type::ChordRest);
                  while (ps && ps != lyricsSegment) {
                        Element* pe = ps->element(lyricsTrack);
                        // we're looking for an actual chord on this track
                        if (pe && pe->type() == Element::Type::CHORD)
                              break;
                        s = ps;
                        ps = ps->prev1(Segment::Type::ChordRest);
                        }
                  if (!ps || ps == lyricsSegment) {
                        // no valid previous CR, so try to lengthen melisma instead
                        ps = ns;
                        s = ps->nextCR(lyricsTrack);
                        Element* e = s ? s->element(lyricsTrack) : nullptr;
                        // check to make sure we have a chord
                        if (!e || e->type() != Element::Type::CHORD) {
                              // nothing to do but set ticks to 0
                              // this will result in melisma being deleted later
                              lyrics()->undoChangeProperty(P_ID::LYRIC_TICKS, 0);
                              setTicks(0);
                              return;
                              }
                        }
                  lyrics()->undoChangeProperty(P_ID::LYRIC_TICKS, ps->tick() - lyricsStartTick);
                  }
            setTicks(s->tick() - lyricsStartTick);
            }
      else {                                    // dash(es)
#if defined(USE_FONT_DASH_TICKNESS)
            setLineWidth(Spatium(lyrics()->dashThickness() / spatium()));
#endif
            _nextLyrics = searchNextLyrics(lyrics()->segment(), staffIdx(), lyrics()->no());
            setTick2(_nextLyrics != nullptr ? _nextLyrics->segment()->tick() : tick());
      }
      if (ticks()) {                // only do layout if some time span
            // do layout with non-0 duration
            if (tempMelismaTicks)
                  lyrics()->setTicks(Lyrics::TEMP_MELISMA_TICKS);
            SLine::layout();
            // if temp melisma and there is a first line segment,
            // extend it to be after the lyrics syllable (otherwise
            // the melisma segment will be often covered by the syllable itself)
            if (tempMelismaTicks && segments.size() > 0)
                  segmentAt(0)->rxpos2() += lyrics()->width();
            }
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* LyricsLine::createLineSegment()
      {
      LyricsLineSegment* seg = new LyricsLineSegment(score());
      seg->setTrack(track());
      seg->setColor(color());
      return seg;
      }

//---------------------------------------------------------
//   unchain
//
//    Remove the LyricsLine and its segments from objects which may know about them
//---------------------------------------------------------

void LyricsLine::unchain()
      {
      for (SpannerSegment* ss : spannerSegments())
            if (ss->system())
                  ss->system()->remove(ss);
      score()->removeUnmanagedSpanner(this);
      }

//=========================================================
//   LyricsLineSegment
//=========================================================

LyricsLineSegment::LyricsLineSegment(Score* s)
      : LineSegment(s)
      {
      setFlags(ElementFlag::SEGMENT | ElementFlag::ON_STAFF);
      setGenerated(true);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void LyricsLineSegment::layout()
      {
      Lyrics*     lyr;
      System*     sys;
      bool        endOfSystem = false;
      bool        isEndMelisma      = lyricsLine()->lyrics()->ticks() > 0;
      qreal       sp                = spatium();

      if (lyricsLine()->ticks() <= 0) {   // if no span,
            _numOfDashes = 0;             // nothing to draw
            return;                       // and do nothing
            }

      // HORIZONTAL POSITION
      // A) if line precedes a syllable, advance line end to right before the next syllable text
      // if not a melisma and there is a next syllable;
      if (!isEndMelisma && lyricsLine()->nextLyrics() != nullptr
                  && (spannerSegmentType() == SpannerSegmentType::END
                        || spannerSegmentType() == SpannerSegmentType::SINGLE)) {
            lyr         = lyricsLine()->nextLyrics();
            sys         = lyr->segment()->system();
            endOfSystem = (sys != system());
            // if next lyrics is on a different sytem, this line segment is at the end of its system:
            // do not adjust for next lyrics position
            if (!endOfSystem) {
                  qreal lyrX        = lyr->bbox().x();
                  qreal lyrXp       = lyr->pagePos().x();
                  qreal sysXp       = sys->pagePos().x();
                  qreal offsetX     = lyrXp - sysXp + lyrX - pos().x() - pos2().x();
                  //                    syst.rel. X pos.   | as a delta from current end pos.
                  offsetX           -= Lyrics::LYRICS_DASH_DEFAULT_PAD * sp;    // add ending padding
                  rxpos2()          += offsetX;
                  }
            }
      // B) if line follows a syllable, advance line start to after the syllable text
      lyr   = lyricsLine()->lyrics();
      sys   = lyr->segment()->system();
      if (spannerSegmentType() == SpannerSegmentType::BEGIN || spannerSegmentType() == SpannerSegmentType::SINGLE) {
            qreal lyrX        = lyr->bbox().x();
            qreal lyrXp       = lyr->pagePos().x();
            qreal lyrW        = lyr->bbox().width();
            qreal sysXp       = sys->pagePos().x();
            qreal offsetX     = lyrXp - sysXp + lyrX + lyrW - pos().x();
            //               syst.rel. X pos. | lyr.advance | as a delta from current pos.
            // add initial padding
            offsetX           += (isEndMelisma ? Lyrics::MELISMA_DEFAULT_PAD : Lyrics::LYRICS_DASH_DEFAULT_PAD) * sp;
            rxpos()           += offsetX;
            rxpos2()          -= offsetX;
            }

      // VERTICAL POSITION: at the base line of the syllable text
      if (spannerSegmentType() != SpannerSegmentType::END)
            rypos() = lyr->y();
      else {
            // use Y position of *next* syllable if there is one on same system
            Lyrics* nextLyr = searchNextLyrics(lyr->segment(), lyr->staffIdx(), lyr->no());
            if (nextLyr && nextLyr->segment()->system() == system())
                  rypos() = nextLyr->y();
            else
                  rypos() = lyr->y();
            }

      // MELISMA vs. DASHES
      if (isEndMelisma) {                 // melisma
            _numOfDashes = 1;
            rypos()  -= lyricsLine()->lineWidth().val() * sp * HALF; // let the line 'sit on' the base line
            qreal offsetX = score()->styleD(StyleIdx::minNoteDistance) * sp;
            // if final segment, extend slightly after the chord, otherwise shorten it
            rxpos2() +=
                  (spannerSegmentType() == SpannerSegmentType::BEGIN ||
                        spannerSegmentType() == SpannerSegmentType::MIDDLE)
                  ? -offsetX : +offsetX;

            }
      else {                              // dash(es)
#if defined(USE_FONT_DASH_METRIC)
            rypos()     += lyr->dashY();
            _dashLength = lyr->dashLength();
#else
            rypos()     -= lyr->bbox().height() * Lyrics::LYRICS_DASH_Y_POS_RATIO;    // set conventional dash Y pos
            _dashLength = Lyrics::LYRICS_DASH_DEFAULT_LENGHT * sp;                    // and dash length
#endif
            qreal len         = pos2().x();
            qreal minDashLen  = Lyrics::LYRICS_DASH_MIN_LENGTH * sp;
            if (len < minDashLen) {                                           // if no room for a dash
                  if (endOfSystem) {                                          //   if at end of system
                        rxpos2()          = minDashLen;                       //     draw minimal dash
                        _numOfDashes      = 1;
                        _dashLength       = minDashLen;
                        }
                  else                                                        //   if within system
                        _numOfDashes = 0;                                     //     draw no dash
                  }
            else if (len < (Lyrics::LYRICS_DASH_DEFAULT_STEP * TWICE * sp)) { // if no room for two dashes
                  _numOfDashes = 1;                                           //    draw one dash
                  if (_dashLength > len)                                      // if no room for a full dash
                        _dashLength = len;                                    //    shorten it
                  }
            else
                  _numOfDashes = len / (Lyrics::LYRICS_DASH_DEFAULT_STEP * sp);// draw several dashes
            }

      // set bounding box
      QRectF r = QRectF(0.0, 0.0, pos2().x(), pos2().y()).normalized();
      qreal lw = spatium() * lyricsLine()->lineWidth().val() * HALF;
      setbbox(r.adjusted(-lw, -lw, lw, lw));
      adjustReadPos();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void LyricsLineSegment::draw(QPainter* painter) const
      {
      if (_numOfDashes < 1)               // nothing to draw
            return;
      qreal _spatium = spatium();

      QPen pen(lyricsLine()->curColor());
      pen.setWidthF(lyricsLine()->lineWidth().val() * _spatium);
      pen.setCapStyle(Qt::FlatCap);
      painter->setPen(pen);
      if (lyricsLine()->lyrics()->ticks() > 0)           // melisma
            painter->drawLine(QPointF(), pos2());
      else {                                          // dash(es)
            qreal step  = pos2().x() / (_numOfDashes+1);
            qreal x     = step - _dashLength * HALF;
            for (int i = 0; i < _numOfDashes; i++, x += step)
                  painter->drawLine(QPointF(x, 0.0), QPointF(x + _dashLength, 0.0));
            }
      }

}

