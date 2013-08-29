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

#include "page.h"
#include "score.h"
#include "text.h"
#include "xml.h"
#include "measure.h"
#include "style.h"
#include "chord.h"
#include "beam.h"
#include "tuplet.h"
#include "note.h"
#include "barline.h"
#include "slur.h"
#include "hook.h"
#include "lyrics.h"
#include "bracket.h"
#include "line.h"
#include "staff.h"
#include "system.h"
#include "mscore.h"
#include "segment.h"

namespace Ms {

#define MM(x) ((x)/INCH)

const PaperSize paperSizes[] = {
      PaperSize("A4",        MM(210),  MM(297)),
      PaperSize("B5",        MM(176),  MM(250)),
      PaperSize("Letter",    8.5,      11),
      PaperSize("Legal",     8.5,      14),
      PaperSize("Executive", 7.5,      10),
      PaperSize("A0",        MM(841),  MM(1189)),
      PaperSize("A1",        MM(594),  MM(841)),
      PaperSize("A2",        MM(420),  MM(594)),
      PaperSize("A3",        MM(297),  MM(420)),
      PaperSize("A5",        MM(148),  MM(210)),
      PaperSize("A6",        MM(105),  MM(148)),
      PaperSize("A7",        MM(74),   MM(105)),
      PaperSize("A8",        MM(52),   MM(74)),
      PaperSize("A9",        MM(37),   MM(52)),
      PaperSize("B0",        MM(1000), MM(1414)),
      PaperSize("B1",        MM(707),  MM(1000)),
      PaperSize("B10",       MM(31),   MM(44)),
      PaperSize("B2",        MM(500),  MM(707)),
      PaperSize("B3",        MM(353),  MM(500)),
      PaperSize("B4",        MM(250),  MM(353)),
      PaperSize("B5",        MM(125),  MM(176)),
      PaperSize("B6",        MM(88),   MM(125)),
      PaperSize("B7",        MM(62),   MM(88)),
      PaperSize("B8",        MM(44),   MM(62)),
      PaperSize("B9",        MM(163),  MM(229)),
      PaperSize("Comm10E",   MM(105),  MM(241)),
      PaperSize("DLE",       MM(110),  MM(220)),
      PaperSize("Folio",     MM(210),  MM(330)),
      PaperSize("Ledger",    MM(432),  MM(279)),
      PaperSize("Tabloid",   MM(279),  MM(432)),
      PaperSize("Custom",    MM(1),    MM(1)),
      PaperSize(0,           MM(1),    MM(1))   // mark end of list
      };

//---------------------------------------------------------
//   getPaperSize
//---------------------------------------------------------

const PaperSize* getPaperSize(const QString& name)
      {
      for (int i = 0;;++i) {
            if (paperSizes[i].name == 0)
                  break;
            if (name == paperSizes[i].name)
                  return &paperSizes[i];
            }
      qDebug("unknown paper size");
      return &paperSizes[(sizeof paperSizes/sizeof(PaperSize)) - 2];
      }

//---------------------------------------------------------
//   paperSizeSizeToIndex
//---------------------------------------------------------

static const qreal minSize = 0.1;      // minimum paper size for sanity check
static const qreal maxError = 0.01;    // max allowed error when matching sizes

static qreal sizeError(const qreal si, const qreal sref)
      {
      qreal relErr = (si - sref) / sref;
      return relErr > 0 ? relErr : -relErr;
      }

//---------------------------------------------------------
//   getPaperSize
//---------------------------------------------------------

const PaperSize* getPaperSize(const qreal wi, const qreal hi)
      {
      if (wi < minSize || hi < minSize)
            return &paperSizes[0];
      for (int i = 0;;++i) {
            if (paperSizes[i].name == 0)
                  break;
            if (sizeError(wi, paperSizes[i].w) < maxError
               && sizeError(hi, paperSizes[i].h) < maxError)
                  return &paperSizes[i];
            if (sizeError(wi, paperSizes[i].h) < maxError
               && sizeError(hi, paperSizes[i].w) < maxError)
                  return &paperSizes[i];
            }
      qDebug("unknown paper size for %f x %f", wi, hi);
      //return custom
      return &paperSizes[(sizeof paperSizes/sizeof(PaperSize)) - 2];
      }

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

Page::Page(Score* s)
   : Element(s),
   _no(0)
      {
      bspTreeValid = false;
      }

Page::~Page()
      {
      }

//---------------------------------------------------------
//   items
//---------------------------------------------------------

QList<Element*> Page::items(const QRectF& r)
      {
#ifdef USE_BSP
      if (!bspTreeValid)
            doRebuildBspTree();
      QList<Element*> el = bspTree.items(r);
      return el;
#else
      return QList<Element*>();
#endif
      }

QList<Element*> Page::items(const QPointF& p)
      {
#ifdef USE_BSP
      if (!bspTreeValid)
            doRebuildBspTree();
      return bspTree.items(p);
#else
      return QList<Element*>();
#endif
      }

//---------------------------------------------------------
//   appendSystem
//---------------------------------------------------------

void Page::appendSystem(System* s)
      {
      s->setParent(this);
      _systems.append(s);
      }

//---------------------------------------------------------
//   setNo
//---------------------------------------------------------

void Page::setNo(int n)
      {
      _no = n;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Page::layout()
      {
      bbox().setRect(0.0, 0.0, score()->loWidth(), score()->loHeight());
      }

//---------------------------------------------------------
//   draw
//    bounding rectange fr is relative to page QPointF
//---------------------------------------------------------

void Page::draw(QPainter* painter) const
      {
      if (score()->layoutMode() != LayoutPage)
            return;
      //
      // draw header/footer
      //

      QTextDocument d;
      d.setDocumentMargin(0.0);
      d.setUseDesignMetrics(true);

      int n = no() + 1 + _score->pageNumberOffset();
      d.setTextWidth(score()->loWidth() - lm() - rm());

      QPointF o1(lm(), tm());

      painter->translate(o1);
      painter->setPen(curColor());

      QString s1, s2, s3;

      if (_score->styleB(ST_showHeader) && (no() || _score->styleB(ST_headerFirstPage))) {
            TextStyle ts = score()->textStyle(TEXT_STYLE_HEADER);
            QPointF o(ts.offset(spatium()));

            bool odd = (n & 1) && _score->styleB(ST_headerOddEven);
            if (odd) {
                  o.setX(-o.x());
                  s1 = _score->styleSt(ST_oddHeaderL);
                  s2 = _score->styleSt(ST_oddHeaderC);
                  s3 = _score->styleSt(ST_oddHeaderR);
                  }
            else {
                  s1 = _score->styleSt(ST_evenHeaderL);
                  s2 = _score->styleSt(ST_evenHeaderC);
                  s3 = _score->styleSt(ST_evenHeaderR);
                  }

            if (_score->styleB(ST_headerStyled)) {
                  drawStyledHeaderFooter(painter, 0, o, s1);
                  drawStyledHeaderFooter(painter, 1, o, s2);
                  drawStyledHeaderFooter(painter, 2, o, s3);
                  }
            else {
                  d.setDefaultFont(ts.font(1.0));
                  d.setTextWidth(_score->loWidth() - lm() - rm() - (2.0 * o.x()));
                  QAbstractTextDocumentLayout::PaintContext c;
                  c.cursorPosition = -1;
                  c.palette.setColor(QPalette::Text, ts.foregroundColor());
                  painter->translate(o);
                  QString s = _score->styleSt(odd ? ST_oddHeaderL : ST_evenHeaderL);
                  if (!s.isEmpty()) {
                        d.setHtml(replaceTextMacros(s));
                        d.documentLayout()->draw(painter, c);
                        }
                  s = replaceTextMacros(_score->styleSt(odd ? ST_oddHeaderC : ST_evenHeaderC));
                  if (!s.isEmpty()) {
                        d.setHtml(s);
                        d.documentLayout()->draw(painter, c);
                        }
                  s = replaceTextMacros(_score->styleSt(odd ? ST_oddHeaderR : ST_evenHeaderR));
                  if (!s.isEmpty()) {
                        d.setHtml(s);
                        d.documentLayout()->draw(painter, c);
                        }
                  painter->translate(-o);
                  }
            }

      if (_score->styleB(ST_showFooter) && (no() || _score->styleB(ST_footerFirstPage))) {
            TextStyle ts = score()->textStyle(TEXT_STYLE_FOOTER);

            QPointF o(ts.offset(spatium()));

            bool odd = (n & 1) && _score->styleB(ST_footerOddEven);

            if (odd) {
                  o.setX(-o.x());
                  s1 = _score->styleSt(ST_oddFooterL);
                  s2 = _score->styleSt(ST_oddFooterC);
                  s3 = _score->styleSt(ST_oddFooterR);
                  }
            else {
                  s1 = _score->styleSt(ST_evenFooterL);
                  s2 = _score->styleSt(ST_evenFooterC);
                  s3 = _score->styleSt(ST_evenFooterR);
                  }

            if (_score->styleB(ST_footerStyled)) {
                  drawStyledHeaderFooter(painter, 3, o, s1);
                  drawStyledHeaderFooter(painter, 4, o, s2);
                  drawStyledHeaderFooter(painter, 5, o, s3);
                  }
            else {
                  qreal w = _score->loWidth() - lm() - rm() - (2.0 * o.x());
                  o = QPointF(0.0, _score->loHeight() - (tm() + bm()));
                  QAbstractTextDocumentLayout::PaintContext c;
                  c.cursorPosition = -1;
                  c.palette.setColor(QPalette::Text, ts.foregroundColor());

                  painter->translate(o);
                  qreal h1, h2, h3;
                  QTextDocument d1, d2, d3;
                  QFont f;
                  if (!s1.isEmpty()) {
                        d1.setDocumentMargin(0.0);
                        d1.setUseDesignMetrics(true);
                        s1 = replaceTextMacros(s1);
                        d1.setTextWidth(w);
                        d1.setHtml(s1);
                        h1 = d1.documentLayout()->documentSize().height();
                        }
                  else
                        h1 = 0.0;
                  if (!s2.isEmpty()) {
                        d2.setDocumentMargin(0.0);
                        d2.setUseDesignMetrics(true);
                        s2 = replaceTextMacros(s2);
                        d2.setTextWidth(w);
                        d2.setHtml(s2);
                        h2 = d2.documentLayout()->documentSize().height();
                        }
                  else
                        h2 = 0.0;
                  if (!s3.isEmpty()) {
                        d3.setDocumentMargin(0.0);
                        d3.setUseDesignMetrics(true);
                        s3 = replaceTextMacros(s3);
                        d3.setTextWidth(w);
                        d3.setHtml(s3);
                        h3 = d3.documentLayout()->documentSize().height();
                        }
                  else
                        h3 = 0.0;
                  qreal h = qMax(h1, h2);
                  h       = qMax(h, h3);

                  QPointF pos(0.0, -h);
                  painter->translate(pos);
                  if (!s1.isEmpty())
                        d1.documentLayout()->draw(painter, c);
                  if (!s2.isEmpty())
                        d2.documentLayout()->draw(painter, c);
                  if (!s3.isEmpty())
                        d3.documentLayout()->draw(painter, c);
                  painter->translate(-(o + pos));
                  }
            }
      painter->translate(-o1);
      }

//---------------------------------------------------------
//   drawStyledFooter
//---------------------------------------------------------

void Page::drawStyledHeaderFooter(QPainter* p, int area, const QPointF& pt,
   const QString& ss) const
      {
      QString s = replaceTextMacros(ss);
      if (s.isEmpty())
            return;
      int textStyle = TEXT_STYLE_FOOTER;
      int flags = Qt::TextDontClip;
      switch(area) {
            case 0:
                  flags |= Qt::AlignLeft | Qt::AlignTop;
                  textStyle = TEXT_STYLE_HEADER;
                  break;
            case 1:
                  flags |= Qt::AlignHCenter | Qt::AlignTop;
                  textStyle = TEXT_STYLE_HEADER;
                  break;
            case 2:
                  flags |= Qt::AlignRight | Qt::AlignTop;
                  textStyle = TEXT_STYLE_HEADER;
                  break;
            case 3:
                  flags |= Qt::AlignLeft | Qt::AlignBottom;
                  break;
            case 4:
                  flags |= Qt::AlignHCenter | Qt::AlignBottom;
                  break;
            case 5:
                  flags |= Qt::AlignRight | Qt::AlignBottom;
                  break;
            }
      TextStyle ts = score()->textStyle(textStyle);
      p->setFont(ts.fontPx(spatium()));
      QRectF r(pt.x(), pt.y(), width() - lm() - rm(), height() - tm() - bm());
      p->drawText(r, flags, s);
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Page::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      foreach(System* s, _systems)
            s->scanElements(data, func, all);
      func(data, this);
      }

//---------------------------------------------------------
//   PageFormat
//---------------------------------------------------------

PageFormat::PageFormat()
      {
      _size             = QSizeF(210.0/INCH, 297.0/INCH); // A4
      _evenLeftMargin   = 10.0 / INCH;
      _oddLeftMargin    = 10.0 / INCH;
      _printableWidth   = _size.width() - 20.0 / INCH;
      _evenTopMargin    = 10.0 / INCH;
      _evenBottomMargin = 20.0 / INCH;
      _oddTopMargin     = 10.0 / INCH;
      _oddBottomMargin  = 20.0 / INCH;
      _twosided         = true;
      }

//---------------------------------------------------------
//   copy
//---------------------------------------------------------

void PageFormat::copy(const PageFormat& p)
      {
      _size               = p._size;
      _printableWidth     = p._printableWidth;
      _evenLeftMargin     = p._evenLeftMargin;
      _oddLeftMargin      = p._oddLeftMargin;
      _evenTopMargin      = p._evenTopMargin;
      _evenBottomMargin   = p._evenBottomMargin;
      _oddTopMargin       = p._oddTopMargin;
      _oddBottomMargin    = p._oddBottomMargin;
      _twosided           = p._twosided;
      }

//---------------------------------------------------------
//   setSize
//---------------------------------------------------------

void PageFormat::setSize(const PaperSize* size)
      {
      _size = QSizeF(size->w, size->h);
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString PageFormat::name() const
      {
      return paperSize()->name;
      }

//---------------------------------------------------------
//   read
//  <page-layout>
//	  <page-height>
//	  <page-width>
//      <pageFormat>A6</pageFormat>
//      <landscape>1</landscape>
//      <page-margins>
//         <left-margin>28.3465</left-margin>
//         <right-margin>28.3465</right-margin>
//         <top-margin>28.3465</top-margin>
//         <bottom-margin>56.6929</bottom-margin>
//         </page-margins>
//      </page-layout>
//---------------------------------------------------------

void PageFormat::read(XmlReader& e, Score* score)
      {
      qreal _oddRightMargin  = 0.0;
      qreal _evenRightMargin = 0.0;
      bool landscape = false;
      QString type;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "pageFormat")            // obsolete
                  setSize(getPaperSize(e.readElementText()));
            else if (tag == "landscape")        // obsolete
                  landscape = e.readInt();
            else if (tag == "page-margins") {
                  type = e.attribute("type","both");
                  qreal lm = 0.0, rm = 0.0, tm = 0.0, bm = 0.0;
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        qreal val = e.readDouble() * 0.5 / PPI;
                        if (tag == "left-margin")
                              lm = val;
                        else if (tag == "right-margin")
                              rm = val;
                        else if (tag == "top-margin")
                              tm = val;
                        else if (tag == "bottom-margin")
                              bm = val;
                        else
                              e.unknown();
                        }
                  _twosided = type == "odd" || type == "even";
                  if (type == "odd" || type == "both") {
                        _oddLeftMargin   = lm;
                        _oddRightMargin  = rm;
                        _oddTopMargin    = tm;
                        _oddBottomMargin = bm;
                        }
                  if (type == "even" || type == "both") {
                        _evenLeftMargin   = lm;
                        _evenRightMargin  = rm;
                        _evenTopMargin    = tm;
                        _evenBottomMargin = bm;
                        }
                  }
            else if (tag == "page-height")
                  _size.rheight() = e.readDouble() * 0.5 / PPI;
            else if (tag == "page-width")
                  _size.rwidth() = e.readDouble() * .5 / PPI;
            else if (tag == "page-offset") {           // obsolete, moved to Score
                  QString val(e.readElementText());
                  if(score)
                        score->setPageNumberOffset(val.toInt());
                  }
            else
                  e.unknown();
            }
      if (landscape)
            _size.transpose();
      qreal w1 = _size.width() - _oddLeftMargin - _oddRightMargin;
      qreal w2 = _size.width() - _evenLeftMargin - _evenRightMargin;
      _printableWidth = qMax(w1, w2);     // silently adjust right margins
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void PageFormat::write(Xml& xml) const
      {
      xml.stag("page-layout");

      // convert inch to 1/10 spatium units
      // 20 - font design size in point
      // SPATIUM = 20/4
      // qreal t = 10 * PPI / (20 / 4);
      qreal t = 2 * PPI;

      xml.tag("page-height", _size.height() * t);
      xml.tag("page-width",  _size.width() * t);

      const char* type = "both";
      if (_twosided) {
            type = "even";
            xml.stag(QString("page-margins type=\"%1\"").arg(type));
            xml.tag("left-margin",   evenLeftMargin() * t);
            xml.tag("right-margin",  evenRightMargin() * t);
            xml.tag("top-margin",    evenTopMargin() * t);
            xml.tag("bottom-margin", evenBottomMargin() * t);
            xml.etag();
            type = "odd";
            }
      xml.stag(QString("page-margins type=\"%1\"").arg(type));
      xml.tag("left-margin",   oddLeftMargin() * t);
      xml.tag("right-margin",  oddRightMargin() * t);
      xml.tag("top-margin",    oddTopMargin() * t);
      xml.tag("bottom-margin", oddBottomMargin() * t);
      xml.etag();

      xml.etag();
      }

//---------------------------------------------------------
//   doRebuildBspTree
//---------------------------------------------------------

#ifdef USE_BSP
void Page::doRebuildBspTree()
      {
      QList<Element*> el;
      foreach(System* s, _systems) {
            foreach(MeasureBase* m, s->measures()) {
                  m->scanElements(&el, collectElements, false);
                  }
            }
      scanElements(&el, collectElements, false);

      int n = el.size();
      if (score()->layoutMode() == LayoutLine) {
            qreal h = _systems.front()->height();
            MeasureBase* mb = _systems.front()->measures().back();
            qreal w = mb->x() + mb->width();
            bspTree.initialize(QRectF(0.0, 0.0, w, h), n);
            }
      else
            bspTree.initialize(abbox(), n);
      for (int i = 0; i < n; ++i)
            bspTree.insert(el.at(i));
      bspTreeValid = true;
      }
#endif

//---------------------------------------------------------
//   replaceTextMacros
//    $p          - page number
//    $$          - $
//    $n          - number of pages
//    $f          - file name
//    $F          - file path+name
//    $d          - current date
//    $D          - creation date
//    $:tag:      - meta data tag
//       already defined tags:
//       movementNumber
//       movementTitle
//       workNumber
//       workTitle
//       source
//       copyright
//---------------------------------------------------------

QString Page::replaceTextMacros(const QString& s) const
      {
      int pageno = no() + 1 + _score->pageNumberOffset();
      QString d;
      int n = s.size();
      for (int i = 0; i < n; ++i) {
            QChar c = s[i];
            if (c == '$' && (i < (n-1))) {
                  QChar c = s[i+1];
                  switch(c.toLatin1()) {
                        case 'p':
                              d += QString("%1").arg(pageno);
                              break;
                        case 'n':
                              d += QString("%1").arg(_score->pages().size() + _score->pageNumberOffset());
                              break;
                        case 'f':
                              d += _score->name();
                              break;
                        case 'F':
                              d += _score->absoluteFilePath();
                              break;
                        case 'd':
                              d += QDate::currentDate().toString(Qt::DefaultLocaleShortDate);
                              break;
                        case 'D':
                              {
                              QString creationDate = score()->metaTag("creationDate");
                              if(!creationDate.isNull()) {
                                    d += QDate::fromString(creationDate, Qt::ISODate).toString(Qt::DefaultLocaleShortDate);
                                    }
                              }
                              break;
                        case '$':
                              d += '$';
                              break;
                        case ':':
                              {
                              QString tag;
                              int k = i+2;
                              for (; k < n; ++k) {
                                    if (s[k].toLatin1() == ':')
                                          break;
                                    tag += s[k];
                                    }
                              if (k != n) {       // found ':' ?
                                    d += score()->metaTag(tag);
                                    i = k-1;
                                    }
                              }
                              break;
                        default:
                              d += '$';
                              d += c;
                              break;
                        }
                  ++i;
                  }
            else
                  d += c;
            }
      return d;
      }

//---------------------------------------------------------
//   isOdd
//---------------------------------------------------------

bool Page::isOdd() const
      {
      return (_no + 1 + score()->pageNumberOffset()) & 1;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Page::write(Xml& xml) const
      {
      xml.stag("Page");
      foreach(System* system, _systems) {
            system->write(xml);
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Page::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "System") {
                  System* system = new System(score());
                  score()->systems()->append(system);
                  system->read(e);
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   searchSystem
//    return list of systems as there may be more than
//    one system in a row
//    p is in canvas coordinates
//---------------------------------------------------------

QList<System*> Page::searchSystem(const QPointF& pos) const
      {
      QList<System*> systems;
      qreal y = pos.y();  // transform to page relative
      qreal y2;
      int n = _systems.size();
      for (int i = 0; i < n; ++i) {
            System* s = _systems.at(i);
            System* ns = 0;               // next system row
            int ii = i + 1;
            for (; ii < n; ++ii) {
                  ns = _systems.at(ii);
                  if (ns->y() != s->y())
                        break;
                  }
            if ((ii == n) || (ns == 0))
                  y2 = height();
            else  {
                  qreal sy2 = s->y() + s->bbox().height();
                  y2         = sy2 + (ns->y() - sy2) * .5;
                  }
            if (y < y2) {
                  systems.append(s);
                  for (int ii = i+1; ii < n; ++ii) {
                        if (_systems.at(ii)->y() != s->y())
                              break;
                        systems.append(_systems.at(ii));
                        }
                  return systems;
                  }
            }
      return systems;
      }

//---------------------------------------------------------
//   searchMeasure
//    p is in canvas coordinates
//---------------------------------------------------------

Measure* Page::searchMeasure(const QPointF& p) const
      {
      QList<System*> systems = searchSystem(p);
      if (systems.isEmpty())
            return 0;

      foreach(System* system, systems) {
            qreal x = p.x() - system->pagePos().x();
            foreach(MeasureBase* mb, system->measures()) {
                  if (mb->type() != MEASURE)
                        continue;
                  if (x < (mb->x() + mb->bbox().width()))
                        return static_cast<Measure*>(mb);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   pos2measure
//---------------------------------------------------------

/**
 Return measure for canvas relative position \a p.
*/

MeasureBase* Page::pos2measure(const QPointF& p, int* rst, int* pitch,
   Segment** seg, QPointF* offset) const
      {
      Measure* m = searchMeasure(p);
      if (m == 0)
            return 0;

      System* s = m->system();
      qreal y   = p.y() - s->canvasPos().y();

      int i;
      for (i = 0; i < score()->nstaves();) {
            SysStaff* stff = s->staff(i);
            if (!stff->show() || !score()->staff(i)->show()) {
                  ++i;
                  continue;
                  }
            int ni = i;
            for (;;) {
                  ++ni;
                  if (ni == score()->nstaves() || (s->staff(ni)->show() && score()->staff(ni)->show()))
                        break;
                  }

            qreal sy2;
            if (ni != score()->nstaves()) {
                  SysStaff* nstaff = s->staff(ni);
                  qreal s1y2 = stff->bbox().y() + stff->bbox().height();
                  sy2 = s1y2 + (nstaff->bbox().y() - s1y2)/2;
                  }
            else
                  sy2 = s->page()->height() - s->pos().y();   // s->height();
            if (y > sy2) {
                  i   = ni;
                  continue;
                  }
            break;
            }

      // search for segment + offset
      QPointF pppp = p - m->pagePos();
      int track    = i * VOICES;

      SysStaff* sstaff = m->system()->staff(i);
      for (Segment* segment = m->first(Segment::SegChordRest); segment; segment = segment->next(Segment::SegChordRest)) {
            if ((segment->element(track) == 0)
               && (segment->element(track+1) == 0)
               && (segment->element(track+2) == 0)
               && (segment->element(track+3) == 0)
               )
                  continue;
            Segment* ns = segment->next();
            for (; ns; ns = ns->next()) {
                  if (ns->segmentType() != Segment::SegChordRest)
                        continue;
                  if (ns->element(track)
                     || ns->element(track+1)
                     || ns->element(track+2)
                     || ns->element(track+3))
                        break;
                  }
            if (!ns || (pppp.x() < (segment->x() + (ns->x() - segment->x())* .5))) {
                  *rst = i;
                  if (pitch) {
//                        Staff* s = score()->staff(i);
//                        int clef = s->clef(segment->tick());
//TODO                        *pitch = score()->y2pitch(pppp.y() - sstaff->bbox().y(), clef, s->spatium());
                        }
                  if (offset)
                        *offset = pppp - QPointF(segment->x(), sstaff->bbox().y());
                  if (seg)
                        *seg = segment;
                  return m;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   elements
//---------------------------------------------------------

QList<const Element*> Page::elements()
      {
      QList<const Element*> el;
      foreach (System* s, _systems) {
            foreach(MeasureBase* m, s->measures())
                  m->scanElements(&el, collectElements, false);
            }
      scanElements(&el, collectElements, false);
      return el;
      }

//---------------------------------------------------------
//   tm
//---------------------------------------------------------

qreal Page::tm() const
      {
      const PageFormat* pf = _score->pageFormat();
      return ((!pf->twosided() || isOdd()) ? pf->oddTopMargin() : pf->evenTopMargin()) * MScore::DPI;
      }

//---------------------------------------------------------
//   bm
//---------------------------------------------------------

qreal Page::bm() const
      {
      const PageFormat* pf = _score->pageFormat();
      return ((!pf->twosided() || isOdd()) ? pf->oddBottomMargin() : pf->evenBottomMargin()) * MScore::DPI;
      }

//---------------------------------------------------------
//   lm
//---------------------------------------------------------

qreal Page::lm() const
      {
      const PageFormat* pf = _score->pageFormat();
      return ((!pf->twosided() || isOdd()) ? pf->oddLeftMargin() : pf->evenLeftMargin()) * MScore::DPI;
      }

//---------------------------------------------------------
//   rm
//---------------------------------------------------------

qreal Page::rm() const
      {
      const PageFormat* pf = _score->pageFormat();
      return ((!pf->twosided() || isOdd()) ? pf->oddRightMargin() : pf->evenRightMargin()) * MScore::DPI;
      }

}

