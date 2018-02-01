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

namespace Ms {

// some useful values:
static const qreal HALF  = 0.5;
static const qreal TWICE = 2.0;

//---------------------------------------------------------
//   searchNextLyrics
//---------------------------------------------------------

static Lyrics* searchNextLyrics(Segment* s, int staffIdx, int verse, Placement p)
      {
      Lyrics* l = 0;
      while ((s = s->next1(SegmentType::ChordRest))) {
            int strack = staffIdx * VOICES;
            int etrack = strack + VOICES;
            // search through all tracks of current staff looking for a lyric in specified verse
            for (int track = strack; track < etrack; ++track) {
                  ChordRest* cr = toChordRest(s->element(track));
                  if (cr) {
                        // cr with lyrics found, but does it have a syllable in specified verse?
                        l = cr->lyrics(verse, p);
                        if (l)
                              break;
                        }
                  }
            if (l)
                  break;
            }
      return l;
      }

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

Lyrics::Lyrics(Score* s)
   : TextBase(s)
      {
      init(SubStyle::LYRIC1);
      _no         = 0;
      _ticks      = 0;
      _syllabic   = Syllabic::SINGLE;
      _separator  = 0;
      placementStyle = PropertyFlags::STYLED;
      setPlacement(Placement(s->styleI(StyleIdx::lyricsPlacement)));
      }

Lyrics::Lyrics(const Lyrics& l)
   : TextBase(l)
      {
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
      xml.stag("Lyrics");
      writeProperty(xml, P_ID::VERSE);
      if (_syllabic != Syllabic::SINGLE) {
            static const char* sl[] = {
                  "single", "begin", "end", "middle"
                  };
            xml.tag("syllabic", sl[int(_syllabic)]);
            }
      writeProperty(xml, P_ID::LYRIC_TICKS);

      TextBase::writeProperties(xml);
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
                  _verseNumber = new Text(score());
                  _verseNumber->read(e);
                  _verseNumber->setParent(this);
                  }
            else if (tag == "placement") {
                  placementStyle = PropertyFlags::UNSTYLED;
                  TextBase::readProperties(e);
                  }
            else if (!TextBase::readProperties(e))
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
            if (_separator != nullptr && el == _separator) {
                  // Lyrics::remove() and LyricsLine::removeUnmanaged() call each other;
                  // be sure each finds a clean context
                  LyricsLine* separ = _separator;
                  _separator = 0;
                  separ->setParent(0);
                  separ->removeUnmanaged();
                  delete separ;
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
            Segment* s     = cr->segment()->next1();
            ChordRest* ncr = s ? s->nextChordRest(cr->track()) : 0;
            if (ncr && !ncr->lyrics(_no, placement()))
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
      layout1();
      adjustReadPos();
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
      setPos(QPointF());
      TextBase::layout1();
      if (!parent()) // palette & clone trick
          return;

      qreal lh = lineSpacing() * score()->styleD(StyleIdx::lyricsLineHeight);
      qreal y = 0;

      if (placeBelow())
//            y  = lh * (_no+1) + score()->styleP(StyleIdx::lyricsPosBelow) + staff()->height();
            y  = lh * _no + score()->styleP(StyleIdx::lyricsPosBelow) + staff()->height();
      else {
            // we are counting _no from bottom to top for verses above
            y = -lh * _no + score()->styleP(StyleIdx::lyricsPosAbove);
            }

      qreal x  = 0.0;

      //
      // parse leading verse number and/or punctuation, so we can factor it into layout separately
      //
      bool hasNumber     = false; // _verseNumber;
      qreal centerAdjust = 0.0;
      qreal leftAdjust   = 0.0;
      QString s          = plainText();

      // find:
      // 1) string of numbers and non-word characters at start of syllable
      // 2) at least one other character (indicating start of actual lyric)
      // 3) string of non-word characters at end of syllable
      //QRegularExpression leadingPattern("(^[\\d\\W]+)([^\\d\\W]+)");

      if (score()->styleB(StyleIdx::lyricsAlignVerseNumber)) {
            QRegularExpression punctuationPattern("(^[\\d\\W]*)([^\\d\\W].*?)([\\d\\W]*$)", QRegularExpression::UseUnicodePropertiesOption);
            QRegularExpressionMatch punctuationMatch = punctuationPattern.match(s);
            if (punctuationMatch.hasMatch()) {
#if 0 // TODO::ws
                  // leading and trailing punctuation
                  QString lp = punctuationMatch.captured(1);
                  QString tp = punctuationMatch.captured(3);
                  // actual lyric
                  //QString actualLyric = punctuationMatch.captured(2);
                  Text leading(*this);
                  leading.setPlainText(lp);
                  leading.layout1();
                  Text trailing(*this);
                  trailing.setPlainText(tp);
                  trailing.layout1();
                  leftAdjust = leading.width();
                  centerAdjust = leading.width() - trailing.width();
                  if (!lp.isEmpty() && lp[0].isDigit())
                        hasNumber = true;
#endif
                  }
            }

      ChordRest* cr = chordRest();
      Align ta = align();
      if (ta & Align::HCENTER) {
            //
            // center under notehead, not origin
            // however, lyrics that are melismas or have verse numbers will be forced to left alignment
            // TODO: provide a way to disable the automatic left alignment
            //
            qreal nominalWidth = symWidth(SymId::noteheadBlack);
            if (!isMelisma() && !hasNumber)     // center under notehead
                  x += nominalWidth * .5 - cr->x() - centerAdjust * 0.5;
            else                                // force left alignment
                  x += width() * .5 - cr->x() - leftAdjust;
            }
      else if (!(ta & Align::RIGHT)) {
            // even for left aligned syllables, ignore leading verse numbers and/or punctuation
            x -= leftAdjust;
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
      else {
            if (_separator) {
                  _separator->removeUnmanaged();
                  delete _separator;
                  _separator = nullptr;
                  }
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
      return data.element->isText() || TextBase::acceptDrop(data);
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Lyrics::drop(EditData& data)
      {
      ElementType type = data.element->type();
      if (type == ElementType::SYMBOL || type == ElementType::FSYMBOL) {
            TextBase::drop(data);
            return 0;
            }
      if (!data.element->isText()) {
            delete data.element;
            return 0;
            }
      Text* e = toText(data.element);
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
      if (type() == ElementType::LYRICS) {
            if ((_no & 1) && subStyle() == SubStyle::LYRIC1)
                  initSubStyle(SubStyle::LYRIC2);
            if (!(_no & 1) && subStyle() == SubStyle::LYRIC2)
                  initSubStyle(SubStyle::LYRIC1);
            }
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
            case P_ID::VERSE:
                  return _no;
            default:
                  return TextBase::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Lyrics::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::PLACEMENT:
                  placementStyle = PropertyFlags::UNSTYLED;
                  setPlacement(Placement(v.toInt()));
                  break;
            case P_ID::SYLLABIC:
                  _syllabic = Syllabic(v.toInt());
                  break;
            case P_ID::LYRIC_TICKS:
                  _ticks = v.toInt();
                  break;
            case P_ID::VERSE:
                  _no = v.toInt();
                  break;
            default:
                  if (!TextBase::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayout(tick());
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Lyrics::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::SUB_STYLE:
                  return int(SubStyle::LYRIC1);
            case P_ID::PLACEMENT:
                  return score()->styleI(StyleIdx::lyricsPlacement);
            case P_ID::SYLLABIC:
                  return int(Syllabic::SINGLE);
            case P_ID::LYRIC_TICKS:
            case P_ID::VERSE:
                  return 0;
            default: return TextBase::propertyDefault(id);
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
      _nextLyrics = 0;
      }

LyricsLine::LyricsLine(const LyricsLine& g)
   : SLine(g)
      {
      _nextLyrics = 0;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void LyricsLine::layout()
      {
      bool tempMelismaTicks = (lyrics()->ticks() == Lyrics::TEMP_MELISMA_TICKS);
      if (lyrics()->ticks()) {              // melisma
            setLineWidth(score()->styleS(StyleIdx::lyricsLineThickness));
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
                  s = s->nextCR(lyricsTrack, true);
            if (!s) {
                  // user probably deleted measures at end of score, leaving this melisma too long
                  // set s to last segment and reset lyricsEndTick to trigger FIXUP code below
                  s = score()->lastSegment();
                  lyricsEndTick = -1;
                  }
            Element* se = s->element(lyricsTrack);
            // everything is OK if we have reached a chord at right tick on right track
            if (s->tick() == lyricsEndTick && se && se->type() == ElementType::CHORD) {
                  // advance to next CR, or last segment if no next CR
                  s = s->nextCR(lyricsTrack, true);
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
                  Segment* ps = s->prev1(SegmentType::ChordRest);
                  while (ps && ps != lyricsSegment) {
                        Element* pe = ps->element(lyricsTrack);
                        // we're looking for an actual chord on this track
                        if (pe && pe->type() == ElementType::CHORD)
                              break;
                        s = ps;
                        ps = ps->prev1(SegmentType::ChordRest);
                        }
                  if (!ps || ps == lyricsSegment) {
                        // no valid previous CR, so try to lengthen melisma instead
                        ps = ns;
                        s = ps->nextCR(lyricsTrack, true);
                        Element* e = s ? s->element(lyricsTrack) : nullptr;
                        // check to make sure we have a chord
                        if (!e || e->type() != ElementType::CHORD) {
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
            _nextLyrics = searchNextLyrics(lyrics()->segment(), staffIdx(), lyrics()->no(), lyrics()->placement());
            setTick2(_nextLyrics ? _nextLyrics->segment()->tick() : tick());
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
//   removeUnmanaged
//    same as Spanner::removeUnmanaged(), but in addition, remove from hosting Lyrics
//---------------------------------------------------------

void LyricsLine::removeUnmanaged()
      {
      Spanner::removeUnmanaged();
      if (lyrics())
            lyrics()->remove(this);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool LyricsLine::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::SPANNER_TICKS:
                  {
                  // if parent lyrics has a melisma, change its length too
                  if (parent() && parent()->type() == ElementType::LYRICS
                              && toLyrics(parent())->ticks() > 0) {
                        int newTicks   = toLyrics(parent())->ticks() + v.toInt() - ticks();
                        parent()->undoChangeProperty(P_ID::LYRIC_TICKS, newTicks);
                        }
                  setTicks(v.toInt());
                  }
                  break;
            default:
                  if (!SLine::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll();
      return true;
      }

//=========================================================
//   LyricsLineSegment
//=========================================================

LyricsLineSegment::LyricsLineSegment(Score* s)
      : LineSegment(s)
      {
      setFlags(ElementFlag::SEGMENT | ElementFlag::ON_STAFF);
      clearFlags(ElementFlag::SELECTABLE | ElementFlag::MOVABLE);
      setGenerated(true);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void LyricsLineSegment::layout()
      {
      bool        endOfSystem       = false;
      bool        isEndMelisma      = lyricsLine()->lyrics()->ticks() > 0;
      Lyrics*     lyr               = 0;
      Lyrics*     nextLyr           = 0;
      qreal       fromX             = 0;
      qreal       toX               = 0;             // start and end point of intra-lyrics room
      qreal       sp                = spatium();
      System*     sys;

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
            lyr         = nextLyr = lyricsLine()->nextLyrics();
            sys         = lyr->segment()->system();
            endOfSystem = (sys != system());
            // if next lyrics is on a different sytem, this line segment is at the end of its system:
            // do not adjust for next lyrics position
            if (!endOfSystem) {
                  qreal lyrX        = lyr->bbox().x();
                  qreal lyrXp       = lyr->pagePos().x();
                  qreal sysXp       = sys->pagePos().x();
                  toX               = lyrXp - sysXp + lyrX;       // syst.rel. X pos.
                  qreal offsetX     = toX - pos().x() - pos2().x() - Lyrics::LYRICS_DASH_DEFAULT_PAD * sp;
                  //                    delta from current end pos.| ending padding
                  rxpos2()          += offsetX;
                  }
            }
      // B) if line follows a syllable, advance line start to after the syllable text
      lyr   = lyricsLine()->lyrics();
      sys   = lyr->segment()->system();
      if (sys && (spannerSegmentType() == SpannerSegmentType::BEGIN || spannerSegmentType() == SpannerSegmentType::SINGLE)) {
            qreal lyrX        = lyr->bbox().x();
            qreal lyrXp       = lyr->pagePos().x();
            qreal lyrW        = lyr->bbox().width();
            qreal sysXp       = sys->pagePos().x();
            fromX             = lyrXp - sysXp + lyrX + lyrW;
            //               syst.rel. X pos. | lyr.advance
            qreal offsetX     = fromX - pos().x() + (isEndMelisma ? Lyrics::MELISMA_DEFAULT_PAD : Lyrics::LYRICS_DASH_DEFAULT_PAD) * sp;
            //               delta from curr.pos. | add initial padding
            rxpos()           += offsetX;
            rxpos2()          -= offsetX;
            }

      // VERTICAL POSITION: at the base line of the syllable text
      if (spannerSegmentType() != SpannerSegmentType::END)
            rypos() = lyr->y();
      else {
            // use Y position of *next* syllable if there is one on same system
            Lyrics* nextLyr = searchNextLyrics(lyr->segment(), lyr->staffIdx(), lyr->no(), lyr->placement());
            if (nextLyr && nextLyr->segment()->system() == system())
                  rypos() = nextLyr->y();
            else
                  rypos() = lyr->y();
            }

      // MELISMA vs. DASHES
      if (isEndMelisma) {                 // melisma
            _numOfDashes = 1;
            rypos()  -= lyricsLine()->lineWidth().val() * sp * HALF; // let the line 'sit on' the base line
            qreal offsetX = score()->styleP(StyleIdx::minNoteDistance) * mag();
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
            // set conventional dash Y pos
            rypos() -= MScore::pixelRatio * lyr->fontMetrics().xHeight() * Lyrics::LYRICS_DASH_Y_POS_RATIO;
            _dashLength = score()->styleP(StyleIdx::lyricsDashMaxLength) * mag();  // and dash length
#endif
            qreal len         = pos2().x();
            qreal minDashLen  = score()->styleS(StyleIdx::lyricsDashMinLength).val() * sp;
            qreal maxDashDist = score()->styleS(StyleIdx::lyricsDashMaxDistance).val() * sp;
            if (len < minDashLen) {                                           // if no room for a dash
                  // if at end of system or dash is forced
                  if (endOfSystem || score()->styleB(StyleIdx::lyricsDashForce)) {
                        rxpos2()          = minDashLen;                       //     draw minimal dash
                        _numOfDashes      = 1;
                        _dashLength       = minDashLen;
                        }
                  else                                                        //   if within system or dash not forced
                        _numOfDashes = 0;                                     //     draw no dash
                  }
            else if (len < (maxDashDist * TWICE)) {                           // if no room for two dashes
                  _numOfDashes = 1;                                           //    draw one dash
                  if (_dashLength > len)                                      // if no room for a full dash
                        _dashLength = len;                                    //    shorten it
                  }
            else
                  _numOfDashes = len / (maxDashDist);                         // draw several dashes

            // adjust next lyrics horiz. position if too little a space forced to skip the dash
            if (_numOfDashes == 0 && nextLyr != nullptr && len > 0)
                  nextLyr->rxpos() -= (toX - fromX);
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

      QPen pen(lyricsLine()->lyrics()->curColor());
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

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyFlags& Lyrics::propertyFlags(P_ID id)
      {
      switch (id) {
            case P_ID::PLACEMENT:
                  return ScoreElement::propertyFlags(id);   // return PropertyFlags::NOSTYLE;

            default:
                  return TextBase::propertyFlags(id);
            }
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx Lyrics::getPropertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::PLACEMENT:
                  return StyleIdx::lyricsPlacement;
            case P_ID::FONT_FACE:
                  return isEven() ? StyleIdx::lyricsEvenFontFace : StyleIdx::lyricsOddFontFace;
            case P_ID::FONT_SIZE:
                  return isEven() ? StyleIdx::lyricsEvenFontSize : StyleIdx::lyricsOddFontSize;
            case P_ID::FONT_BOLD:
                  return isEven() ? StyleIdx::lyricsEvenFontBold : StyleIdx::lyricsOddFontBold;
            case P_ID::FONT_ITALIC:
                  return isEven() ? StyleIdx::lyricsEvenFontItalic : StyleIdx::lyricsOddFontItalic;
            case P_ID::FONT_UNDERLINE:
                  return isEven() ? StyleIdx::lyricsEvenFontUnderline : StyleIdx::lyricsOddFontUnderline;
            case P_ID::ALIGN:
                  return isEven() ? StyleIdx::lyricsEvenAlign : StyleIdx::lyricsOddAlign;
            default:
                  break;
            }
      return StyleIdx::NOSTYLE;
      }

//---------------------------------------------------------
//   styleChanged
//    reset all styled values to actual style
//---------------------------------------------------------

void Lyrics::styleChanged()
      {
      if (placementStyle == PropertyFlags::STYLED)
            setPlacement(Placement(score()->styleI(StyleIdx::lyricsPlacement)));
      TextBase::styleChanged();
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Lyrics::reset()
      {
      undoResetProperty(P_ID::PLACEMENT);
      TextBase::reset();
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Lyrics::resetProperty(P_ID id)
      {
      switch (id) {
            case P_ID::PLACEMENT:
                  setProperty(id, propertyDefault(id));
                  placementStyle = PropertyFlags::STYLED;
                  break;

            default:
                  return TextBase::resetProperty(id);
            }
      }

}

