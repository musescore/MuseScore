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

#ifndef __LYRICS_H__
#define __LYRICS_H__

#include "../types/types.h"
#include "line.h"
#include "textbase.h"

namespace mu::engraving::layout::v0 {
class LyricsLayout;
}

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
    static constexpr int TEMP_MELISMA_TICKS      = 1;

    // WORD_MIN_DISTANCE has never been implemented
    // static constexpr double  LYRICS_WORD_MIN_DISTANCE = 0.33;     // min. distance between lyrics from different words

public:
    ~Lyrics();

    Lyrics* clone() const override { return new Lyrics(*this); }
    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    Segment* segment() const { return toSegment(explicitParent()->explicitParent()); }
    Measure* measure() const { return toMeasure(explicitParent()->explicitParent()->explicitParent()); }
    ChordRest* chordRest() const { return toChordRest(explicitParent()); }

    void layout2(int);

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    int subtype() const override { return _no; }
    TranslatableString subtypeUserName() const override;
    void setNo(int n) { _no = n; }
    int no() const { return _no; }
    bool isEven() const { return _no % 2; }
    void setSyllabic(LyricsSyllabic s) { _syllabic = s; }
    LyricsSyllabic syllabic() const { return _syllabic; }
    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;
    bool isEditAllowed(EditData&) const override;
    void endEdit(EditData&) override;

    Fraction ticks() const { return _ticks; }
    void setTicks(const Fraction& tick) { _ticks = tick; }
    Fraction endTick() const;
    void removeFromScore();
    void setRemoveInvalidSegments() { _removeInvalidSegments = true; }
    void adjustPrevious();

    using EngravingObject::undoChangeProperty;
    void paste(EditData& ed, const String& txt) override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;
    void triggerLayout() const override;

protected:
    int _no;                  ///< row index
    bool _even;

private:

    friend class layout::v0::LyricsLayout;
    friend class Factory;
    Lyrics(ChordRest* parent);
    Lyrics(const Lyrics&);

    bool isMelisma() const;
    void removeInvalidSegments();
    void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;

    Fraction _ticks;          ///< if > 0 then draw an underline to tick() + _ticks
                              ///< (melisma)
    LyricsSyllabic _syllabic;
    LyricsLine* _separator;
    bool _removeInvalidSegments = false;
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
    Lyrics* nextLyrics() const { return _nextLyrics; }
    bool isEndMelisma() const { return lyrics() && lyrics()->ticks().isNotZero(); }
    bool isDash() const { return !isEndMelisma(); }
    bool setProperty(Pid propertyId, const PropertyValue& v) override;

protected:
    friend class layout::v0::LyricsLayout;

    Lyrics* _nextLyrics;
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
    void draw(mu::draw::Painter*) const override;

    // helper functions
    LyricsLine* lyricsLine() const { return toLyricsLine(spanner()); }
    Lyrics* lyrics() const { return lyricsLine()->lyrics(); }

protected:
    friend class layout::v0::LyricsLayout;
    int _numOfDashes = 0;
    double _dashLength = 0;
};
} // namespace mu::engraving
#endif
