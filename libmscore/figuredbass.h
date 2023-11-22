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

#ifndef __FIGUREDBASS_H__
#define __FIGUREDBASS_H__

#include "text.h"

namespace Ms {

class Segment;

/*---------------------------------------------------------
NOTE ON ARCHITECTURE

FiguredBass elements are stored in the annotations of a Segment (like for instance Harmony)

FiguredBass is rather simple: it contains only _ticks, telling the duration of the element,
and a list of FiguredBassItem elements which do most of the job. It also maintains a text with the
normalized (made uniform) version of the text, which is used during editing.

Normally, a FiguredBass element is assumed to be styled with an internally maintained text style
(based on the parameters of the general style "Figured Bass") FIGURED_BASS style and it is set
in this way upon creation and upon layout().
- - - -
FiguredBassItem contains the actually f.b. info; it is made of 4 parts (in this order):
1) prefix: one of [nothing, doubleflat, flat, natural, sharp, doublesharp, cross]
2) digit: one digit from 1 to 9
3) suffix: one of [nothing, doubleflat, flat, natural, sharp, doublesharp, cross, backslash, slash]
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

#define FBIDigitNone    -1

//---------------------------------------------------------
//   @@ FiguredBassItem
///   One line of a figured bass indication
//
//   @P continuationLine   enum (FiguredBassItem.NONE, .SIMPLE, .EXTENDED)  whether item has continuation line or not, and of which type
//   @P digit              int                              main digit(s) (0 - 9)
//   @P displayText        string                           text displayed (depends on configured fonts) (read only)
//   @P normalizedText     string                           conventional textual representation of item properties (= text used during input) (read only)
//   @P parenthesis1       enum (FiguredBassItem.NONE, .ROUNDOPEN, .ROUNDCLOSED, .SQUAREDOPEN, .SQUAREDCLOSED)  parenthesis before the prefix
//   @P parenthesis2       enum (FiguredBassItem.NONE, .ROUNDOPEN, .ROUNDCLOSED, .SQUAREDOPEN, .SQUAREDCLOSED)  parenthesis after the prefix / before the digit
//   @P parenthesis3       enum (FiguredBassItem.NONE, .ROUNDOPEN, .ROUNDCLOSED, .SQUAREDOPEN, .SQUAREDCLOSED)  parenthesis after the digit / before the suffix
//   @P parenthesis4       enum (FiguredBassItem.NONE, .ROUNDOPEN, .ROUNDCLOSED, .SQUAREDOPEN, .SQUAREDCLOSED)  parenthesis after the suffix / before the cont. line
//   @P parenthesis5       enum (FiguredBassItem.NONE, .ROUNDOPEN, .ROUNDCLOSED, .SQUAREDOPEN, .SQUAREDCLOSED)  parenthesis after the cont. line
//   @P prefix             enum (FiguredBassItem.NONE, .DOUBLEFLAT, .FLAT, .NATURAL, .SHARP, .DOUBLESHARP, .PLUS, .BACKSLASH, .SLASH)  accidental before the digit
//   @P suffix             enum (FiguredBassItem.NONE, .DOUBLEFLAT, .FLAT, .NATURAL, .SHARP, .DOUBLESHARP, .PLUS, .BACKSLASH, .SLASH)  accidental/diacritic after the digit
//---------------------------------------------------------

class FiguredBass;

class FiguredBassItem final : public Element {
   public:
      enum class Modifier : char {
            NONE = 0,
            DOUBLEFLAT,
            FLAT,
            NATURAL,
            SHARP,
            DOUBLESHARP,
            CROSS,
            BACKSLASH,
            SLASH,
                  NUMOF
            };
      enum class Parenthesis : char {
            NONE = 0,
            ROUNDOPEN,
            ROUNDCLOSED,
            SQUAREDOPEN,
            SQUAREDCLOSED,
                  NUMOF
            };
      enum class ContLine : char {
            NONE = 0,
            SIMPLE,                     // cont. line stops at f.b. element end
            EXTENDED                    // cont. line joins with next element, if possible
            };

      enum class Style : char {
            MODERN = 0,
            HISTORIC,
                  NUMOF
            };
      enum class Combination : char {
            SIMPLE = 0,
            CROSSED,
            BACKSLASHED,
            SLASHED,
                  NUMOF
            };

   private:

      static const QChar normParenthToChar[int(Parenthesis::NUMOF)];

      QString           _displayText;           // the constructed display text (read-only)
      int               ord;                    // the line ordinal of this element in the FB stack
      // the parts making a FiguredBassItem up
      Modifier          _prefix;                // the accidental coming before the body
      int               _digit;                 // the main digit (if present)
      Modifier          _suffix;                // the accidental coming after the body
      ContLine          _contLine;              // whether the item has continuation line or not
      Parenthesis       parenth[5];             // each of the parenthesis: before, between and after parts
      qreal             textWidth;              // the text width (in raster units), set during layout()
                                                //    used by draw()
      // part parsing
      int               parseDigit(QString& str);
      int               parseParenthesis(QString& str, int parenthIdx);
      int               parsePrefixSuffix(QString& str, bool bPrefix);

      void              setDisplayText(const QString& s)    { _displayText = s;       }
      // read / write MusicXML support
      QString           Modifier2MusicXML(FiguredBassItem::Modifier prefix) const;

   public:
      FiguredBassItem(Score * s = 0, int line = 0);
      FiguredBassItem(const FiguredBassItem&);
      ~FiguredBassItem();

      FiguredBassItem &operator=(const FiguredBassItem&) = delete;

      FiguredBassItem::Modifier MusicXML2Modifier(const QString prefix) const;

      // standard re-implemented virtual functions
      FiguredBassItem*  clone() const override  { return new FiguredBassItem(*this); }
      ElementType       type() const override   { return ElementType::INVALID; }
      void              draw(QPainter* painter) const override;
      void              layout() override;
      void              read(XmlReader&) override;
      void              write(XmlWriter& xml) const override;

      // read / write MusicXML
      void              writeMusicXML(XmlWriter& xml, bool isOriginalFigure, int crEndTick, int fbEndTick) const;
      bool              startsWithParenthesis() const;

      // specific API
      const FiguredBass*    figuredBass() const       { return (FiguredBass*)(parent()); }
      bool              parse(QString& text);

      // getters / setters
      Modifier          prefix() const                { return _prefix;       }
      void              setPrefix(const Modifier& v)  { _prefix = v;          }
      void              undoSetPrefix(Modifier pref);
      int               digit() const                 { return _digit;        }
      void              setDigit(int val)             { _digit = val;         }
      void              undoSetDigit(int digit);
      Modifier          suffix() const                { return _suffix;       }
      void              setSuffix(const Modifier& v)  { _suffix = v;          }
      void              undoSetSuffix(Modifier suff);
      ContLine          contLine() const              { return _contLine;     }
      void              setContLine(const ContLine& v){ _contLine = v;        }
      void              undoSetContLine(ContLine val);
      Parenthesis       parenth1()                    { return parenth[0];    }
      Parenthesis       parenth2()                    { return parenth[1];    }
      Parenthesis       parenth3()                    { return parenth[2];    }
      Parenthesis       parenth4()                    { return parenth[3];    }
      Parenthesis       parenth5()                    { return parenth[4];    }

      void              setParenth1(Parenthesis v)    { parenth[0] = v;    }
      void              setParenth2(Parenthesis v)    { parenth[1] = v;    }
      void              setParenth3(Parenthesis v)    { parenth[2] = v;    }
      void              setParenth4(Parenthesis v)    { parenth[3] = v;    }
      void              setParenth5(Parenthesis v)    { parenth[4] = v;    }

      void              undoSetParenth1(Parenthesis par);
      void              undoSetParenth2(Parenthesis par);
      void              undoSetParenth3(Parenthesis par);
      void              undoSetParenth4(Parenthesis par);
      void              undoSetParenth5(Parenthesis par);
      QString           normalizedText() const;
      QString           displayText() const           { return _displayText;  }

      QVariant  getProperty(Pid propertyId) const override;
      bool      setProperty(Pid propertyId, const QVariant&) override;
      QVariant  propertyDefault(Pid) const override;
      };

//---------------------------------------------------------
//   FiguredBassFont
//---------------------------------------------------------

struct FiguredBassFont {
      QString           family;
      QString           displayName;
      qreal             defPitch;
      qreal             defLineHeight;
      QChar             displayAccidental[int(FiguredBassItem::Modifier::NUMOF)];
      QChar             displayParenthesis[int(FiguredBassItem::Parenthesis::NUMOF)];
      QChar             displayDigit[int(FiguredBassItem::Style::NUMOF)][10][int(FiguredBassItem::Combination::NUMOF)];

      bool read(XmlReader&);
      };

//---------------------------------------------------------
//   @@ FiguredBass
///    A complete figured bass indication
//
//   @P onNote  bool  whether it is placed on a note beginning or between notes (read only)
//   @P ticks   int   duration in ticks
//---------------------------------------------------------

class FiguredBass final : public TextBase {
      std::vector<FiguredBassItem*> items;      // the individual lines of the F.B.
      QVector<qreal>    _lineLengths;           // lengths of duration indicator lines (in raster units)
      bool              _onNote;                // true if this element is on a staff note | false if it is betweee notes
      Fraction          _ticks;                 // the duration (used for cont. lines and for multiple F.B.
                                                // under the same note)
      qreal             _printedLineLength;     // the length of lines actually printed (i.e. continuation lines)
      void              layoutLines();
      bool              hasParentheses() const; // read / write MusicXML support

      Sid getPropertyStyle(Pid) const override;

   public:
      FiguredBass(Score* s = 0);
      FiguredBass(const FiguredBass&);
      ~FiguredBass();

      // a convenience static function to create/retrieve a new FiguredBass into/from its intended parent
      static FiguredBass* addFiguredBassToSegment(Segment* seg, int track, const Fraction& extTicks, bool *pNew);

      // static functions for font config files
      static bool       readConfigFile(const QString& fileName);
      static QList<QString>  fontNames();
      static bool       fontData(int nIdx, QString *pFamily, QString *pDisplayName,
                              qreal * pSize, qreal * pLineHeight);

      // standard re-implemented virtual functions
      FiguredBass*  clone() const override     { return new FiguredBass(*this); }
      ElementType   type() const override      { return ElementType::FIGURED_BASS; }
      void      draw(QPainter* painter) const override;
      void      endEdit(EditData&) override;
      void      layout() override;
      void      read(XmlReader&) override;
      void      setSelected(bool f) override;
      void      setVisible(bool f) override;
      void      startEdit(EditData&) override;
      void      write(XmlWriter& xml) const override;

      // read / write MusicXML
      void              writeMusicXML(XmlWriter& xml, bool isOriginalFigure, int crEndTick, int fbEndTick, bool writeDuration, int divisions) const;

//DEBUG
//Q_INVOKABLE Ms::FiguredBassItem* addItem();

      // getters / setters / properties
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
      qreal             lineLength(int idx) const     {   if(_lineLengths.size() > idx)
                                                            return _lineLengths.at(idx);
                                                          return 0;   }
      qreal             printedLineLength() const     { return _printedLineLength; }
      bool              onNote() const          { return _onNote; }
      size_t            numOfItems() const      { return items.size(); }
      void              setOnNote(bool val)     { _onNote = val;  }
      Segment *         segment() const         { return (Segment*)(parent()); }
      Fraction          ticks() const           { return _ticks;  }
      void              setTicks(const Fraction& v) { _ticks = v;   }

      qreal             additionalContLineX(qreal pagePosY) const;// returns the X coord (in page coord) of cont. line at pagePosY, if any
      FiguredBass *     nextFiguredBass() const;                  // returns next *adjacent* f.b. item, if any

      QVariant  getProperty(Pid propertyId) const override;
      bool      setProperty(Pid propertyId, const QVariant&) override;
      QVariant  propertyDefault(Pid) const override;

      void appendItem(FiguredBassItem* item) {  items.push_back(item); }
      };


}     // namespace Ms

Q_DECLARE_METATYPE(Ms::FiguredBassItem::Modifier);
Q_DECLARE_METATYPE(Ms::FiguredBassItem::Parenthesis);
Q_DECLARE_METATYPE(Ms::FiguredBassItem::ContLine);

#endif

