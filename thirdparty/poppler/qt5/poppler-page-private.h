/* poppler-page.cc: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2007, 2012, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2008, Pino Toscano <pino@kde.org>
 * Copyright (C) 2015 Adam Reichold <adamreichold@myopera.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _POPPLER_PAGE_PRIVATE_H_
#define _POPPLER_PAGE_PRIVATE_H_

#include "CharTypes.h"

class QRectF;

class LinkAction;
class Page;
class TextPage;

namespace Poppler
{

class DocumentData;
class PageTransition;

class PageData {
public:
  Link* convertLinkActionToLink(::LinkAction * a, const QRectF &linkArea);

  DocumentData *parentDoc;
  ::Page *page;
  int index;
  PageTransition *transition;

  static Link* convertLinkActionToLink(::LinkAction * a, DocumentData *parentDoc, const QRectF &linkArea);
  
  TextPage *prepareTextSearch(const QString &text, Page::Rotation rotate, QVector<Unicode> *u);
  GBool performSingleTextSearch(TextPage* textPage, QVector<Unicode> &u, double &sLeft, double &sTop, double &sRight, double &sBottom, Page::SearchDirection direction, GBool sCase, GBool sWords);
  QList<QRectF> performMultipleTextSearch(TextPage* textPage, QVector<Unicode> &u, GBool sCase, GBool sWords);
};

}

#endif
