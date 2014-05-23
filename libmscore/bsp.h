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
//
//  This code is from Qt implementation of QGraphicsItem
//    Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
//=============================================================================

#ifndef __BSP_H__
#define __BSP_H__

namespace Ms {

class BspTreeVisitor;
class InsertItemBspTreeVisitor;
class RemoveItemBspTreeVisitor;
class FindItemBspTreeVisitor;

class Element;

//---------------------------------------------------------
//   BspTree
//    binary space partitioning
//---------------------------------------------------------

class BspTree
      {
   public:
      struct Node {
            enum class Type : char { HORIZONTAL, VERTICAL, LEAF };
            union {
                  qreal offset;
                  int leafIndex;
                  };
            Type type;
            };
   private:
      uint depth;
      void initialize(const QRectF& rect, int depth, int index);
      void climbTree(BspTreeVisitor* visitor, const QPointF& pos, int index = 0);
      void climbTree(BspTreeVisitor* visitor, const QRectF& rect, int index = 0);

      void findItems(QList<Element*>* foundItems, const QRectF& rect, int index);
      void findItems(QList<Element*>* foundItems, const QPointF& pos, int index);
      QRectF rectForIndex(int index) const;

      QVector<Node> nodes;
      QVector<QList<Element*> > leaves;
      int leafCnt;
      QRectF rect;

   public:
      BspTree();

      void initialize(const QRectF& rect, int depth);
      void clear();

      void insert(Element* item);
      void remove(Element* item);

      QList<Element*> items(const QRectF& rect);
      QList<Element*> items(const QPointF& pos);

      int leafCount() const                       { return leafCnt; }
      inline int firstChildIndex(int index) const { return index * 2 + 1; }

      inline int parentIndex(int index) const {
            return index > 0 ? ((index & 1) ? ((index - 1) / 2) : ((index - 2) / 2)) : -1;
            }
#ifndef NDEBUG
      QString debug(int index) const;
#endif
      };

//---------------------------------------------------------
//   BspTreeVisitor
//---------------------------------------------------------

class BspTreeVisitor
      {
   public:
      virtual ~BspTreeVisitor() {}
      virtual void visit(QList<Element*>* items) = 0;
      };

}     // namespace Ms
#endif
