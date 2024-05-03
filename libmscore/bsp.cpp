//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2007-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//
//  This code is from Qt implementation of QGraphicsItem
//    Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
//=============================================================================

#include "bsp.h"
#include "element.h"

namespace Ms {

//---------------------------------------------------------
//   InsertItemBspTreeVisitor
//---------------------------------------------------------

class InsertItemBspTreeVisitor : public BspTreeVisitor
      {
   public:
      Element* item;

      inline void visit(QList<Element*> *items) { items->prepend(item); }
      };

//---------------------------------------------------------
//   RemoveItemBspTreeVisitor
//---------------------------------------------------------

class RemoveItemBspTreeVisitor : public BspTreeVisitor
      {
   public:
      Element* item;

      inline void visit(QList<Element*> *items) { items->removeAll(item); }
      };

//---------------------------------------------------------
//   FindItemBspTreeVisitor
//---------------------------------------------------------

class FindItemBspTreeVisitor : public BspTreeVisitor
      {
   public:
      QList<Element*> foundItems;

      void visit(QList<Element*>* items) {
            for (int i = 0; i < items->size(); ++i) {
                  Element* item = items->at(i);
                  if (!item->itemDiscovered) {
                        item->itemDiscovered = true;
                        foundItems.prepend(item);
                        }
                  }
            }
      };

//---------------------------------------------------------
//   BspTree
//---------------------------------------------------------

BspTree::BspTree()
   : leafCnt(0)
      {
      depth = 0;
      }

//---------------------------------------------------------
//   intmaxlog
//---------------------------------------------------------

static inline int intmaxlog(int n)
      {
      return (n > 0 ? qMax(int(::ceil(::log(qreal(n))/::log(qreal(2)))), 5) : 0);
      }

//---------------------------------------------------------
//   initialize
//---------------------------------------------------------

void BspTree::initialize(const QRectF& rec, int n)
      {
      depth      = intmaxlog(n);
      this->rect = rec;
      leafCnt    = 0;

      nodes.resize((1 << (depth+1)) - 1);
      leaves.resize(1 << depth);
      leaves.fill(QList<Element*>());
      initialize(rec, depth, 0);
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void BspTree::clear()
      {
      leafCnt = 0;
      nodes.clear();
      leaves.clear();
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void BspTree::insert(Element* element)
      {
      InsertItemBspTreeVisitor insertVisitor;
      insertVisitor.item = element;
      climbTree(&insertVisitor, element->pageBoundingRect());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void BspTree::remove(Element* element)
      {
      RemoveItemBspTreeVisitor removeVisitor;
      removeVisitor.item = element;
      climbTree(&removeVisitor, element->pageBoundingRect());
      }

//---------------------------------------------------------
//   items
//---------------------------------------------------------

QList<Element*> BspTree::items(const QRectF& rec)
      {
      FindItemBspTreeVisitor findVisitor;
      climbTree(&findVisitor, rec);
      QList<Element*> l;
      for (Element * e : qAsConst(findVisitor.foundItems)) {
          e->itemDiscovered = false;
          if (e->pageBoundingRect().intersects(rec))
                l.append(e);
          }
          return l;
      }

//---------------------------------------------------------
//   items
//---------------------------------------------------------

QList<Element*> BspTree::items(const QPointF& pos)
      {
      FindItemBspTreeVisitor findVisitor;
      climbTree(&findVisitor, pos);

      QList<Element*> l;
      for (Element* e : qAsConst(findVisitor.foundItems)) {
            e->itemDiscovered = false;
            if (e->contains(pos))
                  l.append(e);
            }
      return l;
      }

#ifndef NDEBUG
//---------------------------------------------------------
//   debug
//---------------------------------------------------------

QString BspTree::debug(int index) const
      {
      const Node* node = &nodes.at(index);

      QString tmp;
      if (node->type == Node::Type::LEAF) {
            QRectF rec = rectForIndex(index);
            if (!leaves[node->leafIndex].empty()) {
                  tmp += QString::fromLatin1("[%1, %2, %3, %4] contains %5 items\n")
                   .arg(rec.left()).arg(rec.top())
                   .arg(rec.width()).arg(rec.height())
                   .arg(leaves[node->leafIndex].size());
                  }
            }
      else {
            if (node->type == Node::Type::HORIZONTAL) {
                  tmp += debug(firstChildIndex(index));
                  tmp += debug(firstChildIndex(index) + 1);
                  }
            else {
                  tmp += debug(firstChildIndex(index));
                  tmp += debug(firstChildIndex(index) + 1);
                  }
            }
      return tmp;
      }
#endif

//---------------------------------------------------------
//   initialize
//---------------------------------------------------------

void BspTree::initialize(const QRectF& rec, int dep, int index)
      {
      Node* node = &nodes[index];
      if (index == 0) {
            node->type = Node::Type::HORIZONTAL;
            node->offset = rec.center().y();
            }

      if (dep) {
            Node::Type type;
            QRectF rect1, rect2;
            qreal offset1, offset2;

            if (node->type == Node::Type::HORIZONTAL) {
                  type = Node::Type::VERTICAL;
                  rect1.setRect(rec.left(), rec.top(), rec.width(), rec.height() * .5);
                  rect2.setRect(rect1.left(), rect1.bottom(), rect1.width(), rec.height() - rect1.height());
                  offset1 = rect1.center().x();
                  offset2 = rect2.center().x();
                  }
            else {
                  type = Node::Type::HORIZONTAL;
                  rect1.setRect(rec.left(), rec.top(), rec.width() * .5, rec.height());
                  rect2.setRect(rect1.right(), rect1.top(), rec.width() - rect1.width(), rect1.height());
                  offset1 = rect1.center().y();
                  offset2 = rect2.center().y();
                  }

            int childIndex = firstChildIndex(index);

            Node* child   = &nodes[childIndex];
            child->offset = offset1;
            child->type   = type;

            child = &nodes[childIndex + 1];
            child->offset = offset2;
            child->type   = type;

            initialize(rect1, dep - 1, childIndex);
            initialize(rect2, dep - 1, childIndex + 1);
            }
      else {
            node->type      = Node::Type::LEAF;
            node->leafIndex = leafCnt++;
            }
      }

//---------------------------------------------------------
//   climbTree
//---------------------------------------------------------

void BspTree::climbTree(BspTreeVisitor* visitor, const QPointF& pos, int index)
      {
      if (nodes.empty())
            return;

      Node* node = &nodes[index];
      int childIndex = firstChildIndex(index);

      switch (node->type) {
            case Node::Type::LEAF:
                  visitor->visit(&leaves[node->leafIndex]);
                  break;
            case Node::Type::VERTICAL:
                  if (pos.x() < node->offset)
                        climbTree(visitor, pos, childIndex);
                  else
                        climbTree(visitor, pos, childIndex + 1);
                  break;
            case Node::Type::HORIZONTAL:
                  if (pos.y() < node->offset)
                        climbTree(visitor, pos, childIndex);
                  else
                        climbTree(visitor, pos, childIndex + 1);
                  break;
            }
      }

//---------------------------------------------------------
//   climbTree
//---------------------------------------------------------

void BspTree::climbTree(BspTreeVisitor* visitor, const QRectF& rec, int index)
      {
      if (nodes.empty())
            return;

      Node* node = &nodes[index];
      int childIndex = firstChildIndex(index);

      switch (node->type) {
            case Node::Type::LEAF:
                  visitor->visit(&leaves[node->leafIndex]);
                  break;
            case Node::Type::VERTICAL:
                  if (rec.left() < node->offset) {
                        climbTree(visitor, rec, childIndex);
                        if (rec.right() >= node->offset)
                              climbTree(visitor, rec, childIndex + 1);
                        }
                  else {
                        climbTree(visitor, rec, childIndex + 1);
                        }
                  break;
            case Node::Type::HORIZONTAL:
                  if (rec.top() < node->offset) {
                        climbTree(visitor, rec, childIndex);
                        if (rec.bottom() >= node->offset)
                              climbTree(visitor, rec, childIndex + 1);
                        }
                  else {
                        climbTree(visitor, rec, childIndex + 1);
                        }
            }
      }

//---------------------------------------------------------
//   rectForIndex
//---------------------------------------------------------

QRectF BspTree::rectForIndex(int index) const
      {
      if (index <= 0)
            return rect;

      int parentIdx = parentIndex(index);
      QRectF rec   = rectForIndex(parentIdx);
      const Node *parent = &nodes.at(parentIdx);

      if (parent->type == Node::Type::HORIZONTAL) {
            if (index & 1)
                  rec.setRight(parent->offset);
            else
                  rec.setLeft(parent->offset);
            }
      else {
            if (index & 1)
                  rec.setBottom(parent->offset);
            else
                  rec.setTop(parent->offset);
            }
      return rec;
      }

}

