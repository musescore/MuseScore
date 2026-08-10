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

#include <gtest/gtest.h>

#include "engraving/compat/dummyelement.h"

#include "engraving/dom/beam.h"
#include "engraving/dom/box.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/page.h"
#include "engraving/dom/rootitem.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/system.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

static const String PARENT_DATA_DIR("all_elements_data/");

//! Tests for the two distinct parent relations an engraving item has:
//!  - ownership: who the item is attached to (null while it is not attached to anything);
//!  - layout: whose coordinate system the item is positioned in.
//! These usually coincide, but deliberately diverge for pages, systems and
//! measures (owned by the score, placed on each other), and for items that float
//! above them (spanner segments, beams).
class Engraving_ParentTests : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_score = ScoreRW::readScore(PARENT_DATA_DIR + u"layout_elements.mscx");
        ASSERT_TRUE(m_score);
    }

    void TearDown() override
    {
        delete m_score;
        m_score = nullptr;
    }

    //! An arbitrary real chord/rest segment to attach test items to.
    Segment* someSegment() const
    {
        Measure* measure = m_score->firstMeasure();
        return measure ? measure->first(SegmentType::ChordRest) : nullptr;
    }

    static bool isChildOf(const EngravingObject* child, const EngravingObject* parent)
    {
        return muse::contains(parent->children(), const_cast<EngravingObject*>(child));
    }

    //! Follows layoutParent() up to the top, with a depth limit so that a cycle
    //! fails the test instead of hanging it. Returns the topmost ancestor.
    static const EngravingItem* topLayoutAncestor(const EngravingItem* item)
    {
        constexpr int MAX_DEPTH = 32;

        const EngravingItem* top = item;
        for (int depth = 0; depth < MAX_DEPTH; ++depth) {
            const EngravingItem* parent = top->layoutParent();
            if (!parent) {
                return top;
            }
            top = parent;
        }

        return nullptr;
    }

    MasterScore* m_score = nullptr;
};

// ---------------------------------------------------------------------------
// Ownership: constructing, attaching, detaching
//
// These tests set the parent link directly rather than going through the
// score-editing API, because the parent link itself is what is under test.
// ---------------------------------------------------------------------------

//! A freshly constructed item is not attached to anything: the constructor
//! argument only supplies context (above all, the score). It does leave the item
//! in that parent's child list, but the item does not report it as its parent.
TEST_F(Engraving_ParentTests, newItemIsNotAttached)
{
    Segment* segment = someSegment();
    ASSERT_TRUE(segment);

    Dynamic* dynamic = Factory::createDynamic(segment, false /*isAccessibleEnabled*/);

    EXPECT_EQ(dynamic->score(), m_score);

    EXPECT_EQ(dynamic->parent(), segment);
    EXPECT_TRUE(isChildOf(dynamic, segment));

    EXPECT_EQ(dynamic->ownershipParent(), nullptr);
    EXPECT_EQ(dynamic->ownershipParentItem(), nullptr);
    EXPECT_EQ(dynamic->layoutParent(), nullptr);

    delete dynamic;
}

TEST_F(Engraving_ParentTests, attachingAndDetaching)
{
    Segment* segment = someSegment();
    ASSERT_TRUE(segment);

    Dynamic* dynamic = Factory::createDynamic(segment, false /*isAccessibleEnabled*/);

    dynamic->setOwnershipParent(segment);

    EXPECT_EQ(dynamic->parent(), segment);
    EXPECT_EQ(dynamic->ownershipParent(), segment);
    EXPECT_EQ(dynamic->ownershipParentItem(), segment);
    EXPECT_EQ(dynamic->layoutParent(), segment);
    EXPECT_TRUE(isChildOf(dynamic, segment));
    EXPECT_FALSE(isChildOf(dynamic, m_score->dummy()));

    dynamic->moveToDummy();

    EXPECT_EQ(dynamic->parent(), m_score->dummy());
    EXPECT_EQ(dynamic->ownershipParent(), nullptr);
    EXPECT_EQ(dynamic->ownershipParentItem(), nullptr);
    EXPECT_EQ(dynamic->layoutParent(), nullptr);
    EXPECT_TRUE(isChildOf(dynamic, m_score->dummy()));
    EXPECT_FALSE(isChildOf(dynamic, segment));

    delete dynamic;
}

