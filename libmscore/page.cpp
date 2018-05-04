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
#include "bracket.h"
#include "line.h"
#include "staff.h"
#include "system.h"
#include "mscore.h"
#include "segment.h"

namespace Ms {

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

Page::Page(Score* s)
   : Element(s),
   _no(0)
      {
      setFlags(0);
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
      Q_UNUSED(r)
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
      Q_UNUSED(p)
      return QList<Element*>();
#endif
      }

//---------------------------------------------------------
//   appendSystem
//--------e-------------------------------------------------

void Page::appendSystem(System* s)
      {
      s->setParent(this);
      _systems.append(s);
      }

//---------------------------------------------------------
//   draw
//    bounding rectangle fr is relative to page QPointF
//---------------------------------------------------------

void Page::draw(QPainter* painter) const
      {
      if (score()->layoutMode() != LayoutMode::PAGE)
            return;
      //
      // draw header/footer
      //

      int n = no() + 1 + score()->pageNumberOffset();
      painter->setPen(curColor());

      QString s1, s2, s3;

      if (score()->styleB(Sid::showHeader) && (no() || score()->styleB(Sid::headerFirstPage))) {
            bool odd = (n & 1) || !score()->styleB(Sid::headerOddEven);
            if (odd) {
                  s1 = score()->styleSt(Sid::oddHeaderL);
                  s2 = score()->styleSt(Sid::oddHeaderC);
                  s3 = score()->styleSt(Sid::oddHeaderR);
                  }
            else {
                  s1 = score()->styleSt(Sid::evenHeaderL);
                  s2 = score()->styleSt(Sid::evenHeaderC);
                  s3 = score()->styleSt(Sid::evenHeaderR);
                  }

            drawHeaderFooter(painter, 0, s1);
            drawHeaderFooter(painter, 1, s2);
            drawHeaderFooter(painter, 2, s3);
            }

      if (score()->styleB(Sid::showFooter) && (no() || score()->styleB(Sid::footerFirstPage))) {
            bool odd = (n & 1) || !score()->styleB(Sid::footerOddEven);
            if (odd) {
                  s1 = score()->styleSt(Sid::oddFooterL);
                  s2 = score()->styleSt(Sid::oddFooterC);
                  s3 = score()->styleSt(Sid::oddFooterR);
                  }
            else {
                  s1 = score()->styleSt(Sid::evenFooterL);
                  s2 = score()->styleSt(Sid::evenFooterC);
                  s3 = score()->styleSt(Sid::evenFooterR);
                  }

            drawHeaderFooter(painter, 3, s1);
            drawHeaderFooter(painter, 4, s2);
            drawHeaderFooter(painter, 5, s3);
            }
      }

//---------------------------------------------------------
//   drawHeaderFooter
//---------------------------------------------------------

void Page::drawHeaderFooter(QPainter* p, int area, const QString& ss) const
      {
      QString s = replaceTextMacros(ss);
      if (s.isEmpty())
            return;

      Text* text;
      if (area < 3) {
            text = score()->headerText();
            if (!text) {
                  text = new Text(SubStyleId::HEADER, score());
                  text->setLayoutToParentWidth(true);
                  score()->setHeaderText(text);
                  }
            }
      else {
            text = score()->footerText();
            if (!text) {
                  text = new Text(SubStyleId::FOOTER, score());
                  text->setLayoutToParentWidth(true);
                  score()->setFooterText(text);
                  }
            }
      text->setParent((Page*)this);
      Align flags = Align::LEFT;
      switch (area) {
            case 0: flags = Align::LEFT    | Align::TOP;    break;
            case 1: flags = Align::HCENTER | Align::TOP;    break;
            case 2: flags = Align::RIGHT   | Align::TOP;    break;
            case 3: flags = Align::LEFT    | Align::BOTTOM; break;
            case 4: flags = Align::HCENTER | Align::BOTTOM; break;
            case 5: flags = Align::RIGHT   | Align::BOTTOM; break;
            }
      text->setAlign(flags);
      text->setXmlText(s);
      text->layout();
      p->translate(text->pos());
      text->draw(p);
      p->translate(-text->pos());
      }

#if 0
//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void Page::styleChanged()
      {
      Text* t = score()->headerText();
      if (t)
            t->styleChanged();
      t = score()->footerText();
      if (t)
            t->styleChanged();
      }
#endif

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Page::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      for (System* s :_systems) {
            for (MeasureBase* m : s->measures())
                  m->scanElements(data, func, all);
            s->scanElements(data, func, all);
            }
      func(data, this);
      }

