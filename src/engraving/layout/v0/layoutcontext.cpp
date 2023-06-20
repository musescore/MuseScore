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
#include "layoutcontext.h"

#include "style/defaultstyle.h"

#include "libmscore/mscoreview.h"
#include "libmscore/score.h"
#include "libmscore/spanner.h"

#include "tlayout.h"

using namespace mu::engraving;
using namespace mu::engraving::layout::v0;

DomAccessor::DomAccessor(Score* s)
    : m_score(s) {}

const std::vector<System*>& DomAccessor::systems() const
{
    IF_ASSERT_FAILED(m_score) {
        static const std::vector<System*> dummy;
        return dummy;
    }
    return m_score->systems();
}

size_t DomAccessor::nstaves() const
{
    IF_ASSERT_FAILED(m_score) {
        return 0;
    }
    return m_score->nstaves();
}

const std::vector<Staff*>& DomAccessor::staves() const
{
    IF_ASSERT_FAILED(m_score) {
        static const std::vector<Staff*> dummy;
        return dummy;
    }
    return m_score->staves();
}

const Staff* DomAccessor::staff(staff_idx_t idx) const
{
    IF_ASSERT_FAILED(m_score) {
        return nullptr;
    }
    return m_score->staff(idx);
}

size_t DomAccessor::ntracks() const
{
    IF_ASSERT_FAILED(m_score) {
        return 0;
    }
    return m_score->ntracks();
}

const Measure* DomAccessor::tick2measure(const Fraction& _tick) const
{
    IF_ASSERT_FAILED(m_score) {
        return nullptr;
    }
    return m_score->tick2measure(_tick);
}

LayoutContext::LayoutContext(Score* score)
    : m_score(score), m_dom(score)
{
    firstSystemIndent = score && score->styleB(Sid::enableIndentationOnFirstSystem);
}

LayoutContext::~LayoutContext()
{
    for (Spanner* s : processedSpanners) {
        TLayout::layoutSystemsDone(s);
    }

    for (MuseScoreView* v : score()->getViewer()) {
        v->layoutChanged();
    }
}

bool LayoutContext::isPaletteMode() const
{
    IF_ASSERT_FAILED(m_score) {
        return false;
    }

    return m_score->isPaletteScore();
}

bool LayoutContext::printingMode() const
{
    IF_ASSERT_FAILED(m_score) {
        return false;
    }

    return m_score->printing();
}

bool LayoutContext::lineMode() const
{
    IF_ASSERT_FAILED(m_score) {
        return false;
    }
    return m_score->lineMode();
}

bool LayoutContext::floatMode() const
{
    IF_ASSERT_FAILED(m_score) {
        return false;
    }
    return m_score->floatMode();
}

double LayoutContext::spatium() const
{
    IF_ASSERT_FAILED(m_score) {
        return DefaultStyle::defaultStyle().styleD(Sid::spatium);
    }
    return m_score->spatium();
}

double LayoutContext::point(const Spatium sp) const
{
    return sp.val() * spatium();
}

const MStyle& LayoutContext::style() const
{
    IF_ASSERT_FAILED(m_score) {
        return DefaultStyle::defaultStyle();
    }

    return m_score->style();
}

double LayoutContext::noteHeadWidth() const
{
    IF_ASSERT_FAILED(m_score) {
        return 0.0;
    }
    return m_score->noteHeadWidth();
}

bool LayoutContext::showInvisible() const
{
    IF_ASSERT_FAILED(m_score) {
        return 0.0;
    }
    return m_score->showInvisible();
}

IEngravingFontPtr LayoutContext::engravingFont() const
{
    IF_ASSERT_FAILED(m_score) {
        return nullptr;
    }
    return m_score->engravingFont();
}

const DomAccessor& LayoutContext::dom() const
{
    return m_dom;
}

compat::DummyElement* LayoutContext::dummyParent() const
{
    IF_ASSERT_FAILED(m_score) {
        return nullptr;
    }
    return m_score->dummy();
}

void LayoutContext::undoAddElement(EngravingItem* item, bool addToLinkedStaves, bool ctrlModifier)
{
    IF_ASSERT_FAILED(m_score) {
        return;
    }
    m_score->undoAddElement(item, addToLinkedStaves, ctrlModifier);
}

void LayoutContext::undoRemoveElement(EngravingItem* item)
{
    IF_ASSERT_FAILED(m_score) {
        return;
    }
    m_score->undoRemoveElement(item);
}

void LayoutContext::undo(UndoCommand* cmd, EditData* ed) const
{
    IF_ASSERT_FAILED(m_score) {
        return;
    }
    m_score->undo(cmd, ed);
}

void LayoutContext::addElement(EngravingItem* item)
{
    IF_ASSERT_FAILED(m_score) {
        return;
    }
    m_score->addElement(item);
}

void LayoutContext::removeElement(EngravingItem* item)
{
    IF_ASSERT_FAILED(m_score) {
        return;
    }
    m_score->removeElement(item);
}

void LayoutContext::addUnmanagedSpanner(Spanner* s)
{
    IF_ASSERT_FAILED(m_score) {
        return;
    }
    m_score->addUnmanagedSpanner(s);
}

void LayoutContext::setLayout(const Fraction& tick1, const Fraction& tick2, staff_idx_t staff1, staff_idx_t staff2, const EngravingItem* e)
{
    IF_ASSERT_FAILED(m_score) {
        return;
    }
    m_score->setLayout(tick1, tick2, staff1, staff2, e);
}

void LayoutContext::addRefresh(const mu::RectF& r)
{
    IF_ASSERT_FAILED(m_score) {
        return;
    }
    m_score->addRefresh(r);
}
