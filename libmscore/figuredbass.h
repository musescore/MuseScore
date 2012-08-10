//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: figuredbass.h 5526 2012-04-09 10:17:11Z lvinken $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __FIGUREDBASS_H__
#define __FIGUREDBASS_H__

#include "segment.h"
#include "text.h"

/*---------------------------------------------------------
NOTE ON ARCHITECTURE

FiguredBass elements are stored in the annotations of a Segment (like for instance Harmony)

FiguredBass is rather simple: it contains only _ticks, telling the duration of the element,
and a list of FiguredBassItem elements which do most of the job. It also maintains a text with the
normalized (made uniform) version of the text, which is used during editing.

Normally, a FiguredBass element is assumed to be styled with the FIGURED_BASS style and it is set
in this way upon creation.
- - - -
FiguredBassItem contains the actually f.b. info; it is made of 4 parts (in this order):
1) prefix: one of [nothing, doubleflat, flat, natural, sharp, doublesharp]
2) digit: one digit from 1 to 9
3) suffix: one of [nothing, doubleflat, flat, natural, sharp, doublesharp, plus, backslash, slash]
4) contLine: true if the item has a continuation line (whose length is determined by parent's _ticks)
and 5 parenthesis flags, one for each position before, between and after the four parts above:
each of them may contain one of [nothing, roundOpen, roundClosed, squaredOpen, squaredClosed].

There is a number of restrictions, implemented at the end of FiguredBassItem::parse().
Currently, no attempt is made to ensure that, if multiple parentheses are present, they are consistent
(matching open and closed parentheses is left to the user).

If an item cannot be parsed, the whole FiguredBass element is kept as entered, possibly un-styled.
If all items can be parsed, each item generates a display text from its properties,
lays it out so that it properly aligns under the chord, draws it at its proper location
and provides its FiguredBass parent with a normalized text for future editing.

FiguredBassItem has not use for formats (italics, bold, ...) and it is never edited directly;
more generally, it is never accessed directly, only via its FiguredBass parent;
so it is directly derived from Element and returns INVALID as type.

FiguredBass might require formatting (discouraged, but might be necessary for very uncommon cases)
and it is edited (via the normalized text); so it is derived from Text.
---------------------------------------------------------*/

//---------------------------------------------------------
//   @@ FiguredBassItem
//---------------------------------------------------------

class FiguredBass;

class FiguredBassItem : public Element {
      Q_OBJECT

      enum FBIAccidental {
            FBIAccidNone = 0,
            FBIAccidDoubleFlat,
            FBIAccidFlat,
            FBIAccidNatural,
            FBIAccidSharp,
            FBIAccidDoubleSharp,
            FBIAccidPlus,
            FBIAccidBackslash,
            FBIAccidSlash,
                  FBINumOfAccid
      };
      enum FBIParenthesis {
            FBIParenthNone    = 0,
            FBIParenthRoundOpen,
            FBIParenthRoundClosed,
            FBIParenthSquaredOpen,
            FBIParenthSquaredClosed,
                  FBINumOfParenth
      };

      static const QChar normParenthToChar[FBINumOfParenth];

      QString           _displayText;           // the constructed display text (read-only)
      int               ord;                    // the line ordinal of this element in the FB stack
      // the parts making a FiguredBassItem up
      FBIAccidental     prefix;                 // the accidental coming before the body
      int               digit;                  // the main digit (if present)
      FBIAccidental     suffix;                 // the accidental coming after the body
      bool              contLine;               // wether the item has continuation line or not
      FBIParenthesis    parenth[5];             // each of the parenthesis: before, between and after parts
      qreal             textWidth;              // the text width (in raster units), set during layout()
                                                //    used by draw()
      // part parsing
      int               parseDigit(QString& str);
      int               parseParenthesis(QString& str, int parenthIdx);
      int               parsePrefixSuffix(QString& str, bool bPrefix);

      void              setDisplayText(const QString& s)    { _displayText = s;       }

   public:
      FiguredBassItem(Score * score, int line);
      FiguredBassItem(const FiguredBassItem&);
      ~FiguredBassItem();

      FiguredBassItem &operator=(const FiguredBassItem&);

      // standard re-implemented virtual functions
      virtual FiguredBassItem*      clone() const     { return new FiguredBassItem(*this); }
      virtual ElementType           type() const      { return INVALID; }
      virtual void      draw(QPainter* painter) const;
      virtual void      layout();
      virtual void      read(const QDomElement&);
      virtual void      write(Xml& xml) const;

