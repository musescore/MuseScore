/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "dom/undo.h"
#include "style/defaultstyle.h"

#include "dom/mscoreview.h"
#include "dom/score.h"
#include "dom/spanner.h"

#include "tlayout.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

// ================================================
// LayoutConfiguration
// ================================================

LayoutConfiguration::LayoutConfiguration(IGetScoreInternal* s)
    : m_getScore(s)
{}

const Score* LayoutConfiguration::score() const
{
    const Score* score = m_getScore->score();
    IF_ASSERT_FAILED(score) {
        return nullptr;
    }
    return score;
}

const LayoutOptions& LayoutConfiguration::options() const
{
    return score()->layoutOptions();
}

bool LayoutConfiguration::isPaletteMode() const
{
    IF_ASSERT_FAILED(score()) {
        return false;
    }

    return score()->isPaletteScore();
}

bool LayoutConfiguration::isPrintingMode() const
{
    IF_ASSERT_FAILED(score()) {
        return false;
    }

    return score()->printing();
}

std::shared_ptr<const IEngravingFont> LayoutConfiguration::engravingFont() const
{
    IF_ASSERT_FAILED(score()) {
        return nullptr;
    }
    return score()->engravingFont();
}

const MStyle& LayoutConfiguration::style() const
{
    IF_ASSERT_FAILED(score()) {
        return DefaultStyle::defaultStyle();
    }
    return score()->style();
}

bool LayoutConfiguration::isShowInvisible() const
{
    IF_ASSERT_FAILED(score()) {
        return false;
    }
    return score()->isShowInvisible();
}

int LayoutConfiguration::pageNumberOffset() const
{
    IF_ASSERT_FAILED(score()) {
        return 0;
    }
    return score()->pageNumberOffset();
}

bool LayoutConfiguration::isVerticalSpreadEnabled() const
{
    return styleB(Sid::enableVerticalSpread) && (viewMode() != LayoutMode::SYSTEM);
}

double LayoutConfiguration::maxSystemDistance() const
{
    if (isVerticalSpreadEnabled()) {
        return style().styleMM(Sid::maxSystemSpread);
    } else {
        return style().styleMM(Sid::maxSystemDistance);
    }
}

bool LayoutConfiguration::isShowInstrumentNames() const
{
    return score()->showInstrumentNames();
}

// ================================================
// DomAccessor
// ================================================

DomAccessor::DomAccessor(IGetScoreInternal* s)
    : m_getScore(s)
{}

const Score* DomAccessor::score() const
{
    return m_getScore->score();
}

Score* DomAccessor::score()
{
    return m_getScore->score();
}

const std::vector<Part*>& DomAccessor::parts() const
{
    IF_ASSERT_FAILED(score()) {
        static const std::vector<Part*> dummy;
        return dummy;
    }
    return score()->parts();
}

int DomAccessor::visiblePartCount() const
{
    IF_ASSERT_FAILED(score()) {
        return 0;
    }
    return score()->visiblePartCount();
}

size_t DomAccessor::npages() const
{
    IF_ASSERT_FAILED(score()) {
        return 0;
    }
    return score()->npages();
}

const std::vector<Page*>& DomAccessor::pages() const
{
    IF_ASSERT_FAILED(score()) {
        static const std::vector<Page*> dummy;
        return dummy;
    }
    return score()->pages();
}

std::vector<Page*>& DomAccessor::pages()
{
    IF_ASSERT_FAILED(score()) {
        static std::vector<Page*> dummy;
        return dummy;
    }
    return score()->pages();
}

const std::vector<System*>& DomAccessor::systems() const
{
    IF_ASSERT_FAILED(score()) {
        static const std::vector<System*> dummy;
        return dummy;
    }
    return score()->systems();
}

std::vector<System*>& DomAccessor::systems()
{
    IF_ASSERT_FAILED(score()) {
        static std::vector<System*> dummy;
        return dummy;
    }
    return score()->systems();
}

size_t DomAccessor::nstaves() const
{
    IF_ASSERT_FAILED(score()) {
        return 0;
    }
    return score()->nstaves();
}

