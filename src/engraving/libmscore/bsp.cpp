/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <cmath>

#include "bsp.h"
#include "engravingitem.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   InsertItemBspTreeVisitor
//---------------------------------------------------------

class InsertItemBspTreeVisitor : public BspTreeVisitor
{
    OBJECT_ALLOCATOR(engraving, InsertItemBspTreeVisitor)
public:
    EngravingItem* item;

    inline void visit(std::list<EngravingItem*>* items) { items->push_front(item); }
};

//---------------------------------------------------------
//   RemoveItemBspTreeVisitor
//---------------------------------------------------------

class RemoveItemBspTreeVisitor : public BspTreeVisitor
{
    OBJECT_ALLOCATOR(engraving, RemoveItemBspTreeVisitor)
public:
    EngravingItem* item;

    inline void visit(std::list<EngravingItem*>* items) { items->remove(item); }
};

//---------------------------------------------------------
//   FindItemBspTreeVisitor
//---------------------------------------------------------

class FindItemBspTreeVisitor : public BspTreeVisitor
{
    OBJECT_ALLOCATOR(engraving, FindItemBspTreeVisitor)
public:
    std::list<EngravingItem*> foundItems;

    void visit(std::list<EngravingItem*>* items)
    {
        for (auto it = items->begin(); it != items->end(); ++it) {
            EngravingItem* item = *it;
            if (!item->itemDiscovered) {
                item->itemDiscovered = true;
                foundItems.push_front(item);
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
    return n > 0 ? std::max(int(::ceil(::log(double(n)) / ::log(double(2)))), 5) : 0;
}

//---------------------------------------------------------
//   initialize
//---------------------------------------------------------

void BspTree::initialize(const RectF& rec, int n)
{
    depth      = intmaxlog(n);
    this->rect = rec;
    leafCnt    = 0;

    nodes.resize((1 << (depth + 1)) - 1);
    leaves.resize(1LL << depth);
    std::fill(leaves.begin(), leaves.end(), std::list<EngravingItem*>());
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

void BspTree::insert(EngravingItem* element)
{
    InsertItemBspTreeVisitor insertVisitor;
    insertVisitor.item = element;
    climbTree(&insertVisitor, element->pageBoundingRect());
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void BspTree::remove(EngravingItem* element)
{
    RemoveItemBspTreeVisitor removeVisitor;
    removeVisitor.item = element;
    climbTree(&removeVisitor, element->pageBoundingRect());
}

//---------------------------------------------------------
//   items
//---------------------------------------------------------

std::vector<EngravingItem*> BspTree::items(const RectF& rec)
{
    FindItemBspTreeVisitor findVisitor;
    climbTree(&findVisitor, rec);
    std::vector<EngravingItem*> l;
    for (EngravingItem* e : findVisitor.foundItems) {
        e->itemDiscovered = false;
        if (e->pageBoundingRect().intersects(rec)) {
            l.push_back(e);
        }
    }
    return l;
}

//---------------------------------------------------------
//   items
//---------------------------------------------------------

std::vector<EngravingItem*> BspTree::items(const PointF& pos)
{
    FindItemBspTreeVisitor findVisitor;
    climbTree(&findVisitor, pos);

    std::vector<EngravingItem*> l;
    for (EngravingItem* e : findVisitor.foundItems) {
        e->itemDiscovered = false;
        if (e->contains(pos)) {
            l.push_back(e);
        }
    }
    return l;
}

#ifndef NDEBUG
//---------------------------------------------------------
//   debug
//---------------------------------------------------------

String BspTree::debug(int index) const
{
    const Node* node = &nodes.at(index);

    String tmp;
    if (node->type == Node::Type::LEAF) {
        RectF rec = rectForIndex(index);
        if (!leaves[node->leafIndex].empty()) {
            tmp += String(u"[%1, %2, %3, %4] contains %5 items\n")
                   .arg(rec.left()).arg(rec.top())
                   .arg(rec.width()).arg(rec.height())
                   .arg(leaves[node->leafIndex].size());
        }
    } else {
        if (node->type == Node::Type::HORIZONTAL) {
            tmp += debug(firstChildIndex(index));
            tmp += debug(firstChildIndex(index) + 1);
        } else {
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

void BspTree::initialize(const RectF& rec, int dep, int index)
{
    Node* node = &nodes[index];
    if (index == 0) {
        node->type = Node::Type::HORIZONTAL;
        node->offset = rec.center().x();
    }

    if (dep) {
        Node::Type type;
        RectF rect1, rect2;
        double offset1, offset2;

        if (node->type == Node::Type::HORIZONTAL) {
            type = Node::Type::VERTICAL;
            rect1.setRect(rec.left(), rec.top(), rec.width(), rec.height() * .5);
            rect2.setRect(rect1.left(), rect1.bottom(), rect1.width(), rec.height() - rect1.height());
            offset1 = rect1.center().x();
            offset2 = rect2.center().x();
        } else {
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
    } else {
        node->type      = Node::Type::LEAF;
        node->leafIndex = leafCnt++;
    }
}

//---------------------------------------------------------
//   climbTree
//---------------------------------------------------------

void BspTree::climbTree(BspTreeVisitor* visitor, const mu::PointF& pos, int index)
{
    if (nodes.empty()) {
        return;
    }

    Node* node = &nodes[index];
    int childIndex = firstChildIndex(index);

    switch (node->type) {
    case Node::Type::LEAF:
        visitor->visit(&leaves[node->leafIndex]);
        break;
    case Node::Type::VERTICAL:
        if (pos.x() < node->offset) {
            climbTree(visitor, pos, childIndex);
        } else {
            climbTree(visitor, pos, childIndex + 1);
        }
        break;
    case Node::Type::HORIZONTAL:
        if (pos.y() < node->offset) {
            climbTree(visitor, pos, childIndex);
        } else {
            climbTree(visitor, pos, childIndex + 1);
        }
        break;
    }
}

//---------------------------------------------------------
//   climbTree
//---------------------------------------------------------

void BspTree::climbTree(BspTreeVisitor* visitor, const mu::RectF& rec, int index)
{
    if (nodes.empty()) {
        return;
    }

    Node* node = &nodes[index];
    int childIndex = firstChildIndex(index);

    switch (node->type) {
    case Node::Type::LEAF:
        visitor->visit(&leaves[node->leafIndex]);
        break;
    case Node::Type::VERTICAL:
        if (rec.left() < node->offset) {
            climbTree(visitor, rec, childIndex);
            if (rec.right() >= node->offset) {
                climbTree(visitor, rec, childIndex + 1);
            }
        } else {
            climbTree(visitor, rec, childIndex + 1);
        }
        break;
    case Node::Type::HORIZONTAL:
        if (rec.top() < node->offset) {
            climbTree(visitor, rec, childIndex);
            if (rec.bottom() >= node->offset) {
                climbTree(visitor, rec, childIndex + 1);
            }
        } else {
            climbTree(visitor, rec, childIndex + 1);
        }
    }
}

//---------------------------------------------------------
//   rectForIndex
//---------------------------------------------------------

mu::RectF BspTree::rectForIndex(int index) const
{
    if (index <= 0) {
        return rect;
    }

    int parentIdx = parentIndex(index);
    RectF rec   = rectForIndex(parentIdx);
    const Node* parent = &nodes.at(parentIdx);

    if (parent->type == Node::Type::HORIZONTAL) {
        if (index & 1) {
            rec.setRight(parent->offset);
        } else {
            rec.setLeft(parent->offset);
        }
    } else {
        if (index & 1) {
            rec.setBottom(parent->offset);
        } else {
            rec.setTop(parent->offset);
        }
    }
    return rec;
}
}
