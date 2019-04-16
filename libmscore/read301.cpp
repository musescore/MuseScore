//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "xml.h"
#include "score.h"
#include "staff.h"
#include "revisions.h"
#include "part.h"
#include "page.h"
#include "style.h"
#include "sym.h"
#include "audio.h"
#include "sig.h"
#include "barline.h"
#include "excerpt.h"
#include "spanner.h"

#ifdef OMR
#include "omr/omr.h"
#include "omr/omrpage.h"
#endif

namespace Ms {

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

bool Score::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            e.setTrack(-1);
            const QStringRef& tag(e.name());
            if (tag == "Staff")
                  readStaff(e);
            else if (tag == "Omr") {
#ifdef OMR
                  masterScore()->setOmr(new Omr(this));
                  masterScore()->omr()->read(e);
#else
                  e.skipCurrentElement();
#endif
                  }
            else if (tag == "Audio") {
                  _audio = new Audio;
                  _audio->read(e);
                  }
            else if (tag == "showOmr")
                  masterScore()->setShowOmr(e.readInt());
            else if (tag == "playMode")
                  _playMode = PlayMode(e.readInt());
            else if (tag == "LayerTag") {
                  int id = e.intAttribute("id");
                  const QString& t = e.attribute("tag");
                  QString val(e.readElementText());
                  if (id >= 0 && id < 32) {
                        _layerTags[id] = t;
                        _layerTagComments[id] = val;
                        }
                  }
            else if (tag == "Layer") {
                  Layer layer;
                  layer.name = e.attribute("name");
                  layer.tags = e.attribute("mask").toUInt();
                  _layer.append(layer);
                  e.readNext();
                  }
            else if (tag == "currentLayer")
                  _currentLayer = e.readInt();
            else if (tag == "Synthesizer")
                  _synthesizerState.read(e);
            else if (tag == "page-offset")
                  _pageNumberOffset = e.readInt();
            else if (tag == "Division")
                  _fileDivision = e.readInt();
            else if (tag == "showInvisible")
                  _showInvisible = e.readInt();
            else if (tag == "showUnprintable")
                  _showUnprintable = e.readInt();
            else if (tag == "showFrames")
                  _showFrames = e.readInt();
            else if (tag == "showMargins")
                  _showPageborders = e.readInt();
            else if (tag == "Style") {
                  qreal sp = style().value(Sid::spatium).toDouble();
                  style().load(e);
                  // if (_layoutMode == LayoutMode::FLOAT || _layoutMode == LayoutMode::SYSTEM) {
                  if (_layoutMode == LayoutMode::FLOAT) {
                        // style should not change spatium in
                        // float mode
                        style().set(Sid::spatium, sp);
                        }
                  _scoreFont = ScoreFont::fontFactory(style().value(Sid::MusicalSymbolFont).toString());
                  }
            else if (tag == "copyright" || tag == "rights") {
                  Text* text = new Text(this);
                  text->read(e);
                  setMetaTag("copyright", text->xmlText());
                  delete text;
                  }
            else if (tag == "movement-number")
                  setMetaTag("movementNumber", e.readElementText());
            else if (tag == "movement-title")
                  setMetaTag("movementTitle", e.readElementText());
            else if (tag == "work-number")
                  setMetaTag("workNumber", e.readElementText());
            else if (tag == "work-title")
                  setMetaTag("workTitle", e.readElementText());
            else if (tag == "source")
                  setMetaTag("source", e.readElementText());
            else if (tag == "metaTag") {
                  QString name = e.attribute("name");
                  setMetaTag(name, e.readElementText());
                  }
            else if (tag == "Part") {
                  Part* part = new Part(this);
                  part->read(e);
                  _parts.push_back(part);
                  }
            else if ((tag == "HairPin")
                || (tag == "Ottava")
                || (tag == "TextLine")
                || (tag == "Volta")
                || (tag == "Trill")
                || (tag == "Slur")
                || (tag == "Pedal")) {
                  Spanner* s = toSpanner(Element::name2Element(tag, this));
                  s->read(e);
                  addSpanner(s);
                  }
            else if (tag == "Excerpt") {
                  if (MScore::noExcerpts)
                        e.skipCurrentElement();
                  else {
                        if (isMaster()) {
                              Excerpt* ex = new Excerpt(static_cast<MasterScore*>(this));
                              ex->read(e);
                              excerpts().append(ex);
                              }
                        else {
                              qDebug("Score::read(): part cannot have parts");
                              e.skipCurrentElement();
                              }
                        }
                  }
            else if (e.name() == "Tracklist") {
                  int strack = e.intAttribute("sTrack",   -1);
                  int dtrack = e.intAttribute("dstTrack", -1);
                  if (strack != -1 && dtrack != -1)
                        e.tracks().insert(strack, dtrack);
                  e.skipCurrentElement();
                  }
            else if (tag == "Score") {          // recursion
                  if (MScore::noExcerpts)
                        e.skipCurrentElement();
                  else {
                        e.tracks().clear();     // ???
                        MasterScore* m = masterScore();
                        Score* s       = new Score(m, MScore::baseStyle());
                        Excerpt* ex    = new Excerpt(m);

                        ex->setPartScore(s);
                        e.setLastMeasure(nullptr);
                        s->read(e);
                        ex->setTracks(e.tracks());
                        m->addExcerpt(ex);
                        }
                  }
            else if (tag == "name") {
                  QString n = e.readElementText();
                  if (!isMaster()) //ignore the name if it's not a child score
                        excerpt()->setTitle(n);
                  }
            else if (tag == "layoutMode") {
                  QString s = e.readElementText();
                  if (s == "line")
                        _layoutMode = LayoutMode::LINE;
                  else if (s == "system")
                        _layoutMode = LayoutMode::SYSTEM;
                  else
                        qDebug("layoutMode: %s", qPrintable(s));
                  }
            else
                  e.unknown();
            }
      style().toPageLayout301(); // completes the loading of styles
      e.reconnectBrokenConnectors();
      if (e.error() != QXmlStreamReader::NoError) {
            qDebug("%s: xml read error at line %lld col %lld: %s",
               qPrintable(e.getDocName()), e.lineNumber(), e.columnNumber(),
               e.name().toUtf8().data());
            MScore::lastError = QObject::tr("XML read error at line %1, column %2: %3").arg(e.lineNumber()).arg(e.columnNumber()).arg(e.name().toString());
            return false;
            }

      connectTies();

      _fileDivision = MScore::division;

