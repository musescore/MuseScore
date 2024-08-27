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

#ifndef MU_ENGRAVING_LYRICS_H
#define MU_ENGRAVING_LYRICS_H

#include "../types/types.h"
#include "line.h"
#include "textbase.h"

namespace mu::engraving {
//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

class LyricsLine;

class Lyrics final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, Lyrics)
    DECLARE_CLASSOF(ElementType::LYRICS)

public:

    // MELISMA FIRST UNDERSCORE:
    // used as_ticks value to mark a melisma for which only the first chord has been spanned so far
    // and to give the user a visible feedback that the underscore has been actually entered;
    // it should be cleared to 0 at some point, so that it will not be carried over
    // if the melisma is not extended beyond a single chord, but no suitable place to do this
    // has been identified yet.
    static constexpr Fraction TEMP_MELISMA_TICKS = Fraction::fromTicks(1); // THIS WAS A HORRIBLE HACK. At some point we must remove it and replace it with a proper solution. (M.S.)

    ~Lyrics();

    Lyrics* clone() const override { return new Lyrics(*this); }
    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    Segment* segment() const { return toSegment(explicitParent()->explicitParent()); }
    Measure* measure() const { return toMeasure(explicitParent()->explicitParent()->explicitParent()); }
    ChordRest* chordRest() const { return toChordRest(explicitParent()); }

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    int subtype() const override { return m_no; }
    TranslatableString subtypeUserName() const override;
    void setNo(int n) { m_no = n; }
    int no() const { return m_no; }
    bool isEven() const { return m_no % 2; }
    void setSyllabic(LyricsSyllabic s) { m_syllabic = s; }
    LyricsSyllabic syllabic() const { return m_syllabic; }
    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;
    bool isEditAllowed(EditData&) const override;
    void endEdit(EditData&) override;

    const Fraction& ticks() const { return m_ticks; }
    void setTicks(const Fraction& tick) { m_ticks = tick; }
    Fraction endTick() const;
    void removeFromScore();

    void adjustPrevious();

    bool needRemoveInvalidSegments() const { return m_needRemoveInvalidSegments; }
    void setNeedRemoveInvalidSegments() { m_needRemoveInvalidSegments = true; }
    void removeInvalidSegments();

    LyricsLine* separator() const { return m_separator; }
    void setSeparator(LyricsLine* s) { m_separator = s; }

    bool isMelisma() const;

    bool allowTimeAnchor() const override { return false; }

    using EngravingObject::undoChangeProperty;
    void paste(EditData& ed, const String& txt) override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;
    void triggerLayout() const override;

    double yRelativeToStaff() const;
    void setYRelativeToStaff(double y);

protected:
    int m_no = 0;  // row index

private:

    friend class Factory;
    Lyrics(ChordRest* parent);
    Lyrics(const Lyrics&);

    void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;

    Fraction m_ticks;          // if > 0 then draw an underline to tick() + _ticks (melisma)
    LyricsSyllabic m_syllabic = LyricsSyllabic::SINGLE;
    LyricsLine* m_separator = nullptr;
    bool m_needRemoveInvalidSegments = false;
};

//---------------------------------------------------------
//   LyricsLine
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

class LyricsLine final : public SLine
{
    OBJECT_ALLOCATOR(engraving, LyricsLine)
    DECLARE_CLASSOF(ElementType::LYRICSLINE)

public:
    LyricsLine(EngravingItem* parent);
    LyricsLine(const LyricsLine&);

    LyricsLine* clone() const override { return new LyricsLine(*this); }

    LineSegment* createLineSegment(System* parent) override;
    void removeUnmanaged() override;
    void styleChanged() override;

    Lyrics* lyrics() const { return toLyrics(explicitParent()); }
    Lyrics* nextLyrics() const { return m_nextLyrics; }
    void setNextLyrics(Lyrics* l) { m_nextLyrics = l; }
    bool isEndMelisma() const { return lyrics() && lyrics()->ticks().isNotZero(); }
    bool isDash() const { return !isEndMelisma(); }
    bool setProperty(Pid propertyId, const PropertyValue& v) override;

protected:
    Lyrics* m_nextLyrics = nullptr;

    void doComputeEndElement() override;
};

//---------------------------------------------------------
//   LyricsLineSegment
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

class LyricsLineSegment final : public LineSegment
{
    OBJECT_ALLOCATOR(engraving, LyricsLineSegment)
    DECLARE_CLASSOF(ElementType::LYRICSLINE_SEGMENT)

public:
    LyricsLineSegment(LyricsLine*, System* parent);

    LyricsLineSegment* clone() const override { return new LyricsLineSegment(*this); }

    LyricsLine* lyricsLine() const { return toLyricsLine(spanner()); }
    Lyrics* lyrics() const { return lyricsLine()->lyrics(); }

    double baseLineShift() const;

    struct LayoutData : public LineSegment::LayoutData {
    public:
        const std::vector<LineF>& dashes() const { return m_dashes; }
        void clearDashes() { m_dashes.clear(); }
        void addDash(const LineF& dash) { m_dashes.push_back(dash); }
    private:
        std::vector<LineF> m_dashes;
    };
    DECLARE_LAYOUTDATA_METHODS(LyricsLineSegment)
};
} // namespace mu::engraving
#endif
