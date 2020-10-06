//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "mscoreview.h"
#include "score.h"
#include "page.h"

namespace Ms {

//---------------------------------------------------------
//   elementLower
//---------------------------------------------------------

static bool elementLower(const Element* e1, const Element* e2)
      {
      if (!e1->selectable())
            return false;
      if (!e2->selectable())
            return true;
      return e1->z() < e2->z();
      }

//---------------------------------------------------------
//   elementAt
//---------------------------------------------------------

Element* MuseScoreView::elementAt(const QPointF& p)
      {
      QList<Element*> el = elementsAt(p);
#if 0
      qDebug("elementAt");
      for (const Element* e : el)
            qDebug("  %s %d", e->name(), e->selected());
#endif
      Element* e = el.value(0);
      if (e && e->isPage())
            e = el.value(1);
      return e;
      }

//---------------------------------------------------------
//   point2page
//---------------------------------------------------------

Page* MuseScoreView::point2page(const QPointF& p)
      {
      if (score()->layoutMode() == LayoutMode::LINE)
            return score()->pages().isEmpty() ? 0 : score()->pages().front();
      foreach(Page* page, score()->pages()) {
            if (page->bbox().translated(page->pos()).contains(p))
                  return page;
            }
      return 0;
      }

//---------------------------------------------------------
//   elementsAt
//    p is in canvas coordinates
//---------------------------------------------------------

const QList<Element*> MuseScoreView::elementsAt(const QPointF& p)
      {
      QList<Element*> el;

      Page* page = point2page(p);
      if (page) {
            el = page->items(p - page->pos());
            std::sort(el.begin(), el.end(), elementLower);
            }
      return el;
      }


}