#if 0 // TODO:barline
      //
      //    sanity check for barLineSpan
      //
      for (Staff* st : staves()) {
            int barLineSpan = st->barLineSpan();
            int idx = st->idx();
            int n   = nstaves();
            if (idx + barLineSpan > n) {
                  qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
                  // span until last staff
                  barLineSpan = n - idx;
                  st->setBarLineSpan(barLineSpan);
                  }
            else if (idx == 0 && barLineSpan == 0) {
                  qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
                  // span from the first staff until the start of the next span
                  barLineSpan = 1;
                  for (int i = 1; i < n; ++i) {
                        if (staff(i)->barLineSpan() == 0)
                              ++barLineSpan;
                        else
                              break;
                        }
                  st->setBarLineSpan(barLineSpan);
                  }
            // check spanFrom
            int minBarLineFrom = st->lines(0) == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : MIN_BARLINE_SPAN_FROMTO;
            if (st->barLineFrom() < minBarLineFrom)
                  st->setBarLineFrom(minBarLineFrom);
            if (st->barLineFrom() > st->lines(0) * 2)
                  st->setBarLineFrom(st->lines(0) * 2);
            // check spanTo
            Staff* stTo = st->barLineSpan() <= 1 ? st : staff(idx + st->barLineSpan() - 1);
            // 1-line staves have special bar line spans
            int maxBarLineTo        = stTo->lines(0) == 1 ? BARLINE_SPAN_1LINESTAFF_TO : stTo->lines(0) * 2;
            if (st->barLineTo() < MIN_BARLINE_SPAN_FROMTO)
                  st->setBarLineTo(MIN_BARLINE_SPAN_FROMTO);
            if (st->barLineTo() > maxBarLineTo)
                  st->setBarLineTo(maxBarLineTo);
            // on single staff span, check spanFrom and spanTo are distant enough
            if (st->barLineSpan() == 1) {
                  if (st->barLineTo() - st->barLineFrom() < MIN_BARLINE_FROMTO_DIST) {
                        st->setBarLineFrom(0);
                        st->setBarLineTo(0);
                        }
                  }
            }
#endif

      if (!masterScore()->omr())
            masterScore()->setShowOmr(false);

      fixTicks();
      masterScore()->rebuildMidiMapping();
      masterScore()->updateChannel();
//      createPlayEvents();
      return true;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool MasterScore::read(XmlReader& e)
      {
      if (!Score::read(e))
            return false;
      for (Staff* s : staves())
            s->updateOttava();
      setCreated(false);
      return true;
      }

//---------------------------------------------------------
//   addMovement
//---------------------------------------------------------

void MasterScore::addMovement(MasterScore* score)
      {
      score->_movements = _movements;
      _movements->push_back(score);
      MasterScore* ps = 0;
      for (MasterScore* s : *_movements) {
            s->setPrev(ps);
            if (ps)
                  ps->setNext(s);
            s->setNext(0);
            ps = s;
            }
      }