//! Deleting an owner must not leave its children pointing at freed memory:
//! they are parked back on the dummy, i.e. become unattached again.
TEST_F(Engraving_ParentTests, deletingOwnerParksChildrenOnDummy)
{
    Segment* segment = someSegment();
    ASSERT_TRUE(segment);

    Chord* chord = Factory::createChord(segment, false /*isAccessibleEnabled*/);
    Note* note = Factory::createNote(chord, false /*isAccessibleEnabled*/);
    note->setOwnershipParent(chord);
    ASSERT_EQ(note->ownershipParent(), chord);

    delete chord;

    EXPECT_EQ(note->parent(), m_score->dummy());
    EXPECT_EQ(note->ownershipParent(), nullptr);

    delete note;
}

//! Accessibility walks the layout hierarchy, but unattached items (e.g. palette
//! items) must still reach the dummy so that they get an accessible ancestor.
TEST_F(Engraving_ParentTests, accessibleParentFallsBackToTheDummy)
{
    Segment* segment = someSegment();
    ASSERT_TRUE(segment);

    Dynamic* dynamic = Factory::createDynamic(segment, false /*isAccessibleEnabled*/);

    dynamic->setOwnershipParent(segment);
    EXPECT_EQ(dynamic->accessibleParentItem(), segment);

    dynamic->moveToDummy();

    EXPECT_EQ(dynamic->layoutParent(), nullptr);
    EXPECT_EQ(dynamic->accessibleParentItem(), m_score->dummy());

    delete dynamic;
}

// ---------------------------------------------------------------------------
// Layout hierarchy in a laid-out score
// ---------------------------------------------------------------------------

//! Pages, systems and measures are all owned by the score, and placed on each other:
//! a system does not own the measures on it, and a page does not own the systems on it.
TEST_F(Engraving_ParentTests, pagesSystemsAndMeasuresAreOwnedByTheScore)
{
    ASSERT_FALSE(m_score->pages().empty());

    Page* page = m_score->pages().front();
    ASSERT_FALSE(page->systems().empty());
    System* system = page->systems().front();
    ASSERT_FALSE(system->measures().empty());
    MeasureBase* measure = system->measures().front();

    // the score is not an EngravingItem, so it is an owner but never a layout parent
    EXPECT_EQ(page->ownershipParent(), m_score);
    EXPECT_EQ(page->ownershipParentItem(), nullptr);
    EXPECT_EQ(page->layoutParent(), nullptr);

    EXPECT_EQ(system->ownershipParent(), m_score);
    EXPECT_EQ(system->ownershipParentItem(), nullptr);
    EXPECT_EQ(system->page(), page);
    EXPECT_EQ(system->layoutParent(), page);

    EXPECT_EQ(measure->ownershipParent(), m_score);
    EXPECT_EQ(measure->ownershipParentItem(), nullptr);
    EXPECT_EQ(measure->system(), system);
    EXPECT_EQ(measure->layoutParent(), system);

    // positions accumulate along the placement chain, not the ownership one
    EXPECT_EQ(page->pagePos(), page->pos());
    EXPECT_EQ(system->pagePos(), system->pos());
    EXPECT_EQ(measure->pagePos(), measure->pos() + system->pos());
    EXPECT_EQ(measure->canvasPos(), measure->pagePos() + page->pos());
}

//! For an ordinary item, the layout parent is simply the owner.
TEST_F(Engraving_ParentTests, layoutParentDefaultsToTheOwner)
{
    Measure* measure = m_score->firstMeasure();
    ASSERT_TRUE(measure);
    Segment* segment = measure->first(SegmentType::ChordRest);
    ASSERT_TRUE(segment);
    ASSERT_TRUE(segment->element(0) && segment->element(0)->isChord());
    Chord* chord = toChord(segment->element(0));
    Note* note = chord->notes().front();

    EXPECT_EQ(note->ownershipParentItem(), chord);
    EXPECT_EQ(note->layoutParent(), chord);

    EXPECT_EQ(chord->ownershipParentItem(), segment);
    EXPECT_EQ(chord->layoutParent(), segment);

    EXPECT_EQ(segment->ownershipParentItem(), measure);
    EXPECT_EQ(segment->layoutParent(), measure);
}

//! A spanner segment is owned by its spanner, but placed on a system.
TEST_F(Engraving_ParentTests, spannerSegmentIsOwnedBySpannerAndPlacedOnSystem)
{
    Hairpin* hairpin = nullptr;
    for (const auto& pair : m_score->spanner()) {
        if (pair.second->isHairpin()) {
            hairpin = toHairpin(pair.second);
            break;
        }
    }
    ASSERT_TRUE(hairpin);
    ASSERT_FALSE(hairpin->spannerSegments().empty());

    SpannerSegment* spannerSegment = hairpin->frontSegment();
    ASSERT_TRUE(spannerSegment->system());

    EXPECT_EQ(spannerSegment->ownershipParent(), hairpin);
    EXPECT_EQ(spannerSegment->ownershipParentItem(), hairpin);
    EXPECT_EQ(spannerSegment->layoutParent(), spannerSegment->system());
    EXPECT_NE(spannerSegment->layoutParent(), spannerSegment->ownershipParentItem());
}

