/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "articulationpopupmodel.h"

#include <array>
#include <span>
#include <vector>

#include "containers.h"

#include "engraving/dom/articulation.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/score.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;

namespace {
struct ArticulationTypeItem {
    SymId above;
    SymId below;
    muse::TranslatableString name;
    // Mirrors Articulation::computeCategories() for exactly this popup's 9 supported types, so
    // a target type can be checked for semantic overlap against a chord's OTHER (real, already
    // laid out) articulations via their own isStaccato()/isAccent()/etc. accessors, without
    // needing to construct a throwaway Articulation just to query categories for a hypothetical
    // SymId. Note Staccatissimo intentionally carries no STACCATO overlap - computeCategories()
    // doesn't consider it part of that category either.
    ArticulationCategories categories;
};

// `name` is the plain type name, independent of the above/below placement (which this popup
// never changes), reusing the same translated strings as the add-staccato/add-tenuto/etc.
// actions where they already exist.

// Page 0 (default): the 5 basic articulation types, in this fixed order.
const std::array<ArticulationTypeItem, 5> BASE_PAGE_TYPES = { {
    { SymId::articMarcatoAbove, SymId::articMarcatoBelow, muse::TranslatableString("action", "Marcato"), ArticulationCategory::MARCATO },
    { SymId::articAccentAbove, SymId::articAccentBelow, muse::TranslatableString("action", "Accent"), ArticulationCategory::ACCENT },
    { SymId::articTenutoAbove, SymId::articTenutoBelow, muse::TranslatableString("action", "Tenuto"), ArticulationCategory::TENUTO },
    { SymId::articStaccatoAbove, SymId::articStaccatoBelow, muse::TranslatableString("action", "Staccato"),
      ArticulationCategory::STACCATO },
    { SymId::articStaccatissimoAbove, SymId::articStaccatissimoBelow, muse::TranslatableString("engraving/sym", "Staccatissimo"),
      ArticulationCategory::NONE },
} };

// Page 1: the 5 "double" combos, reached by paginating left from the base page.
const std::array<ArticulationTypeItem, 5> COMBO_PAGE_TYPES = { {
    { SymId::articAccentStaccatoAbove, SymId::articAccentStaccatoBelow, muse::TranslatableString("engraving/sym", "Accent-staccato"),
      ArticulationCategory::ACCENT | ArticulationCategory::STACCATO },
    { SymId::articTenutoAccentAbove, SymId::articTenutoAccentBelow, muse::TranslatableString("engraving/sym", "Tenuto-accent"),
      ArticulationCategory::TENUTO | ArticulationCategory::ACCENT },
    { SymId::articTenutoStaccatoAbove, SymId::articTenutoStaccatoBelow, muse::TranslatableString("engraving/sym", "Tenuto-staccato"),
      ArticulationCategory::TENUTO | ArticulationCategory::STACCATO },
    { SymId::articMarcatoStaccatoAbove, SymId::articMarcatoStaccatoBelow, muse::TranslatableString("engraving/sym", "Marcato-staccato"),
      ArticulationCategory::MARCATO | ArticulationCategory::STACCATO },
    { SymId::articMarcatoTenutoAbove, SymId::articMarcatoTenutoBelow, muse::TranslatableString("engraving/sym", "Marcato-tenuto"),
      ArticulationCategory::MARCATO | ArticulationCategory::TENUTO },
} };

bool containsAbove(std::span<const ArticulationTypeItem> types, SymId symId)
{
    for (const ArticulationTypeItem& item : types) {
        if (item.above == symId) {
            return true;
        }
    }
    return false;
}

bool containsEither(std::span<const ArticulationTypeItem> types, SymId symId)
{
    for (const ArticulationTypeItem& item : types) {
        if (item.above == symId || item.below == symId) {
            return true;
        }
    }
    return false;
}

bool isAboveSymId(SymId symId)
{
    return containsAbove(BASE_PAGE_TYPES, symId) || containsAbove(COMBO_PAGE_TYPES, symId);
}

ArticulationCategories categoriesFor(SymId symId)
{
    for (const ArticulationTypeItem& item : BASE_PAGE_TYPES) {
        if (item.above == symId || item.below == symId) {
            return item.categories;
        }
    }
    for (const ArticulationTypeItem& item : COMBO_PAGE_TYPES) {
        if (item.above == symId || item.below == symId) {
            return item.categories;
        }
    }
    return ArticulationCategory::NONE;
}

ArticulationCategories categoriesOf(const Articulation* item)
{
    ArticulationCategories result = ArticulationCategory::NONE;
    if (item->isStaccato()) {
        result |= ArticulationCategory::STACCATO;
    }
    if (item->isAccent()) {
        result |= ArticulationCategory::ACCENT;
    }
    if (item->isMarcato()) {
        result |= ArticulationCategory::MARCATO;
    }
    if (item->isTenuto()) {
        result |= ArticulationCategory::TENUTO;
    }
    return result;
}

// True if two marks with these categories shouldn't coexist on the same chord: either they share
// a base category (e.g. both imply staccato - an existing plain Staccato conflicts with a target
// of Accent-Staccato, which already implies staccato), or one is accent-flavored and the other
// marcato-flavored. The latter isn't a shared-category case (ACCENT and MARCATO are disjoint in
// computeCategories()) - it mirrors Chord::updateArticulations(), which hardcodes accent and
// marcato as always mutually exclusive on a chord, independent of the category system.
bool categoriesConflict(ArticulationCategories a, ArticulationCategories b)
{
    if (a & b) {
        return true;
    }
    return (a & ArticulationCategory::ACCENT && b & ArticulationCategory::MARCATO)
           || (a & ArticulationCategory::MARCATO && b & ArticulationCategory::ACCENT);
}

QVariantList buildPage(std::span<const ArticulationTypeItem> types, bool above, const IEngravingFontPtr& engravingFont)
{
    QVariantList page;
    for (const ArticulationTypeItem& item : types) {
        SymId symId = above ? item.above : item.below;
        page.append(QVariantMap {
                { "symId", static_cast<int>(symId) },
                { "text", engravingFont->toString(symId).toQString() },
                { "accessibleName", item.name.translated().toQString() },
            });
    }
    return page;
}
} // namespace

