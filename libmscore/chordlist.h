//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __CHORDLIST_H__
#define __CHORDLIST_H__

namespace Ms {

class Xml;
class XmlReader;
class ChordList;

//---------------------------------------------------------
//   class HDegree
//---------------------------------------------------------

enum HDegreeType {
      UNDEF, ADD, ALTER, SUBTRACT
      };

class HDegree {
      int _value;
      int _alter;       // -1, 0, 1  (b - - #)
      int _type;

   public:
      HDegree() { _value = 0; _alter = 0; _type = UNDEF; }
      HDegree(int v, int a, int t) { _value = v; _alter = a; _type = t; }
      int value() const { return _value; }
      int alter() const { return _alter; }
      int type() const  { return _type; }
      QString text() const;
      };

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

class HChord {
      QString str;

   protected:
      int keys;

   public:
      HChord()      { keys = 0; }
      HChord(int k) { keys = k; }
      HChord(int a, int b, int c=-1, int d=-1, int e=-1, int f=-1, int g=-1,
            int h=-1, int i=-1, int k=-1, int l=-1);
      HChord(const QString&);

      void rotate(int semiTones);

      bool contains(int key) const {       // key in chord?
            return (1 << (key % 12)) & keys;
            }
      HChord& operator+= (int key) {
            keys |= (1 << (key % 12));
            return *this;
            }
      HChord& operator-= (int key) {
            keys &= ~(1 << (key % 12));
            return *this;
            }
      bool operator==(const HChord& o) const { return (keys == o.keys); }
      bool operator!=(const HChord& o) const { return (keys != o.keys); }

      int getKeys() const { return keys; }
      void print() const;

      QString name(int tpc) const;
      QString voicing() const;
      void add(const QList<HDegree>& degreeList);
      };

//---------------------------------------------------------
//   RenderAction
//---------------------------------------------------------

struct RenderAction {
      enum RenderActionType {
            RENDER_SET, RENDER_MOVE, RENDER_PUSH, RENDER_POP,
            RENDER_NOTE, RENDER_ACCIDENTAL
            };

      RenderActionType type;
      qreal movex, movey;          // RENDER_MOVE
      QString text;                 // RENDER_SET

      RenderAction() {}
      RenderAction(RenderActionType t) : type(t) {}
      };

//---------------------------------------------------------
//   ChordToken
//---------------------------------------------------------

enum ChordTokenClass {
      ALL, QUALITY, EXTENSION, MODIFIER, ALTERATION, ADJUST, MODE, SUSPENSION, ADDITION, SUBTRACTION
      };

class ChordToken {
   public:
      ChordTokenClass tokenClass;
      QStringList names;
      QList<RenderAction> renderList;
      void read(XmlReader&);
      void write(Xml&) const;
      };

//---------------------------------------------------------
//   ParsedChord
//---------------------------------------------------------

class ParsedChord {
   public:
      bool parse(const QString&, const ChordList*, bool syntaxOnly = false);
      QString fromXml(const QString&, const QString&, const QString&, const QString&, const QList<HDegree>&, const ChordList*);
      const QList<RenderAction>& renderList(const ChordList*);
      bool parseable() const                    { return _parseable; }
      bool understandable() const               { return _understandable; }
      const QString& xmlKind() const            { return _xmlKind; }
      const QString& xmlText() const            { return _xmlText; }
      const QString& xmlSymbols() const         { return _xmlSymbols; }
      const QString& xmlParens() const          { return _xmlParens; }
      const QStringList& xmlDegrees() const     { return _xmlDegrees; }
      int keys() const                          { return chord.getKeys(); }
      operator QString() const                  { return _handle; }
      bool operator==(const ParsedChord& c) const     { return (this->_handle == c._handle); }
      bool operator!=(const ParsedChord& c) const     { return !(*this == c); }
      ParsedChord();
   private:
      QString _name;
      QString _handle;
      QString _quality;
      QString _extension;
      QString _modifiers;
      QStringList _modifierList;
      QList<ChordToken> _tokenList;
      QList<RenderAction> _renderList;
      QString _xmlKind;
      QString _xmlText;
      QString _xmlSymbols;
      QString _xmlParens;
      QStringList _xmlDegrees;
      QStringList major, minor, diminished, augmented, lower, raise, mod1, mod2, symbols;
      HChord chord;
      bool _parseable;
      bool _understandable;
      void configure(const ChordList*);
      void correctXmlText(const QString& s = "");
      void addToken(QString, ChordTokenClass);
      };

//---------------------------------------------------------
//   ChordDescription
//---------------------------------------------------------

struct ChordDescription {
      int id;                 // Chord id number (Band In A Box Chord Number)
      QStringList names;      // list of alternative chord names
                              // that will by recognized from keyboard entry (without root/base)
      QList<ParsedChord> parsedChords;
                              // parsed forms of primary name (optionally also include parsed forms of other names)
      QString xmlKind;        // MusicXml: kind
      QString xmlText;        // MusicXml: kind text=
      QString xmlSymbols;     // MusicXml: kind use-symbols=
      QString xmlParens;      // MusicXml: kind parentheses-degrees=
      QStringList xmlDegrees; // MusicXml: list of degrees (if any)
      HChord chord;           // C based chord
      QList<RenderAction> renderList;
      bool generated;
      bool renderListGenerated;
      bool exportOk;

   public:
      ChordDescription(int, ChordList*);
      ChordDescription(const QString&, ChordList*);
      void complete(ParsedChord* pc, const ChordList*);
      void read(XmlReader&);
      void write(Xml&) const;
      };

//---------------------------------------------------------
//   ChordSymbol
//---------------------------------------------------------

struct ChordSymbol {
      int fontIdx;
      QString name;
      QChar code;

      ChordSymbol() { fontIdx = -1; }
      bool isValid() const { return fontIdx != -1; }
      };

//---------------------------------------------------------
//   ChordFont
//---------------------------------------------------------

struct ChordFont {
      QString family;
      qreal mag;
      };

//---------------------------------------------------------
//   ChordList
//---------------------------------------------------------

class ChordList : public QMap<int, ChordDescription*> {
      QHash<QString, ChordSymbol> symbols;

   public:
      QList<ChordFont> fonts;
      QList<RenderAction> renderListRoot;
      QList<RenderAction> renderListBase;
      QList<ChordToken> chordTokenList;
      static int privateID;

      ChordList();

      virtual ~ChordList();
      void write(Xml& xml) const;
      void read(XmlReader&);
      bool read(const QString&);
      bool write(const QString&) const;
      ChordSymbol symbol(const QString& s) const { return symbols.value(s); }
      };


}     // namespace Ms
#endif

