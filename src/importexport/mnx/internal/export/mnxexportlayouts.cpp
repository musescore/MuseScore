/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
#include "mnxexporter.h"

#include <algorithm>
#include <optional>
#include <vector>

#include "engraving/dom/bracketItem.h"
#include "engraving/dom/part.h"
#include "engraving/dom/score.h"
#include "engraving/dom/staff.h"
#include "log.h"
#include "internal/shared/mnxtypesconv.h"

using namespace mu::engraving;

namespace mu::iex::mnxio {
namespace {
struct GroupSpan {
    size_t start{};
    size_t end{};
    size_t column{};
    BracketType type{ BracketType::NO_BRACKET };
};

struct GroupNode {
    GroupSpan span;
    std::vector<size_t> children;
};

struct LayoutBuildContext {
    MnxExporter* exporter{};
    const std::vector<Staff*>* staves{};
    size_t staffCount{};
    const std::vector<GroupNode>* nodes{};
};
} // namespace

//---------------------------------------------------------
//   containsSpan
//---------------------------------------------------------

static bool containsSpan(const GroupSpan& parent, const GroupSpan& child)
{
    return parent.start <= child.start && parent.end >= child.end;
}

//---------------------------------------------------------
//   sortChildIndices
//---------------------------------------------------------

static void sortChildIndices(std::vector<size_t>& childIdxs, const std::vector<GroupNode>& nodes)
{
    std::sort(childIdxs.begin(), childIdxs.end(), [&](size_t lhs, size_t rhs) {
        const GroupSpan& a = nodes[lhs].span;
        const GroupSpan& b = nodes[rhs].span;
        if (a.start != b.start) {
            return a.start < b.start;
        }
        if (a.end != b.end) {
            return a.end > b.end;
        }
        return a.column < b.column;
    });
}

//---------------------------------------------------------
//   buildGroupSpans
//---------------------------------------------------------

static std::vector<GroupSpan> buildGroupSpans(const std::vector<Staff*>& staves, size_t staffCount)
{
    std::vector<GroupSpan> groups;
    groups.reserve(staffCount);

    for (size_t staffIdx = 0; staffIdx < staffCount; ++staffIdx) {
        const Staff* staff = staves[staffIdx];
        if (!staff) {
            continue;
        }
        for (const BracketItem* bracket : staff->brackets()) {
            if (!bracket) {
                continue;
            }
            if (bracket->bracketType() == BracketType::NO_BRACKET) {
                continue;
            }
            const size_t span = bracket->bracketSpan();
            if (span == 0) {
                continue;
            }
            size_t end = staffIdx + span - 1;
            if (end >= staffCount) {
                LOGW() << "Bracket span exceeds staff count; clamping span. Start="
                       << staffIdx << " span=" << span << " staves=" << staffCount;
                end = staffCount - 1;
                if (end < staffIdx) {
                    continue;
                }
            }
            groups.push_back({ staffIdx, end, bracket->column(), bracket->bracketType() });
        }
    }

    std::sort(groups.begin(), groups.end(), [](const GroupSpan& a, const GroupSpan& b) {
        if (a.start != b.start) {
            return a.start < b.start;
        }
        if (a.end != b.end) {
            return a.end > b.end;
        }
        return a.column < b.column;
    });

    return groups;
}

//---------------------------------------------------------
//   buildGroupNodes
//---------------------------------------------------------

static void buildGroupNodes(const std::vector<GroupSpan>& groups, std::vector<GroupNode>& nodes,
                            std::vector<size_t>& rootChildren)
{
    nodes.clear();
    rootChildren.clear();
    nodes.reserve(groups.size());

    for (const GroupSpan& group : groups) {
        const size_t nodeIdx = nodes.size();
        nodes.push_back({ group, {} });

        std::optional<size_t> parentIdx;
        for (size_t candidate = 0; candidate < nodeIdx; ++candidate) {
            const GroupSpan& parentSpan = nodes[candidate].span;
            if (!containsSpan(parentSpan, group) || parentSpan.column >= group.column) {
                continue;
            }
            if (!parentIdx) {
                parentIdx = candidate;
                continue;
            }
            const GroupSpan& bestSpan = nodes[*parentIdx].span;
            if (parentSpan.column > bestSpan.column) {
                parentIdx = candidate;
                continue;
            }
            const size_t parentSpanSize = parentSpan.end - parentSpan.start;
            const size_t bestSpanSize = bestSpan.end - bestSpan.start;
            if (parentSpan.column == bestSpan.column && parentSpanSize < bestSpanSize) {
                parentIdx = candidate;
            }
        }

        if (parentIdx) {
            nodes[*parentIdx].children.push_back(nodeIdx);
        } else {
            rootChildren.push_back(nodeIdx);
        }
    }

    for (auto& node : nodes) {
        sortChildIndices(node.children, nodes);
    }
    sortChildIndices(rootChildren, nodes);
}

//---------------------------------------------------------
//   appendLayoutStaff
//---------------------------------------------------------

static void appendLayoutStaff(LayoutBuildContext& ctx, mnx::ContentArray content, size_t staffIdx)
{
    Staff* staff = ctx.staves->at(staffIdx);
    if (!staff) {
        LOGW() << "Skipping layout staff " << staffIdx << " with null staff.";
        return;
    }
    const staff_idx_t scoreStaffIdx = staff->idx();
    const auto& staffToPartStaff = ctx.exporter->staffToPartStaff();
    const auto it = staffToPartStaff.find(scoreStaffIdx);
    if (it == staffToPartStaff.end()) {
        LOGW() << "Skipping layout staff " << staffIdx << " with no part/staff mapping.";
        return;
    }
    const auto [partIndex, staffNum] = it->second;
    std::string partId;
    const mnx::Document& document = ctx.exporter->mnxDocument();
    if (partIndex < document.parts().size()) {
        const auto mnxPart = document.parts()[partIndex];
        if (const auto id = mnxPart.id()) {
            partId = *id;
        }
    }
    if (partId.empty()) {
        Part* part = staff ? staff->part() : nullptr;
        if (part) {
            partId = ctx.exporter->getOrAssignEID(part).toStdString();
        }
    }
    if (partId.empty()) {
        LOGW() << "Skipping layout staff " << staffIdx << " with no part id.";
        return;
    }
    auto mnxStaff = content.append<mnx::layout::Staff>();
    auto source = mnxStaff.sources().append(partId);
    source.set_staff(staffNum);
}

//---------------------------------------------------------
//   buildContent
//---------------------------------------------------------

static void buildContent(LayoutBuildContext& ctx, mnx::ContentArray content,
                         size_t start, size_t end, const std::vector<size_t>& children)
{
    size_t staffIdx = start;
    size_t childPos = 0;
    const auto& nodes = *ctx.nodes;
    while (staffIdx <= end && staffIdx < static_cast<size_t>(ctx.staffCount)) {
        while (childPos < children.size()
               && nodes[children[childPos]].span.end < staffIdx) {
            ++childPos;
        }
        if (childPos < children.size()
            && nodes[children[childPos]].span.start == staffIdx) {
            const GroupNode& node = nodes[children[childPos]];
            auto mnxGroup = content.append<mnx::layout::Group>();
            const mnx::LayoutSymbol symbol = toMnxLayoutSymbol(node.span.type);
            if (symbol != mnx::LayoutSymbol::NoSymbol) {
                mnxGroup.set_symbol(symbol);
            }
            buildContent(ctx, mnxGroup.content(), node.span.start, node.span.end, node.children);
            staffIdx = node.span.end + 1;
            ++childPos;
        } else {
            appendLayoutStaff(ctx, content, staffIdx);
            ++staffIdx;
        }
    }
}

//---------------------------------------------------------
//   createLayout
//---------------------------------------------------------

void MnxExporter::createLayout(const std::vector<Staff*>& staves, const std::string& layoutId)
{
    if (!m_mnxDocument.layouts()) {
        m_mnxDocument.ensure_layouts();
    }

    auto mnxLayout = m_mnxDocument.layouts().value().append();
    if (!layoutId.empty()) {
        mnxLayout.set_id(layoutId);
    }

    const size_t staffCount = staves.size();
    if (staffCount == 0) {
        return;
    }

    std::vector<GroupSpan> groups = buildGroupSpans(staves, staffCount);

    std::vector<GroupNode> nodes;
    std::vector<size_t> rootChildren;
    buildGroupNodes(groups, nodes, rootChildren);

    LayoutBuildContext ctx;
    ctx.exporter = this;
    ctx.staves = &staves;
    ctx.staffCount = staffCount;
    ctx.nodes = &nodes;

    buildContent(ctx, mnxLayout.content(), 0, staffCount - 1, rootChildren);
}
} // namespace mu::iex::mnxio