ArticulationPopupModel::ArticulationPopupModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_ARTICULATION, parent)
{
}

bool ArticulationPopupModel::canOpen(const EngravingItem* element)
{
    if (!element || !element->isArticulation()) {
        return false;
    }

    SymId symId = toArticulation(element)->symId();
    return containsEither(BASE_PAGE_TYPES, symId) || containsEither(COMBO_PAGE_TYPES, symId);
}

QString ArticulationPopupModel::fontFamily() const
{
    IF_ASSERT_FAILED(m_item) {
        return QString();
    }

    return QString::fromStdString(m_item->score()->engravingFont()->family());
}

QVariantList ArticulationPopupModel::pages() const
{
    return m_pages;
}

void ArticulationPopupModel::init()
{
    AbstractElementPopupModel::init();

    connect(this, &AbstractElementPopupModel::dataChanged, this, [this]() {
        load();
    });

    load();
}

void ArticulationPopupModel::load()
{
    if (!m_item || !m_item->isArticulation()) {
        if (!m_pages.isEmpty()) {
            m_pages.clear();
            emit pagesChanged();
        }
        return;
    }

    bool above = isAboveSymId(toArticulation(m_item)->symId());

    // dataChanged fires for any ARTICULATION change in the score, not just this popup's own
    // item, so most reloads are redundant - skip rebuilding (and re-emitting, which would force
    // the QML Repeater to tear down and recreate its delegates) when nothing actually changed.
    if (!m_pages.isEmpty() && above == m_pagesAbove) {
        return;
    }
    m_pagesAbove = above;

    m_pages.clear();
    IEngravingFontPtr engravingFont = m_item->score()->engravingFont();

    // QVariant::fromValue is required here: QVariantList::append(const QVariantList&) resolves
    // to QList's "concatenate" overload and would flatten each page into m_pages instead of
    // nesting it, since a plain QVariantList argument is an exact-type match for that overload.
    m_pages.append(QVariant::fromValue(buildPage(BASE_PAGE_TYPES, above, engravingFont)));
    m_pages.append(QVariant::fromValue(buildPage(COMBO_PAGE_TYPES, above, engravingFont)));

    emit pagesChanged();
}

