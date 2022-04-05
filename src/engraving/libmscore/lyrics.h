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

#include "line.h"
#include "text.h"

namespace Ms {
//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

class LyricsLine;

class Lyrics final : public TextBase
{
    Q_GADGET
public:
    enum class Syllabic : char {
        ///.\{
        SINGLE, BEGIN, END, MIDDLE
        ///\}
    };
    Q_ENUM(Syllabic)

    // MELISMA FIRST UNDERSCORE:
    // used as_ticks value to mark a melisma for which only the first chord has been spanned so far
    // and to give the user a visible feedback that the undercore has been actually entered;
    // it should be cleared to 0 at some point, so that it will not be carried over
    // if the melisma is not extended beyond a single chord, but no suitable place to do this
    // has been identified yet.
    static constexpr int TEMP_MELISMA_TICKS      = 1;

    // WORD_MIN_DISTANCE has never been implemented
    // static constexpr qreal  LYRICS_WORD_MIN_DISTANCE = 0.33;     // min. distance between lyrics from different words

private:
    Fraction _ticks;          ///< if > 0 then draw an underline to tick() + _ticks
                              ///< (melisma)
    Syllabic _syllabic;
    LyricsLine* _separator;

    friend class mu::engraving::Factory;
    Lyrics(ChordRest* parent);
    Lyrics(const Lyrics&);

    bool isMelisma() const;
    void undoChangeProperty(Pid id, const mu::engraving::PropertyValue&, PropertyFlags ps) override;

protected:
    int _no;                  ///< row index
    bool _even;

public:
    ~Lyrics();

    Lyrics* clone() const override { return new Lyrics(*this); }
    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    Segment* segment() const { return toSegment(explicitParent()->explicitParent()); }
    Measure* measure() const { return toMeasure(explicitParent()->explicitParent()->explicitParent()); }
    ChordRest* chordRest() const { return toChordRest(explicitParent()); }

    void layout() override;
    void layout2(int);

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;
    bool readProperties(XmlReader&) override;
    int subtype() const override { return _no; }
    QString subtypeName() const override { return QObject::tr("Verse %1").arg(_no + 1); }
    void setNo(int n) { _no = n; }
    int no() const { return _no; }
    bool isEven() const { return _no % 1; }
    void setSyllabic(Syllabic s) { _syllabic = s; }
    Syllabic syllabic() const { return _syllabic; }
    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;
    bool isEditAllowed(EditData&) const override;
    bool edit(EditData&) override;
    void endEdit(EditData&) override;

    Fraction ticks() const { return _ticks; }
    void setTicks(const Fraction& tick) { _ticks = tick; }
    Fraction endTick() const;
    void removeFromScore();

    using EngravingObject::undoChangeProperty;
    void paste(EditData& ed, const QString& txt) override;

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid id) const override;
};

//---------------------------------------------------------
//   LyricsLine
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

class LyricsLine final : public SLine
{
protected:
    Lyrics* _nextLyrics;

public:
    LyricsLine(EngravingItem* parent);
    LyricsLine(const LyricsLine&);

    LyricsLine* clone() const override { return new LyricsLine(*this); }
    void layout() override;
    LineSegment* createLineSegment(System* parent) override;
    void removeUnmanaged() override;
    void styleChanged() override;

    Lyrics* lyrics() const { return toLyrics(explicitParent()); }
    Lyrics* nextLyrics() const { return _nextLyrics; }
    bool isEndMelisma() const { return lyrics()->ticks().isNotZero(); }
    bool isDash() const { return !isEndMelisma(); }
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue& v) override;
    SpannerSegment* layoutSystem(System*) override;
};

//---------------------------------------------------------
//   LyricsLineSegment
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

class LyricsLineSegment final : public LineSegment
{
protected:
    int _numOfDashes = 0;
    qreal _dashLength = 0;

public:
    LyricsLineSegment(LyricsLine*, System* parent);

    LyricsLineSegment* clone() const override { return new LyricsLineSegment(*this); }
    void draw(mu::draw::Painter*) const override;
    void layout() override;
    // helper functions
    LyricsLine* lyricsLine() const { return toLyricsLine(spanner()); }
    Lyrics* lyrics() const { return lyricsLine()->lyrics(); }
};
}     // namespace Ms
#endif