//---------------------------------------------------------
//   read301
//---------------------------------------------------------

Score::FileError MasterScore::read301(XmlReader& e)
      {
      bool top = true;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "programVersion") {
                  setMscoreVersion(e.readElementText());
                  parseVersion(mscoreVersion());
                  }
            else if (tag == "programRevision")
                  setMscoreRevision(e.readIntHex());
            else if (tag == "Score") {
                  MasterScore* score;
                  if (top) {
                        score = this;
                        top   = false;
                        }
                  else {
                        score = new MasterScore();
                        score->setMscVersion(mscVersion());
                        addMovement(score);
                        }
                  if (!score->read(e))
                        return FileError::FILE_BAD_FORMAT;
                  }
            else if (tag == "Revision") {
                  Revision* revision = new Revision;
                  revision->read(e);
                  revisions()->add(revision);
                  }
            }
      return FileError::FILE_NO_ERROR;
      }

//------------------------------------------------------------------------------
//   MStyle::spatium301() - For reading 3.01 and earlier file formats, where the
//                          spatium is stored in floating-point millimeters.
// These four hard-coded values are the only values that MuseScore ever stored
// for the default spatium value in millimeters:
//  1.76389 = SPATIUM20 in millimeters, rounded by Qt when converted to a string
//  1.764   = 1.76389 rounded to 3 decimals by pagesettings.ui
//  1.7526  = SPATIUM20 in inches, rounded to 3 decimals, and converted to mm
//  1.753   = 1.7526 rounded to 3 decimals by pagesettings.ui
// These four values do not convert cleanly to the default spatium value of
// SPATIUM20. This code sets their value to SPATIUM20, instead of converting.
//------------------------------------------------------------------------------

void MStyle::spatium301(XmlReader& e)
      {
      QString txt = e.readElementText();
      if (txt == "1.76389" || txt == "1.764")
            set(Sid::spatium, SPATIUM20); // force the default value
      else
            set(Sid::spatium, txt.toDouble() * DPMM);
      }

//------------------------------------------------------------------------------
//   MStyle::fuzzySet
//     Fuzzy match for page margins in Inches
//     If the difference between n and the current style value is greater than
//     or equal to epislon, then set the value. If the difference is less than
//     epsilon, leave the original value alone.
//------------------------------------------------------------------------------

void MStyle::fuzzySet(Sid sid, double val, double factor, double epsilon)
      {
      double n = val * factor;
      if (abs(value(sid).toDouble() - n) >= epsilon)
            set(sid, n);
      }

//------------------------------------------------------------------------------
//   MStyle::fuzzyDefault - should be named fuzzyBase(), but MStyle::isDefault()
//                          should be renamed MStyle::isBase() too.
//     Reverse fuzzy match, uses the baseStyle value if the current value is
//     close enough to that baseStyle value. Helpful for older files where
//     rounded values have been stored. Combined with MStyle::isDefault(), it
//     prevents near-baseStyle values from being stored in the file.
//------------------------------------------------------------------------------

double MStyle::fuzzyDefault(Sid sid, double epsilon)
      {
      double n = MScore::baseStyle().value(sid).toDouble();
      if (abs(value(sid).toDouble() - n) < epsilon)
            set(sid, n); // necessary for fuzzySet() in fromPageLayout()
      return value(sid).toDouble();
      }

//------------------------------------------------------------------------------
//   MStyle::pageSizeId301
//      - for file format version 3.01 and earlier
//      - returns a page size id based on a fuzzy match to the size argument
//      - MuseScore uses a subset of QPageSize::PageSizeId, not the full enum
//------------------------------------------------------------------------------
QPageSize::PageSizeId MStyle::pageSizeId301(QSizeF& size) const
      {
      QPageSize::PageSizeId psid = QPageSize::id(size,
                                                 QPageSize::Inch,
                                                 QPageSize::FuzzyOrientationMatch);
      int id = int(psid);
      if (psid != QPageSize::Custom &&
          MScore::sizesMetric.find(id)   == MScore::sizesMetric.end() &&
          MScore::sizesImperial.find(id) == MScore::sizesImperial.end() &&
          MScore::sizesOther.find(id)    == MScore::sizesOther.end())
            {
            psid = QPageSize::Custom;
            }
      return psid;
      }

//------------------------------------------------------------------------------
//   MStyle::toPageLayout301
//------------------------------------------------------------------------------

