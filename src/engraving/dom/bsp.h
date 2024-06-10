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

#ifndef MU_ENGRAVING_BSP_H
#define MU_ENGRAVING_BSP_H

#include <list>

#include "global/allocator.h"
#include "types/string.h"
#include "../types/types.h"

namespace mu::engraving {
class BspTreeVisitor;
class InsertItemBspTreeVisitor;
class RemoveItemBspTreeVisitor;
class FindItemBspTreeVisitor;

class EngravingItem;

//---------------------------------------------------------
//   BspTree
//    binary space partitioning
//---------------------------------------------------------

class BspTree
{
public:
    struct Node {
        enum class Type : char {
            HORIZONTAL, VERTICAL, LEAF
        };
        union {
            double offset;
            int leafIndex;
        };
        Type type;
    };
private:

    void initialize(const RectF& rect, int depth, int index);
    void climbTree(BspTreeVisitor* visitor, const PointF& pos, int index = 0);
    void climbTree(BspTreeVisitor* visitor, const RectF& rect, int index = 0);

    void findItems(std::list<EngravingItem*>* foundItems, const RectF& rect, int index);
    void findItems(std::list<EngravingItem*>* foundItems, const PointF& pos, int index);

    void nearestNeighbor(const PointF& pos, EngravingItem** bestItem, double& bestDistance, int nodeIndex = 0);

    RectF rectForIndex(int index) const;

    unsigned int m_depth = 0;
    std::vector<Node> m_nodes;
    std::vector<std::list<EngravingItem*> > m_leaves;
    int m_leafCnt = 0;
    RectF m_rect;

public:
    BspTree();

    void initialize(const RectF& rect, int depth);
    void clear();

    void insert(EngravingItem* item);
    void remove(EngravingItem* item);

    std::vector<EngravingItem*> items(const RectF& rect);
    std::vector<EngravingItem*> items(const PointF& pos);

    EngravingItem* nearestNeighbor(const PointF& pos);

    int leafCount() const { return m_leafCnt; }
    inline int firstChildIndex(int index) const { return index * 2 + 1; }

    inline int parentIndex(int index) const
    {
        return index > 0 ? ((index & 1) ? ((index - 1) / 2) : ((index - 2) / 2)) : -1;
    }

#ifndef NDEBUG
    String debug(int index) const;
#endif
};

//---------------------------------------------------------
//   BspTreeVisitor
//---------------------------------------------------------

class BspTreeVisitor
{
    OBJECT_ALLOCATOR(engraving, BspTreeVisitor)
public:
    virtual ~BspTreeVisitor() {}
    virtual void visit(std::list<EngravingItem*>* items) = 0;
};
} // namespace mu::engraving
#endif