#ifdef USE_BSP
//---------------------------------------------------------
//   bspInsert
//---------------------------------------------------------

static void bspInsert(void* bspTree, Element* e)
      {
      ((BspTree*) bspTree)->insert(e);
      }

static void countElements(void* data, Element* /*e*/)
      {
      ++(*(int*)data);
      }

//---------------------------------------------------------
//   doRebuildBspTree
//---------------------------------------------------------

void Page::doRebuildBspTree()
      {
      int n = 0;
      scanElements(&n, countElements, false);

      QRectF r;
      if (score()->layoutMode() == LayoutMode::LINE) {
            qreal w = 0.0;
            qreal h = 0.0;
            if (!_systems.empty()) {
                  h = _systems.front()->height();
                  if (!_systems.front()->measures().empty()) {
                        MeasureBase* mb = _systems.front()->measures().back();
                        w = mb->x() + mb->width();
                        }
                  }
            r = QRectF(0.0, 0.0, w, h);
            }
      else
            r = abbox();

      bspTree.initialize(r, n);
      scanElements(&bspTree, &bspInsert, false);
      bspTreeValid = true;
      }
#endif

//---------------------------------------------------------
//   replaceTextMacros
//   (keep in sync with toolTipHeaderFooter in EditStyle::EditStyle())
//    $p          - page number, except on first page
//    $P          - page number, on all pages
//    $N          - page number, if there is more than one
//    $n          - number of pages
//    $i          - part name, except on first page
//    $I          - part name, on all pages
//    $f          - file name
//    $F          - file path+name
//    $d          - current date
//    $D          - creation date
//    $m          - last modification time
//    $M          - last modification date
//    $C          - copyright, on first page only
//    $c          - copyright, on all pages
//    $$          - the $ sign itself
//    $:tag:      - any metadata tag
//
//       tags already defined:
//       (see Score::init()))
//       copyright
//       creationDate
//       movementNumber
//       movementTitle
//       platform
//       source
//       workNumber
//       workTitle
//---------------------------------------------------------