void MStyle::toPageLayout301()
      {
      double factor, w, h, p, left, right, top, bottom, eLeft;
      int idxUnit;
      bool isTwo, isLandscape;
      QMarginsF oddMarg, evenMarg;

      w = fuzzyDefault(Sid::pageWidth);
      h = fuzzyDefault(Sid::pageHeight);
      p = fuzzyDefault(Sid::pagePrintableWidth);

      idxUnit = MScore::unitsValue();
      factor  = pageUnits[idxUnit].inchFactor(); // convert inches to user units
      top     = fuzzyDefault(Sid::pageOddTopMargin) / factor;
      bottom  = fuzzyDefault(Sid::pageOddBottomMargin) / factor;
      left    = fuzzyDefault(Sid::pageOddLeftMargin) / factor;
      eLeft   = fuzzyDefault(Sid::pageEvenLeftMargin) / factor;
      isTwo   = value(Sid::pageTwosided).toBool();
      right   = isTwo ? eLeft : ((w - p) / factor) - eLeft;
      oddMarg = QMarginsF(left, top, right, bottom);
      top     = fuzzyDefault(Sid::pageEvenTopMargin) / factor;
      bottom  = fuzzyDefault(Sid::pageEvenBottomMargin) / factor;
      if (isTwo)
            right = left; // two-sided = mirror image margins
      evenMarg = QMarginsF(eLeft, top, right, bottom);

      QSizeF sz = QSizeF(w, h);
      QPageSize::PageSizeId psid = pageSizeId301(sz);
      if (psid != QPageSize::Custom) {
            QSizeF size = _pageSize.definitionSize();
            isLandscape = ((w > h) != (size.width() > size.height()));
            _pageSize   = QPageSize(psid);
            }
      else {
            isLandscape = false;
            _pageSize   = QPageSize(QSizeF(w / factor, h / factor),
                                    QPageSize::Unit(idxUnit),
                                    QPageSize::name(psid),
                                    QPageSize::ExactMatch);
            }
      _pageOdd = MPageLayout();
      _pageOdd.setPageSize(_pageSize);
      _pageOdd.setUnits(QPageLayout::Unit(idxUnit));
      _pageOdd.setOrientation(isLandscape ? QPageLayout::Landscape
                                          : QPageLayout::Portrait);
      _pageOdd.setMargins(oddMarg);
      if (MScore::testMode && _pageOdd.margins().left() == 0) { /// Travis workaround
            // _pageOdd.fullRect().width() and .height() are reliable. The style
            // margin values have not changed. The code can reliably recalculate
            // pagePrintableWidth with those values. This is for two mtests
            // files in compat114 and one in compat206. pagePrintableWidth is
            // incorrect because of zero margins, but only for these three files.
            w     = _pageOdd.fullRect().width() * factor;
            left  = value(Sid::pageOddLeftMargin).toDouble();
            eLeft = value(Sid::pageEvenLeftMargin).toDouble();
            right = isTwo ? eLeft : w - p - eLeft;
            fuzzySet(Sid::pagePrintableWidth, w - left - right);
            fuzzyDefault(Sid::pagePrintableWidth);

            qDebug("Odd Left Origin: %f", left); // these prove the Travis error
            qDebug("Odd Left Source: %f", oddMarg.left());
            qDebug("Odd Left Margin: %f", _pageOdd.margins().left());
            }
      _pageEven = MPageLayout(_pageOdd);
      _pageEven.setMargins(evenMarg);
      }

//------------------------------------------------------------------------------
//   MStyle::fromPageLayout301
//------------------------------------------------------------------------------

void MStyle::fromPageLayout301()
      {
      // QPageLayout rounds inches to 2 decimals, a coarse resolution.
      // pageUnit.inchFactor() creates unrounded values for file storage. Still,
      // there are issues comparing floats, so fuzzySet() avoids changing the
      // value if it's close enough.
      QRectF rect = _pageOdd.fullRect();
      double factor = pageUnits[int(_pageOdd.units())].inchFactor();
      QMarginsF marg = _pageOdd.margins();

      set(Sid::pageWidth, rect.width() * factor);
      set(Sid::pageHeight, rect.height() * factor);

      /// Travis workaround:
      /// Sometimes the QPageLayouts' margins are all zero values, cause unknown.
      /// This affects many mtests, and only happens on Travis.
      if (MScore::testMode && marg.left() == 0)
            return;

      double val = value(Sid::pageTwosided).toBool() ? marg.right() : marg.left();
      fuzzySet(Sid::pageEvenLeftMargin, val, factor);
      fuzzySet(Sid::pageOddLeftMargin, marg.left(), factor);
      fuzzySet(Sid::pageOddTopMargin, marg.top(), factor);
      fuzzySet(Sid::pageOddBottomMargin, marg.bottom(), factor);
      marg = _pageEven.margins();
      fuzzySet(Sid::pageEvenTopMargin, marg.top(), factor);
      fuzzySet(Sid::pageEvenBottomMargin, marg.bottom(), factor);
      fuzzySet(Sid::pagePrintableWidth, _pageOdd.paintRect().width(), factor);
      }
}