void ArticulationPopupModel::changeArticulation(int page, int index)
{
    IF_ASSERT_FAILED(m_item && m_item->isArticulation() && page >= 0 && page < m_pages.size()) {
        return;
    }

    QVariantList pageItems = m_pages[page].toList();

    IF_ASSERT_FAILED(index >= 0 && index < pageItems.size()) {
        return;
    }

    SymId newSymId = static_cast<SymId>(pageItems[index].toMap().value("symId").toInt());
    Articulation* articulation = toArticulation(m_item);

    // Guard against operating on an item this popup already removed from its chord: after
    // undoRemoveElement below, the C++ object stays alive (owned by the pending undo command)
    // and m_item keeps pointing at it, so a stale reload or a second click on the same button
    // could otherwise reach undoRemoveElement a second time on the same pointer.
    ChordRest* ownerChordRest = articulation->chordRest();
    if (!ownerChordRest || !ownerChordRest->isChord()
        || !muse::contains(toChord(ownerChordRest)->articulations(), articulation)) {
        return;
    }

    Chord* chord = toChord(ownerChordRest);

    if (newSymId == articulation->symId()) {
        // Clicking the type that's already applied removes it, matching the toolbar/shortcut
        // toggle behavior instead of silently doing nothing.
        beginCommand(TranslatableString("undoableAction", "Remove articulation"));
        articulation->score()->undoRemoveElement(articulation);
        endCommand();

        // Nothing in this popup still applies to the chord it was opened on - select it instead
        // of leaving the popup open over a removed item, whether or not other marks remain.
        if (interaction()) {
            interaction()->select({ chord });
        }

        updateNotation();
        return;
    }

    // Resolve conflicts with the chord's OTHER marks instead of silently refusing the click, the
    // same way Chord::updateArticulations() replaces (rather than blocks) a conflicting existing
    // mark when the toolbar/shortcut path adds a new one - e.g. picking Accent-Staccato while a
    // separate plain Staccato is already present removes that redundant Staccato; picking Accent
    // while a separate Marcato is present removes the Marcato (they're always mutually exclusive).
    std::vector<Articulation*> conflicting;
    ArticulationCategories targetCategories = categoriesFor(newSymId);
    for (Articulation* other : chord->articulations()) {
        if (other != articulation && categoriesConflict(categoriesOf(other), targetCategories)) {
            conflicting.push_back(other);
        }
    }

    // setSymId() (called by the property change below) unconditionally resets the anchor to its
    // style default, so a manually-overridden placement needs to be captured and reapplied.
    ArticulationAnchor oldAnchor = articulation->anchor();
    PropertyFlags oldAnchorFlags = articulation->propertyFlags(Pid::ARTICULATION_ANCHOR);
    bool anchorWasOverridden = oldAnchorFlags == PropertyFlags::UNSTYLED;

    beginCommand(TranslatableString("undoableAction", "Change articulation"));
    for (Articulation* other : conflicting) {
        articulation->score()->undoRemoveElement(other);
    }
    articulation->undoChangeProperty(Pid::SYMBOL, PropertyValue::fromValue(newSymId));
    if (anchorWasOverridden) {
        articulation->undoChangeProperty(Pid::ARTICULATION_ANCHOR, int(oldAnchor), oldAnchorFlags);
    }
    endCommand();

    updateNotation();
}