//! A system only places a segment, so a deleted segment has to take that placement
//! with it; otherwise the system is left holding a pointer to freed memory.
TEST_F(Engraving_ParentTests, deletingASegmentUnplacesItFromItsSystem)
{
    ASSERT_FALSE(m_score->systems().empty());
    System* system = m_score->systems().front();

    Hairpin* hairpin = Factory::createHairpin(m_score->dummy()->segment());
    hairpin->setTrack(0);
    hairpin->setTrack2(0);

    // placed on the system, but not handed to the spanner, so that deleting it does
    // not go through Spanner::eraseSpannerSegments() unplacing it first
    SpannerSegment* segment = hairpin->createLineSegment();
    segment->moveToSystem(system);
    ASSERT_TRUE(muse::contains(system->spannerSegments(), segment));

    delete segment;

    EXPECT_FALSE(muse::contains(system->spannerSegments(), segment));

    delete hairpin;
}

//! Owning the segments also means freeing them: they must not survive their spanner
//! by being unparented onto the dummy.
TEST_F(Engraving_ParentTests, deletingASpannerDeletesItsSegments)
{
    const size_t dummyChildrenBefore = m_score->dummy()->children().size();

    Hairpin* hairpin = Factory::createHairpin(m_score->dummy()->segment());
    hairpin->setTrack(0);
    hairpin->setTrack2(0);

    SpannerSegment* segment = hairpin->createLineSegment();
    hairpin->add(segment);
    ASSERT_EQ(segment->ownershipParent(), hairpin);

    delete hairpin;

    // the segment is gone with its spanner, rather than parked on the dummy
    EXPECT_EQ(m_score->dummy()->children().size(), dummyChildrenBefore);
}

//! A beam is placed on the system of the chords it beams, which is not its owner.
TEST_F(Engraving_ParentTests, beamIsPlacedOnTheSystemOfItsElements)
{
    Beam* beam = nullptr;
    for (Measure* measure = m_score->firstMeasure(); measure && !beam; measure = measure->nextMeasure()) {
        for (Segment& segment : measure->segments()) {
            if (!segment.isChordRestType()) {
                continue;
            }
            for (EngravingItem* item : segment.elist()) {
                if (item && item->isChordRest() && toChordRest(item)->beam()) {
                    beam = toChordRest(item)->beam();
                    break;
                }
            }
            if (beam) {
                break;
            }
        }
    }
    ASSERT_TRUE(beam);
    ASSERT_FALSE(beam->elements().empty());

    System* system = beam->elements().front()->measure()->system();
    ASSERT_TRUE(system);

    EXPECT_EQ(beam->layoutParent(), system);
    EXPECT_NE(beam->layoutParent(), beam->ownershipParentItem());
}

//! A frame nested inside another frame is not placed on a system of its own;
//! it falls back to being placed within its owner.
TEST_F(Engraving_ParentTests, nestedFrameIsPlacedWithinItsOwner)
{
    VBox* vbox = Factory::createVBox(m_score, false /*isAccessibleEnabled*/);
    HBox* hbox = Factory::createHBox(m_score, false /*isAccessibleEnabled*/);
    hbox->setOwnershipParent(vbox);

    ASSERT_EQ(hbox->system(), nullptr);
    EXPECT_EQ(hbox->ownershipParentItem(), vbox);
    EXPECT_EQ(hbox->layoutParent(), vbox);

    delete hbox;
    delete vbox;
}

// ---------------------------------------------------------------------------
// Whole-score invariants
// ---------------------------------------------------------------------------

