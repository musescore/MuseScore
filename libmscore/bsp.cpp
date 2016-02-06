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

void BspTree::initialize(const QRectF& rect, int n)
      {
      depth      = intmaxlog(n);
      this->rect = rect;
      leafCnt    = 0;

      nodes.resize((1 << (depth+1)) - 1);
      leaves.resize(1 << depth);
      leaves.fill(QList<Element*>());
      initialize(rect, depth, 0);
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

QList<Element*> BspTree::items(const QRectF& rect)
      {
      FindItemBspTreeVisitor findVisitor;
      climbTree(&findVisitor, rect);
      QList<Element*> l;
      for (Element * e : findVisitor.foundItems) {
          e->itemDiscovered = false;
          if (e->pageBoundingRect().intersects(rect))
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
      for (Element* e : findVisitor.foundItems) {
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
            QRectF rect = rectForIndex(index);
            if (!leaves[node->leafIndex].empty()) {
                  tmp += QString::fromLatin1("[%1, %2, %3, %4] contains %5 items\n")
                   .arg(rect.left()).arg(rect.top())
                   .arg(rect.width()).arg(rect.height())
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

void BspTree::initialize(const QRectF& rect, int depth, int index)
      {
      Node* node = &nodes[index];
      if (index == 0) {
            node->type = Node::Type::HORIZONTAL;
            node->offset = rect.center().x();
            }

      if (depth) {
            Node::Type type;
            QRectF rect1, rect2;
            qreal offset1, offset2;

            if (node->type == Node::Type::HORIZONTAL) {
                  type = Node::Type::VERTICAL;
                  rect1.setRect(rect.left(), rect.top(), rect.width(), rect.height() * .5);
                  rect2.setRect(rect1.left(), rect1.bottom(), rect1.width(), rect.height() - rect1.height());
                  offset1 = rect1.center().x();
                  offset2 = rect2.center().x();
                  }
            else {
                  type = Node::Type::HORIZONTAL;
                  rect1.setRect(rect.left(), rect.top(), rect.width() * .5, rect.height());
                  rect2.setRect(rect1.right(), rect1.top(), rect.width() - rect1.width(), rect1.height());
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

            initialize(rect1, depth - 1, childIndex);
            initialize(rect2, depth - 1, childIndex + 1);
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

void BspTree::climbTree(BspTreeVisitor* visitor, const QRectF& rect, int index)
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
                  if (rect.left() < node->offset) {
                        climbTree(visitor, rect, childIndex);
                        if (rect.right() >= node->offset)
                              climbTree(visitor, rect, childIndex + 1);
                        }
                  else {
                        climbTree(visitor, rect, childIndex + 1);
                        }
                  break;
            case Node::Type::HORIZONTAL:
                  int childIndex = firstChildIndex(index);
                  if (rect.top() < node->offset) {
                        climbTree(visitor, rect, childIndex);
                        if (rect.bottom() >= node->offset)
                              climbTree(visitor, rect, childIndex + 1);
                        }
                  else {
                        climbTree(visitor, rect, childIndex + 1);
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
      QRectF rect   = rectForIndex(parentIdx);
      const Node *parent = &nodes.at(parentIdx);

      if (parent->type == Node::Type::HORIZONTAL) {
            if (index & 1)
                  rect.setRight(parent->offset);
            else
                  rect.setLeft(parent->offset);
            }
      else {
            if (index & 1)
                  rect.setBottom(parent->offset);
            else
                  rect.setTop(parent->offset);
            }
      return rect;
      }

}

