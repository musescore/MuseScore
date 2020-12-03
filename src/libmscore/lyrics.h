//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

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
    Q_ENUM(Syllabic);

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

    bool isMelisma() const;
    void undoChangeProperty(Pid id, const QVariant&, PropertyFlags ps) override;

protected:
    int _no;                  ///< row index
    bool _even;

public:
    Lyrics(Score* = 0);
    Lyrics(const Lyrics&);
    ~Lyrics();

    Lyrics* clone() const override { return new Lyrics(*this); }
    ElementType type() const override { return ElementType::LYRICS; }
    bool acceptDrop(EditData&) const override;
    Element* drop(EditData&) override;

    Segment* segment() const { return toSegment(parent()->parent()); }
    Measure* measure() const { return toMeasure(parent()->parent()->parent()); }
    ChordRest* chordRest() const { return toChordRest(parent()); }

    void layout() override;
    void layout2(int);

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
    void add(Element*) override;
    void remove(Element*) override;
    void endEdit(EditData&) override;

    Fraction ticks() const { return _ticks; }
    void setTicks(const Fraction& tick) { _ticks = tick; }
    Fraction endTick() const;
    void removeFromScore();

    using ScoreElement::undoChangeProperty;
    void paste(EditData&) override;

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid id) const override;
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
    LyricsLine(Score*);
    LyricsLine(const LyricsLine&);

    LyricsLine* clone() const override { return new LyricsLine(*this); }
    ElementType type() const override { return ElementType::LYRICSLINE; }
    void layout() override;
    LineSegment* createLineSegment() override;
    void removeUnmanaged() override;
    void styleChanged() override;

    Lyrics* lyrics() const { return toLyrics(parent()); }
    Lyrics* nextLyrics() const { return _nextLyrics; }
    bool isEndMelisma() const { return lyrics()->ticks().isNotZero(); }
    bool isDash() const { return !isEndMelisma(); }
    bool setProperty(Pid propertyId, const QVariant& v) override;
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
    LyricsLineSegment(Spanner*, Score*);

    LyricsLineSegment* clone() const override { return new LyricsLineSegment(*this); }
    ElementType type() const override { return ElementType::LYRICSLINE_SEGMENT; }
    void draw(QPainter*) const override;
    void layout() override;
    // helper functions
    LyricsLine* lyricsLine() const { return toLyricsLine(spanner()); }
    Lyrics* lyrics() const { return lyricsLine()->lyrics(); }
};
}     // namespace Ms
#endif
