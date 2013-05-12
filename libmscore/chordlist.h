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

      QString name(int tpc);
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
//   ParsedChord
//---------------------------------------------------------

class ParsedChord {
   public:
      const QList<RenderAction>& renderList();
      bool parse(QString);
      bool renderable() { return _parseable; }
      bool transposable() { return _parseable; }
      bool understandable () { return _understandable; }
      bool operator==(const ParsedChord& c) { return (this->handle == c.handle); }
      bool operator!=(const ParsedChord& c) { return !(*this == c); }
      operator QString() { return handle; }
      ParsedChord() { _parseable = false; _understandable = false; }
   private:
      QString handle;
      QString quality;
      QString extension;
      QStringList modifierList;
      QStringList _tokenList;
      QList<RenderAction> _renderList;
      bool _parseable;
      bool _understandable;
      };

//---------------------------------------------------------
//   ChordDescription
//---------------------------------------------------------

struct ChordDescription {
      int id;                 // Chord id number (Band In A Box Chord Number)
                              // 0 = no id specified (valid to allow chords to match without recording id in score file)
      QStringList names;      // list of alternative chord names
                              // that will by recognized from keyboard entry (without root/base)
      QList<ParsedChord> parsedChords;
                              // parsed forms of names
      QString xmlKind;        // MusicXml description: kind
      QStringList xmlDegrees; // MusicXml description: list of degrees (if any)
      HChord chord;           // C based chord
      QList<RenderAction> renderList;

   public:
      void read(XmlReader&);
      void write(Xml&);
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

      ChordList() {}

      virtual ~ChordList();
      void write(Xml& xml);
      void read(XmlReader&);
      bool read(const QString&);
      bool write(const QString&);
      ChordSymbol symbol(const QString& s) const { return symbols.value(s); }
      };


}     // namespace Ms
#endif