const std::vector<Staff*>& DomAccessor::staves() const
{
    IF_ASSERT_FAILED(score()) {
        static const std::vector<Staff*> dummy;
        return dummy;
    }
    return score()->staves();
}

const Staff* DomAccessor::staff(staff_idx_t idx) const
{
    IF_ASSERT_FAILED(score()) {
        return nullptr;
    }
    return score()->staff(idx);
}

size_t DomAccessor::ntracks() const
{
    IF_ASSERT_FAILED(score()) {
        return 0;
    }
    return score()->ntracks();
}

const Measure* DomAccessor::tick2measure(const Fraction& tick) const
{
    IF_ASSERT_FAILED(score()) {
        return nullptr;
    }
    return score()->tick2measure(tick);
}

const Measure* DomAccessor::firstMeasure() const
{
    IF_ASSERT_FAILED(score()) {
        return nullptr;
    }
    return score()->firstMeasure();
}

Measure* DomAccessor::firstMeasure()
{
    IF_ASSERT_FAILED(score()) {
        return nullptr;
    }
    return score()->firstMeasure();
}

Measure* DomAccessor::tick2measure(const Fraction& tick)
{
    IF_ASSERT_FAILED(score()) {
        return nullptr;
    }
    return score()->tick2measure(tick);
}

const SpannerMap& DomAccessor::spannerMap() const
{
    IF_ASSERT_FAILED(score()) {
        static const SpannerMap dummy;
        return dummy;
    }
    return score()->spannerMap();
}

const Segment* DomAccessor::lastSegment() const
{
    IF_ASSERT_FAILED(score()) {
        return nullptr;
    }
    return score()->lastSegment();
}

const ChordRest* DomAccessor::findCR(Fraction tick, track_idx_t track) const
{
    IF_ASSERT_FAILED(score()) {
        return nullptr;
    }
    return score()->findCR(tick, track);
}

const SystemLocks* DomAccessor::systemLocks() const
{
    IF_ASSERT_FAILED(score()) {
        return nullptr;
    }
    return score()->systemLocks();
}

ChordRest* DomAccessor::findCR(Fraction tick, track_idx_t track)
{
    IF_ASSERT_FAILED(score()) {
        return nullptr;
    }
    return score()->findCR(tick, track);
}

MeasureBase* DomAccessor::first()
{
    IF_ASSERT_FAILED(score()) {
        return nullptr;
    }
    return score()->first();
}

RootItem* DomAccessor::rootItem() const
{
    IF_ASSERT_FAILED(score()) {
        return nullptr;
    }
    return score()->rootItem();
}

compat::DummyElement* DomAccessor::dummyParent() const
{
    IF_ASSERT_FAILED(score()) {
        return nullptr;
    }
    return score()->dummy();
}

void DomAccessor::doUndoAddElement(EngravingItem* item)
{
    if (item->generated()) {
        addElement(item);
    } else {
        undo(new AddElement(item));
    }
}

void DomAccessor::undoAddElement(EngravingItem* item, bool addToLinkedStaves, bool ctrlModifier)
{
    IF_ASSERT_FAILED(score()) {
        return;
    }
    score()->undoAddElement(item, addToLinkedStaves, ctrlModifier);
}

void DomAccessor::doUndoRemoveElement(EngravingItem* item)
{
    if (item->generated()) {
        removeElement(item);
        item->deleteLater();
    } else {
        undo(new RemoveElement(item));
    }
}

void DomAccessor::undoRemoveElement(EngravingItem* item)
{
    IF_ASSERT_FAILED(score()) {
        return;
    }
    score()->undoRemoveElement(item);
}

void DomAccessor::undo(UndoCommand* cmd, EditData* ed) const
{
    IF_ASSERT_FAILED(score()) {
        return;
    }
    score()->undo(cmd, ed);
}

void DomAccessor::addElement(EngravingItem* item)
{
    IF_ASSERT_FAILED(score()) {
        return;
    }
    score()->addElement(item);
}

void DomAccessor::removeElement(EngravingItem* item)
{
    IF_ASSERT_FAILED(score()) {
        return;
    }
    score()->removeElement(item);
}

