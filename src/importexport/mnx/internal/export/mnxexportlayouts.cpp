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

#include "engraving/dom/barline.h"
#include "engraving/dom/bracketItem.h"
#include "engraving/dom/part.h"
#include "engraving/dom/score.h"
#include "engraving/dom/staff.h"
#include "log.h"
#include "internal/shared/mnxtypesconv.h"

using namespace mu::engraving;

namespace mu::iex::mnxio {
namespace {
struct LayoutGroupSpan {
    size_t start{};
    size_t end{};
    size_t column{};
    BracketType type{ BracketType::NO_BRACKET };
};

struct LayoutGroupNode {
    LayoutGroupSpan span;
    std::vector<size_t> children;
};

struct LayoutBuildContext {
    MnxExporter* exporter{};
    const std::vector<Staff*>* staves{};
    size_t staffCount{};
    const std::vector<LayoutGroupNode>* nodes{};
};
} // namespace

//---------------------------------------------------------
//   containsSpan
//---------------------------------------------------------

static bool containsSpan(const LayoutGroupSpan& parent, const LayoutGroupSpan& child)
{
    return parent.start <= child.start && parent.end >= child.end;
}

//---------------------------------------------------------
//   sortChildIndices
//---------------------------------------------------------

static void sortChildIndices(std::vector<size_t>& childIdxs, const std::vector<LayoutGroupNode>& nodes)
{
    std::sort(childIdxs.begin(), childIdxs.end(), [&](size_t lhs, size_t rhs) {
        const LayoutGroupSpan& a = nodes[lhs].span;
        const LayoutGroupSpan& b = nodes[rhs].span;
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

static std::vector<LayoutGroupSpan> buildGroupSpans(const std::vector<Staff*>& staves, size_t staffCount)
{
    std::vector<LayoutGroupSpan> groups;
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
            if (bracket->bracketType() == BracketType::NO_BRACKET) {
                if (span <= 1) {
                    continue;
                }
                bool hasBarlineGrouping = false;
                for (size_t idx = staffIdx; idx < end; ++idx) {
                    const Staff* groupedStaff = staves[idx];
                    if (groupedStaff && groupedStaff->barLineSpan()) {
                        hasBarlineGrouping = true;
                        break;
                    }
                }
                if (!hasBarlineGrouping) {
                    continue;
                }
            }
            groups.push_back({ staffIdx, end, bracket->column(), bracket->bracketType() });
        }
    }

    std::sort(groups.begin(), groups.end(), [](const LayoutGroupSpan& a, const LayoutGroupSpan& b) {
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

static void buildGroupNodes(const std::vector<LayoutGroupSpan>& groups, std::vector<LayoutGroupNode>& nodes,
                            std::vector<size_t>& rootChildren)
{
    nodes.clear();
    rootChildren.clear();
    nodes.reserve(groups.size());

    for (const LayoutGroupSpan& group : groups) {
        const size_t nodeIdx = nodes.size();
        nodes.push_back({ group, {} });

        std::optional<size_t> parentIdx;
        for (size_t candidate = 0; candidate < nodeIdx; ++candidate) {
            const LayoutGroupSpan& parentSpan = nodes[candidate].span;
            if (!containsSpan(parentSpan, group) || parentSpan.column >= group.column) {
                continue;
            }
            if (!parentIdx) {
                parentIdx = candidate;
                continue;
            }
            const LayoutGroupSpan& bestSpan = nodes[*parentIdx].span;
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
//   mensurStricheSpanFrom
//---------------------------------------------------------

static int mensurStricheSpanFrom(const Staff* staff)
{
    IF_ASSERT_FAILED(staff) {
        return 0;
    }
    const int lines = staff->lines(Fraction(0, 1)) - 1;
    return lines <= 0 ? BARLINE_SPAN_1LINESTAFF_TO : 2 * lines;
}

//---------------------------------------------------------
//   isSinglePartGroup
//---------------------------------------------------------

static bool isSinglePartGroup(const LayoutBuildContext& ctx, const LayoutGroupSpan& span)
{
    if (span.start >= ctx.staffCount || span.end >= ctx.staffCount) {
        return false;
    }

    const Staff* firstStaff = ctx.staves->at(span.start);
    const Part* firstPart = firstStaff ? firstStaff->part() : nullptr;
    if (!firstPart) {
        return false;
    }

    for (size_t idx = span.start + 1; idx <= span.end; ++idx) {
        const Staff* staff = ctx.staves->at(idx);
        if (!staff || staff->part() != firstPart) {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------
//   calcGroupBarlineOverride
//---------------------------------------------------------

static mnx::StaffGroupBarlineOverride calcGroupBarlineOverride(const LayoutBuildContext& ctx, const LayoutGroupSpan& span)
{
    if (span.end <= span.start || span.end >= ctx.staffCount) {
        return mnx::StaffGroupBarlineOverride::None;
    }

    bool allConnected = true;
    bool allMensurstrich = true;

    for (size_t idx = span.start; idx < span.end; ++idx) {
        const Staff* staff = ctx.staves->at(idx);
        if (!staff || !staff->barLineSpan()) {
            allConnected = false;
            break;
        }

        const bool hasMensurSpan = (staff->barLineTo() == 0)
                                   && (staff->barLineFrom() == mensurStricheSpanFrom(staff));
        allMensurstrich = allMensurstrich && hasMensurSpan;
    }

    if (!allConnected) {
        return mnx::StaffGroupBarlineOverride::None;
    }
    if (allMensurstrich) {
        return mnx::StaffGroupBarlineOverride::Mensurstrich;
    }
    return mnx::StaffGroupBarlineOverride::Unified;
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
            const LayoutGroupNode& node = nodes[children[childPos]];
            auto mnxGroup = content.append<mnx::layout::Group>();
            switch (calcGroupBarlineOverride(ctx, node.span)) {
            case mnx::StaffGroupBarlineOverride::None:
                mnxGroup.set_or_clear_barlineStyle(mnx::StaffGroupBarlineStyle::Individual);
                break;
            case mnx::StaffGroupBarlineOverride::Unified:
                mnxGroup.set_or_clear_barlineStyle(isSinglePartGroup(ctx, node.span)
                                                   ? mnx::StaffGroupBarlineStyle::Instrument
                                                   : mnx::StaffGroupBarlineStyle::Unified);
                break;
            case mnx::StaffGroupBarlineOverride::Mensurstrich:
                mnxGroup.set_or_clear_barlineStyle(mnx::StaffGroupBarlineStyle::Mensurstrich);
                break;
            }
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
    auto mnxLayout = m_mnxDocument.ensure_layouts().append();
    if (!layoutId.empty()) {
        mnxLayout.set_id(layoutId);
    }

    const size_t staffCount = staves.size();
    if (staffCount == 0) {
        return;
    }

    std::vector<LayoutGroupSpan> groups = buildGroupSpans(staves, staffCount);

    std::vector<LayoutGroupNode> nodes;
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