//! Every laid-out item must be reachable from a page by following layoutParent(),
//! without cycles. This is what all position and ancestor lookups rely on.
TEST_F(Engraving_ParentTests, everyLaidOutItemReachesItsPage)
{
    std::vector<const EngravingItem*> unreachable;
    size_t checkedItems = 0;

    m_score->scanElements([&](EngravingItem* item) {
        if (!item->layoutParent() && !item->isPage()) {
            // not placed anywhere; nothing to verify
            return;
        }

        ++checkedItems;

        const EngravingItem* top = topLayoutAncestor(item);
        if (!top || !top->isPage() || !muse::contains(m_score->pages(), const_cast<Page*>(toPage(top)))) {
            unreachable.push_back(item);
            return;
        }

        // findAncestor() walks the same chain and must agree
        EXPECT_EQ(item->findAncestor(ElementType::PAGE), top) << item->typeName();

        // pageX() stops one step below the page, canvasX() does not
        EXPECT_DOUBLE_EQ(item->canvasX() - item->pageX(), top->x()) << item->typeName();
    });

    for (const EngravingItem* item : unreachable) {
        ADD_FAILURE() << item->typeName() << " does not reach a page through layoutParent()";
    }

    EXPECT_GT(checkedItems, 0u);
}

//! tick() and findMeasure() find their answer by walking layoutParent(); the
//! answer must match what the item is actually attached to.
TEST_F(Engraving_ParentTests, ancestorWalksAgreeWithAttachment)
{
    size_t checkedChordRests = 0;

    for (Measure* measure = m_score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        EXPECT_EQ(measure->findMeasure(), measure);

        for (Segment& segment : measure->segments()) {
            if (!segment.isChordRestType()) {
                continue;
            }

            for (EngravingItem* item : segment.elist()) {
                if (!item || !item->isChordRest()) {
                    continue;
                }

                ChordRest* chordRest = toChordRest(item);
                EXPECT_EQ(chordRest->tick(), segment.tick());
                EXPECT_EQ(chordRest->rtick(), segment.rtick());
                EXPECT_EQ(chordRest->findMeasure(), measure);
                EXPECT_EQ(chordRest->findAncestor(ElementType::SEGMENT), &segment);
                ++checkedChordRests;

                if (!chordRest->isChord()) {
                    continue;
                }

                for (Note* note : toChord(chordRest)->notes()) {
                    // a note has no tick of its own; it must find its segment's
                    EXPECT_EQ(note->tick(), segment.tick());
                    EXPECT_EQ(note->findMeasure(), measure);
                }
            }
        }
    }

    EXPECT_GT(checkedChordRests, 0u);
}

//! The accessibility tree mirrors the visual hierarchy, and a screen reader walks it in
//! both directions, so the two directions have to agree: the parent an item names must
//! list that item, and every item a parent lists must name that parent back. Keeping
//! that true wherever ownership and placement diverge is what accessibleChildren() is
//! for.
TEST_F(Engraving_ParentTests, accessibilityTreeAgreesInBothDirections)
{
    auto lists = [](const EngravingItem* parent, const EngravingItem* item) {
        return muse::contains(parent->accessibleChildren(), const_cast<EngravingItem*>(item));
    };

    // Upwards: whoever an item names must list it back
    size_t checkedItems = 0;

    auto checkNamedParent = [&](const EngravingItem* item) {
        const EngravingItem* parent = item->accessibleParentItem();
        if (!parent) {
            // nothing to disagree with
            return;
        }

        ++checkedItems;
        EXPECT_TRUE(lists(parent, item))
            << item->typeName() << " is not listed by the " << parent->typeName()
            << " it names as its accessibility parent";
    };

    m_score->scanElements([&](EngravingItem* item) { checkNamedParent(item); });

    // scanElements() visits what is laid out on the score, not the structure it is laid
    // out on, so walk that separately - it is where ownership and placement diverge
    for (const Page* page : m_score->pages()) {
        checkNamedParent(page);

        for (const System* system : page->systems()) {
            checkNamedParent(system);

            for (const MeasureBase* measure : system->measures()) {
                checkNamedParent(measure);
            }
        }
    }

    EXPECT_GT(checkedItems, 0u);

    // Downwards: whatever a parent lists must name it back. This is the direction a
    // screen reader descends, starting from the root item.
    constexpr int MAX_DEPTH = 64;
    size_t checkedChildren = 0;

    std::function<void(const EngravingItem*, int)> checkChildren = [&](const EngravingItem* parent, int depth) {
        ASSERT_LT(depth, MAX_DEPTH) << "the accessibility tree is too deep, or contains a cycle";

        for (const EngravingItem* child : parent->accessibleChildren()) {
            ++checkedChildren;
            EXPECT_EQ(child->accessibleParentItem(), parent)
                << parent->typeName() << " lists a " << child->typeName() << " that names another parent";

            checkChildren(child, depth + 1);
        }
    };

    checkChildren(m_score->rootItem(), 0);

    EXPECT_GT(checkedChildren, 0u);
}