void DomAccessor::addUnmanagedSpanner(Spanner* s)
{
    IF_ASSERT_FAILED(score()) {
        return;
    }
    score()->addUnmanagedSpanner(s);
}

const std::set<Spanner*>& DomAccessor::unmanagedSpanners() const
{
    IF_ASSERT_FAILED(score()) {
        static const std::set<Spanner*> dummy;
        return dummy;
    }
    return score()->unmanagedSpanners();
}

// =============================================================
// LayoutDebug
// =============================================================
LayoutDebug* LayoutDebug::instance()
{
    static LayoutDebug ld;
    return &ld;
}

void LayoutDebug::callClear()
{
    m_currentCall = nullptr;
    m_calls.clear();
}

void LayoutDebug::callBegin(const std::string_view& name, const std::string_view& info)
{
    //LOGDA() << name << " " << info;
    Call c;
    c.name = name;
    c.info = info;
    if (m_currentCall) {
        c.top = m_currentCall;
        m_currentCall->nested.push_back(c);
        m_currentCall = &m_currentCall->nested.back();
    } else {
        m_calls.push_back(c);
        m_currentCall = &m_calls.back();
    }
}

void LayoutDebug::callEnd()
{
    IF_ASSERT_FAILED(m_currentCall) {
        return;
    }

    //LOGDA() << m_currentCall->name << " " << m_currentCall->info;
    m_currentCall = m_currentCall->top;
}

void LayoutDebug::callDump(const LayoutDebug::Call& c, std::stringstream& ss, int& indent)
{
    std::string gap;
    gap.resize(indent * 2, ' ');
    ss << "\n";
    ss << gap << c.name << " " << c.info;

    for (const Call& cn : c.nested) {
        ++indent;
        callDump(cn, ss, indent);
        --indent;
    }
}

void LayoutDebug::callDump(std::stringstream& ss) const
{
    for (const Call& c : m_calls) {
        int indent = 0;
        callDump(c, ss, indent);
    }
}

void LayoutDebug::callPrint()
{
    LOGDA() << "\n" << callDump();
}

// =============================================================
// LayoutContext
// =============================================================

LayoutContext::LayoutContext(Score* score)
    : m_score(score), m_configuration(this), m_dom(this)
{
    if (score) {
        m_state.setFirstSystemIndent(score->style().styleB(Sid::enableIndentationOnFirstSystem));
    }
}

LayoutContext::~LayoutContext()
{
    for (Spanner* s : m_state.processedSpanners()) {
        TLayout::layoutSystemsDone(s);
    }

    for (MuseScoreView* v : m_score->getViewer()) {
        v->layoutChanged();
    }
}

bool LayoutContext::isValid() const
{
    return m_score != nullptr;
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

DomAccessor& LayoutContext::mutDom()
{
    return m_dom;
}

const LayoutState& LayoutContext::state() const
{
    return m_state;
}

LayoutState& LayoutContext::mutState()
{
    return m_state;
}

void LayoutContext::setLayout(const Fraction& tick1, const Fraction& tick2, staff_idx_t staff1, staff_idx_t staff2, const EngravingItem* e)
{
    IF_ASSERT_FAILED(m_score) {
        return;
    }
    m_score->setLayout(tick1, tick2, staff1, staff2, e);
}

void LayoutContext::addRefresh(const RectF& r)
{
    IF_ASSERT_FAILED(m_score) {
        return;
    }
    m_score->addRefresh(r);
}

const Selection& LayoutContext::selection() const
{
    IF_ASSERT_FAILED(m_score) {
        static const Selection dummy;
        return dummy;
    }
    return m_score->selection();
}

void LayoutContext::select(EngravingItem* item, SelectType type, staff_idx_t staff)
{
    IF_ASSERT_FAILED(m_score) {
        return;
    }
    m_score->select(item, type, staff);
}

void LayoutContext::deselect(EngravingItem* el)
{
    IF_ASSERT_FAILED(m_score) {
        return;
    }
    m_score->deselect(el);
}