      // read / write MusicXML
      void              readMusicXML(const QDomElement& de, bool paren);
      void              writeMusicXML(Xml& xml) const;
      bool              startsWithParenthesis() const;

      // specific API
      const FiguredBass *    figuredBass() const      { return (FiguredBass*)(parent()); }
      bool              parse(QString& text);
      QString           normalizedText() const;
      QString           displayText() const           { return _displayText;    }

protected:

private:
      // read / write MusicXML support
      FiguredBassItem::FBIAccidental MusicXML2FBIAccidental(const QString prefix) const;
      QString                        FBIAccidental2MusicXML(FiguredBassItem::FBIAccidental prefix) const;
};

//---------------------------------------------------------
//   FiguredBassFont
//---------------------------------------------------------

struct FiguredBassFont {
      QString           family;
      QString           displayName;
      qreal             defPitch;
      qreal             defLineHeight;
      QChar             displayAccidental[6];
      QChar             displayParenthesis[5];
      QChar             displayDigit[2][10][4];

      bool read(const QDomElement&);
};

//---------------------------------------------------------
//   @@ FiguredBass
///   A complete figured bass indication
//
//    @P items    array[FiguredBassItem]  the list of individual items
//    @P onNote   bool                    whether it is placed on a note beginning or between notes (r/o)
//    @P ticks    int                     duration in ticks
//---------------------------------------------------------

class FiguredBass : public Text {
      Q_OBJECT

//      Q_PROPERTY(QDeclarativeListProperty<FiguredBassItem> items READ qmlItems)
      Q_PROPERTY(bool   onNote      READ onNote)
      Q_PROPERTY(int    ticks       READ ticks  WRITE setTicks)

      QList<FiguredBassItem>  items;            // the individual lines of the F.B.
      QVector<qreal>    _lineLenghts;           // lengths of duration indicator lines (in raster units)
      bool              _onNote;                // true if this element is on a staff note | false if it is betweee notes
      int               _ticks;                 // the duration (used for cont. lines and for multiple F.B.
                                                // under the same note)
      void              layoutLines();

   public:
      FiguredBass(Score*);
      FiguredBass(const FiguredBass&);
      ~FiguredBass();

      // a convenience static function to create/retrieve a new FiguredBass into/from its intended parent
      static FiguredBass *    addFiguredBassToSegment(Segment *seg, int track, int extTicks, bool *pNew);

      // static functions for font config files
      static bool       readConfigFile(const QString& fileName);
      static QList<QString>  fontNames();
      static bool       fontData(int nIdx, QString *pFamily, QString *pDisplayName,
                              qreal * pSize, qreal * pLineHeight);

      // standard re-implemented virtual functions
      virtual FiguredBass*    clone() const     { return new FiguredBass(*this); }
      virtual ElementType     type() const      { return FIGURED_BASS; }
      virtual void      draw(QPainter* painter) const;
      virtual void      endEdit();
      virtual void      layout();
      virtual void      read(const QDomElement&);
      virtual void      setSelected(bool f);
      virtual void      setVisible(bool f);
      virtual void      write(Xml& xml) const;

      // read / write MusicXML
      void              readMusicXML(const QDomElement& de, int divisions);
      void              writeMusicXML(Xml& xml) const;

      // getter /setters
//      void qmlItemsAppend(QDeclarativeListProperty<FiguredBassItem> *list, FiguredBassItem * pItem)
//                                                {     list->append(pItem);
//                                                      items.append(&pItem);
//                                                }
//      QDeclarativeListProperty<FiguredBassItem> qmlItems()
//                                                {     QList<FiguredBassItem*> list;
//                                                      foreach(FiguredBassItem item, items)
//                                                            list.append(&item);
//                                                      return QDeclarativeListProperty<FiguredBassItem>(this, &items, qmlItemsAppend);
//                                                }
      qreal             lineLength(int idx) const     {   if(_lineLenghts.size() > idx)
                                                            return _lineLenghts.at(idx);
                                                          return 0;   }
      bool              onNote() const          { return _onNote; }
      void              setOnNote(bool val)     { _onNote = val;  }
      Segment *         segment() const         { return static_cast<Segment*>(parent()); }
      int               ticks() const           { return _ticks;  }
      void              setTicks(int val)       { _ticks = val;   }

private:
      // read / write MusicXML support
      bool              hasParentheses() const;
      };

#endif

