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

#ifndef MU_ENGRAVING_ARTICULATION_H
#define MU_ENGRAVING_ARTICULATION_H

#include <set>

#include "chordrest.h"
#include "engravingitem.h"

namespace mu::engraving {
class Factory;

class ChordRest;
class Measure;
class Page;
class Segment;
class System;

//---------------------------------------------------------
//   ArticulationInfo
//    gives infos about note attributes
//---------------------------------------------------------

enum class ArticulationCategory : char {
    NONE = 0x0,
    DOUBLE = 0x1,
    TENUTO = 0x2,
    STACCATO = 0x4,
    ACCENT = 0x8,
    MARCATO = 0x10,
    LUTE_FINGERING = 0x20,
    LAISSEZ_VIB = 0x40,
};
DECLARE_FLAGS(ArticulationCategories, ArticulationCategory)
DECLARE_OPERATORS_FOR_FLAGS(ArticulationCategories)

enum class ArticulationAnchor : char {
    TOP,        // attribute is always placed at top
    BOTTOM,     // attribute is placed at bottom
    AUTO,       // anchor depends on chord direction, away from stem
};

enum class ArticulationStemSideAlign : char
{
    // horizontal align for stem-side articulation layout
    STEM,                // attribute is placed directly over the stem
    NOTEHEAD,            // attribute is centered on the notehead
    AVERAGE,             // attribute is placed at the average of stem pos and notehead center
};

// flags:
enum class ArticulationShowIn : char {
    PITCHED_STAFF = 1, TABLATURE = 2
};

constexpr ArticulationShowIn operator|(ArticulationShowIn a1, ArticulationShowIn a2)
{
    return static_cast<ArticulationShowIn>(static_cast<unsigned char>(a1) | static_cast<unsigned char>(a2));
}

constexpr bool operator&(ArticulationShowIn a1, ArticulationShowIn a2)
{
    return static_cast<unsigned char>(a1) & static_cast<unsigned char>(a2);
}

enum class ArticulationsUpdateMode : unsigned char {
    Insert,
    Remove
};

std::set<SymId> updateArticulations(const std::set<SymId>& articulationSymbolIds, SymId articulationSymbolId,
                                    ArticulationsUpdateMode updateMode = ArticulationsUpdateMode::Insert);
std::set<SymId> splitArticulations(const std::set<SymId>& articulationSymbolIds);
std::set<SymId> joinArticulations(const std::set<SymId>& articulationSymbolIds);
std::set<SymId> flipArticulations(const std::set<SymId>& articulationSymbolIds, PlacementV placement);

//---------------------------------------------------------
//   @@ Articulation
///    articulation marks
//---------------------------------------------------------

class Articulation : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Articulation)
    DECLARE_CLASSOF(ElementType::ARTICULATION)

public:

    Articulation(const Articulation&) = default;
    Articulation& operator=(const Articulation&) = delete;

    virtual Articulation* clone() const override { return new Articulation(*this); }

    virtual double mag() const override;

    SymId symId() const { return m_symId; }
    void setSymId(SymId id);
    virtual int subtype() const override;

    void setTextType(ArticulationTextType textType);
    ArticulationTextType textType() const { return m_textType; }
    Text* text() const { return m_text; }
    void setText(Text* t) { m_text = t; }

    void setSelected(bool f) override;
    void setVisible(bool f) override;

    TranslatableString typeUserName() const override;
    TranslatableString subtypeUserName() const override;
    String articulationName() const;    // type-name of articulation; used for midi rendering
    static String symId2ArticulationName(SymId symId);

    bool layoutCloseToNote() const;

    bool isHiddenOnTabStaff() const;

    std::vector<LineF> dragAnchorLines() const override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;
    void resetProperty(Pid id) override;
    Sid getPropertyStyle(Pid id) const override;

    void setDirection(DirectionV d) { m_direction = d; }
    DirectionV direction() const { return m_direction; }

    ChordRest* chordRest() const;
    Segment* segment() const;
    Measure* measure() const;
    System* system() const;
    Page* page() const;

    ArticulationAnchor anchor() const { return m_anchor; }
    void setAnchor(ArticulationAnchor v) { m_anchor = v; }

    OrnamentStyle ornamentStyle() const { return m_ornamentStyle; }
    void setOrnamentStyle(OrnamentStyle val) { m_ornamentStyle = val; }

    bool playArticulation() const { return m_playArticulation; }
    void setPlayArticulation(bool val) { m_playArticulation = val; }

    String channelName() const { return m_channelName; }
    void setChannelName(const String& s) { m_channelName = s; }

    virtual String accessibleInfo() const override;

    bool isDouble() const { return m_categories & ArticulationCategory::DOUBLE; }
    bool isTenuto() const { return m_categories & ArticulationCategory::TENUTO; }
    bool isStaccato() const { return m_categories & ArticulationCategory::STACCATO; }
    bool isAccent() const { return m_categories & ArticulationCategory::ACCENT; }
    bool isMarcato() const { return m_categories & ArticulationCategory::MARCATO; }
    bool isLuteFingering() const { return m_categories & ArticulationCategory::LUTE_FINGERING; }
    bool isLaissezVib() const { return m_categories & ArticulationCategory::LAISSEZ_VIB; }

    bool isBasicArticulation() const;

    void styleChanged() override;

    bool isOnCrossBeamSide() const;

    staff_idx_t vStaffIdx() const override;

    struct LayoutData : public EngravingItem::LayoutData
    {
        ld_field<bool> up = { "[Articulation] up", true };
        ld_field<SymId> symId = { "[Articulation] symId", SymId::noSym };

        void reset() override
        {
            EngravingItem::LayoutData::reset();
            up.reset();
            symId.reset();
        }

        double opticalCenter() const;
    };
    DECLARE_LAYOUTDATA_METHODS(Articulation)

    void setUp(bool val);

    //! --- DEPRECATED ---
    bool up() const { return ldata()->up; }
    //! ------------------

protected:
    friend class mu::engraving::Factory;
    Articulation(ChordRest* parent, ElementType type = ElementType::ARTICULATION);

private:

    void setupShowOnTabStyles();

    enum class AnchorGroup : unsigned char {
        ARTICULATION,
        LUTE_FINGERING,
        OTHER
    };
    static AnchorGroup anchorGroup(SymId);

    void computeCategories();

    ArticulationCategories m_categories = ArticulationCategory::NONE;

    SymId m_symId = SymId::noSym;
    DirectionV m_direction = DirectionV::AUTO;
    String m_channelName;

    ArticulationTextType m_textType = ArticulationTextType::NO_TEXT;
    Text* m_text = nullptr;

    ArticulationAnchor m_anchor = ArticulationAnchor::AUTO;

    OrnamentStyle m_ornamentStyle = OrnamentStyle::DEFAULT;       // for use in ornaments such as trill
    bool m_playArticulation = true;

    std::pair<Sid, Sid> m_showOnTabStyles = { Sid::NOSTYLE, Sid::NOSTYLE };
};
} // namespace mu::engraving

#endif