QString Page::replaceTextMacros(const QString& s) const
      {
      QString d;
      for (int i = 0, n = s.size(); i < n; ++i) {
            QChar c = s[i];
            if (c == '$' && (i < (n-1))) {
                  QChar c = s[i+1];
                  switch(c.toLatin1()) {
                        case 'p': // not on first page 1
                              if (_no) // FALLTHROUGH
                        case 'N': // on page 1 only if there are multiple pages
                              if ( (score()->npages() + score()->pageNumberOffset()) > 1 ) // FALLTHROUGH
                        case 'P': // on all pages
                              {
                              int no = _no + 1 + score()->pageNumberOffset();
                              if (no > 0 )
                                    d += QString("%1").arg(no);
                              }
                              break;
                        case 'n':
                              d += QString("%1").arg(score()->npages() + score()->pageNumberOffset());
                              break;
                        case 'i': // not on first page
                              if (_no) // FALLTHROUGH
                        case 'I':
                              d += score()->metaTag("partName").toHtmlEscaped();
                              break;
                        case 'f':
                              d += masterScore()->fileInfo()->completeBaseName().toHtmlEscaped();
                              break;
                        case 'F':
                              d += masterScore()->fileInfo()->absoluteFilePath().toHtmlEscaped();
                              break;
                        case 'd':
                              d += QDate::currentDate().toString(Qt::DefaultLocaleShortDate);
                              break;
                        case 'D':
                              {
                              QString creationDate = score()->metaTag("creationDate");
                              if (creationDate.isNull())
                                    d += masterScore()->fileInfo()->created().date().toString(Qt::DefaultLocaleShortDate);
                              else
                                    d += QDate::fromString(creationDate, Qt::ISODate).toString(Qt::DefaultLocaleShortDate);
                              }
                              break;
                        case 'm':
                              if ( score()->dirty() )
                                    d += QTime::currentTime().toString(Qt::DefaultLocaleShortDate);
                              else
                                    d += masterScore()->fileInfo()->lastModified().time().toString(Qt::DefaultLocaleShortDate);
                              break;
                        case 'M':
                              if ( score()->dirty() )
                                    d += QDate::currentDate().toString(Qt::DefaultLocaleShortDate);
                              else
                                    d += masterScore()->fileInfo()->lastModified().date().toString(Qt::DefaultLocaleShortDate);
                              break;
                        case 'C': // only on first page
                              if (!_no) // FALLTHROUGH
                        case 'c':
                              d += score()->metaTag("copyright").toHtmlEscaped();
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
                                    d += score()->metaTag(tag).toHtmlEscaped();
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

void Page::write(XmlWriter& xml) const
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
                  score()->systems().push_back(system);
                  system->read(e);
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   elements
//---------------------------------------------------------

QList<Element*> Page::elements()
      {
      QList<Element*> el;
      scanElements(&el, collectElements, false);
      return el;
      }

//---------------------------------------------------------
//   tm
//---------------------------------------------------------

qreal Page::tm() const
      {
      return ((!score()->styleB(Sid::pageTwosided) || isOdd())
         ? score()->styleD(Sid::pageOddTopMargin) : score()->styleD(Sid::pageEvenTopMargin)) * DPI;
      }

//---------------------------------------------------------
//   bm
//---------------------------------------------------------

qreal Page::bm() const
      {
      return ((!score()->styleB(Sid::pageTwosided) || isOdd())
         ? score()->styleD(Sid::pageOddBottomMargin) : score()->styleD(Sid::pageEvenBottomMargin)) * DPI;
      }

//---------------------------------------------------------
//   lm
//---------------------------------------------------------

qreal Page::lm() const
      {
      return ((!score()->styleB(Sid::pageTwosided) || isOdd())
         ? score()->styleD(Sid::pageOddLeftMargin) : score()->styleD(Sid::pageEvenLeftMargin)) * DPI;
      }

//---------------------------------------------------------
//   rm
//---------------------------------------------------------

qreal Page::rm() const
      {
      return (score()->styleD(Sid::pageWidth) - score()->styleD(Sid::pagePrintableWidth)) * DPI - lm();
      }

//---------------------------------------------------------
//   tbbox
//    calculates and returns smallest rectangle containing all (visible) page elements
//---------------------------------------------------------

QRectF Page::tbbox()
      {
      qreal x1 = width();
      qreal x2 = 0.0;
      qreal y1 = height();
      qreal y2 = 0.0;
      const QList<Element*> el = elements();
      for (Element* e : el) {
            if (e == this || !e->isPrintable())
                  continue;
            QRectF ebbox = e->pageBoundingRect();
            if (ebbox.left() < x1)
                  x1 = ebbox.left();
            if (ebbox.right() > x2)
                  x2 = ebbox.right();
            if (ebbox.top() < y1)
                  y1 = ebbox.top();
            if (ebbox.bottom() > y2)
                  y2 = ebbox.bottom();
            }
      if (x1 < x2 && y1 < y2)
            return QRectF(x1, y1, x2 - x1, y2 - y1);
      else
            return abbox();
      }

//---------------------------------------------------------
//   endTick
//---------------------------------------------------------

int Page::endTick() const
      {
      return _systems.empty() ? -1 : _systems.back()->measures().back()->endTick();
      }
}

