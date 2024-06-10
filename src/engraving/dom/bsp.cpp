/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
    : m_leafCnt(0)
{
    m_depth = 0;
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
    m_depth      = intmaxlog(n);
    this->m_rect = rec;
    m_leafCnt    = 0;

    m_nodes.resize((1 << (m_depth + 1)) - 1);
    m_leaves.resize(1LL << m_depth);
    std::fill(m_leaves.begin(), m_leaves.end(), std::list<EngravingItem*>());
    initialize(rec, m_depth, 0);
}

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void BspTree::clear()
{
    m_leafCnt = 0;
    m_nodes.clear();
    m_leaves.clear();
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

//---------------------------------------------------------
//   nearestNeighbor (public)
//---------------------------------------------------------

EngravingItem* BspTree::nearestNeighbor(const PointF& pos)
{
    EngravingItem* nn = nullptr;
    double bestDistance = std::numeric_limits<double>::max();
    nearestNeighbor(pos, &nn, bestDistance);
    return nn;
}

//---------------------------------------------------------
//   nearestNeighbor (private)
//---------------------------------------------------------

void BspTree::nearestNeighbor(const PointF& pos, EngravingItem** bestItem, double& bestDistance, int nodeIndex)
{
    if (m_nodes.empty()) {
        return;
    }

    Node* node = &m_nodes[nodeIndex];

    // Base case: go through the items in the leaf node (if any), and update bestItem/bestDistance accordingly
    if (node->type == Node::Type::LEAF) {
        for (auto item : m_leaves[node->leafIndex]) {
            PointF itemPos = item->pageBoundingRect().center();
            double currDistance = std::sqrt(std::pow(pos.x() - itemPos.x(), 2) + std::pow(pos.y() - itemPos.y(), 2));
            if (currDistance < bestDistance) {
                *bestItem = item;
                bestDistance = currDistance;
            }
        }
        return;
    }

    // Find which child contains pos and which is the "sibling"
    int containerIdx = firstChildIndex(nodeIndex);
    int siblingIdx =  containerIdx + 1;
    if (node->type == Node::Type::VERTICAL) {
        if (pos.x() >= node->offset) {
            ++containerIdx;
            --siblingIdx;
        }
    } else if (pos.y() >= node->offset) {
        ++containerIdx;
        --siblingIdx;
    }

    // Recursion on container node
    nearestNeighbor(pos, bestItem, bestDistance, containerIdx);

    // If the distance to the "offset" is shorter than the best distance we've found so far, then it's possible that the nearest
    // neighbour is in the sibling node (so we should search there too).
    double distanceToOffset;
    if (node->type == Node::Type::HORIZONTAL) {
        distanceToOffset = std::abs(pos.y() - node->offset);
    } else {
        distanceToOffset = std::abs(pos.x() - node->offset);
    }

    if (distanceToOffset < bestDistance) {
        // Recursion on sibling node
        nearestNeighbor(pos, bestItem, bestDistance, siblingIdx);
    }
}

#ifndef NDEBUG
//---------------------------------------------------------
//   debug
//---------------------------------------------------------

String BspTree::debug(int index) const
{
    const Node* node = &m_nodes.at(index);

    String tmp;
    if (node->type == Node::Type::LEAF) {
        RectF rec = rectForIndex(index);
        if (!m_leaves[node->leafIndex].empty()) {
            tmp += String(u"[%1, %2, %3, %4] contains %5 items\n")
                   .arg(rec.left()).arg(rec.top())
                   .arg(rec.width()).arg(rec.height())
                   .arg(m_leaves[node->leafIndex].size());
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
    Node* node = &m_nodes[index];
    if (index == 0) {
        node->type = Node::Type::HORIZONTAL;
        node->offset = rec.center().y();
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

        Node* child   = &m_nodes[childIndex];
        child->offset = offset1;
        child->type   = type;

        child = &m_nodes[childIndex + 1];
        child->offset = offset2;
        child->type   = type;

        initialize(rect1, dep - 1, childIndex);
        initialize(rect2, dep - 1, childIndex + 1);
    } else {
        node->type      = Node::Type::LEAF;
        node->leafIndex = m_leafCnt++;
    }
}

//---------------------------------------------------------
//   climbTree
//---------------------------------------------------------

void BspTree::climbTree(BspTreeVisitor* visitor, const PointF& pos, int index)
{
    if (m_nodes.empty()) {
        return;
    }

    Node* node = &m_nodes[index];
    int childIndex = firstChildIndex(index);

    switch (node->type) {
    case Node::Type::LEAF:
        visitor->visit(&m_leaves[node->leafIndex]);
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

void BspTree::climbTree(BspTreeVisitor* visitor, const RectF& rec, int index)
{
    if (m_nodes.empty()) {
        return;
    }

    Node* node = &m_nodes[index];
    int childIndex = firstChildIndex(index);

    switch (node->type) {
    case Node::Type::LEAF:
        visitor->visit(&m_leaves[node->leafIndex]);
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

RectF BspTree::rectForIndex(int index) const
{
    if (index <= 0) {
        return m_rect;
    }

    int parentIdx = parentIndex(index);
    RectF rec   = rectForIndex(parentIdx);
    const Node* parent = &m_nodes.at(parentIdx);

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
