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

#ifndef __CHORDLIST_H__
#define __CHORDLIST_H__

#include <map>

#include "global/allocator.h"
#include "types/string.h"
#include "containers.h"
#include "io/iodevice.h"

#include "modularity/ioc.h"
#include "iengravingconfiguration.h"

namespace mu::engraving::compat {
class ReadChordListHook;
}

namespace mu::engraving {
class ChordList;
class MStyle;
class XmlWriter;
class XmlReader;

//---------------------------------------------------------
//   class HDegree
//---------------------------------------------------------

enum class HDegreeType : char {
    UNDEF, ADD, ALTER, SUBTRACT
};

class HDegree
{
    int _value;
    int _alter;         // -1, 0, 1  (b - - #)
    HDegreeType _type;

public:
    HDegree() { _value = 0; _alter = 0; _type = HDegreeType::UNDEF; }
    HDegree(int v, int a, HDegreeType t) { _value = v; _alter = a; _type = t; }
    int value() const { return _value; }
    int alter() const { return _alter; }
    HDegreeType type() const { return _type; }
    String text() const;
};

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

class HChord
{
    String str;

protected:
    int keys;

public:
    HChord() { keys = 0; }
    HChord(int k) { keys = k; }
    HChord(int a, int b, int c=-1, int d=-1, int e=-1, int f=-1, int g=-1, int h=-1, int i=-1, int k=-1, int l=-1);
    HChord(const String&);

    void rotate(int semiTones);

    bool contains(int key) const           // key in chord?
    {
        return (1 << (key % 12)) & keys;
    }

    HChord& operator+=(int key)
    {
        keys |= (1 << (key % 12));
        return *this;
    }

    HChord& operator-=(int key)
    {
        keys &= ~(1 << (key % 12));
        return *this;
    }

    bool operator==(const HChord& o) const { return keys == o.keys; }
    bool operator!=(const HChord& o) const { return keys != o.keys; }

    int getKeys() const { return keys; }
    void print() const;

    String name(int tpc) const;
    String voicing() const;
    void add(const std::vector<HDegree>& degreeList);
};

//---------------------------------------------------------
//   RenderAction
//---------------------------------------------------------

struct RenderAction {
    enum class RenderActionType : char {
        SET, MOVE, PUSH, POP,
        NOTE, ACCIDENTAL
    };

    RenderActionType type = RenderActionType::SET;
    double movex = 0.0, movey = 0.0; // MOVE
    String text;                    // SET

    RenderAction() {}
    RenderAction(RenderActionType t)
        : type(t) {}
    void print() const;
};

//---------------------------------------------------------
//   ChordToken
//---------------------------------------------------------

enum class ChordTokenClass : char {
    ALL, QUALITY, EXTENSION, MODIFIER, ALTERATION, ADJUST, MODE, SUSPENSION, ADDITION, SUBTRACTION
};

class ChordToken
{
public:
    ChordTokenClass tokenClass;
    StringList names;
    std::list<RenderAction> renderList;
    void read(XmlReader&);
    void write(XmlWriter&) const;
};

//---------------------------------------------------------
//   ParsedChord
//---------------------------------------------------------

class ParsedChord
{
public:
    bool parse(const String&, const ChordList*, bool syntaxOnly = false, bool preferMinor = false);
    String fromXml(const String&, const String&, const String&, const String&, const std::list<HDegree>&, const ChordList*);
    const std::list<RenderAction>& renderList(const ChordList*);
    bool parseable() const { return _parseable; }
    bool understandable() const { return _understandable; }
    const String& name() const { return _name; }
    const String& quality() const { return _quality; }
    const String& extension() const { return _extension; }
    const String& modifiers() const { return _modifiers; }
    const StringList& modifierList() const { return _modifierList; }
    const String& xmlKind() const { return _xmlKind; }
    const String& xmlText() const { return _xmlText; }
    const String& xmlSymbols() const { return _xmlSymbols; }
    const String& xmlParens() const { return _xmlParens; }
    const StringList& xmlDegrees() const { return _xmlDegrees; }
    int keys() const { return chord.getKeys(); }
    const String& handle() const { return _handle; }
    operator String() const {
        return _handle;
    }
    bool operator==(const ParsedChord& c) const { return this->_handle == c._handle; }
    bool operator!=(const ParsedChord& c) const { return !(*this == c); }
    ParsedChord();
private:
    String _name;
    String _handle;
    String _quality;
    String _extension;
    String _modifiers;
    StringList _modifierList;
    std::list<ChordToken> _tokenList;
    std::list<RenderAction> _renderList;
    String _xmlKind;
    String _xmlText;
    String _xmlSymbols;
    String _xmlParens;
    StringList _xmlDegrees;
    StringList major, minor, diminished, augmented, lower, raise, mod1, mod2, symbols;
    HChord chord;
    bool _parseable;
    bool _understandable;
    void configure(const ChordList*);
    void correctXmlText(const String& s = String());
    void addToken(String, ChordTokenClass);
};

//---------------------------------------------------------
//   ChordDescription
//---------------------------------------------------------

struct ChordDescription {
    int id = 0;               // Chord id number (Band In A Box Chord Number)
    StringList names;        // list of alternative chord names
                             // that will by recognized from keyboard entry (without root/base)
    std::list<ParsedChord> parsedChords;
    // parsed forms of primary name (optionally also include parsed forms of other names)
    String xmlKind;          // MusicXml: kind
    String xmlText;          // MusicXml: kind text=
    String xmlSymbols;       // MusicXml: kind use-symbols=
    String xmlParens;        // MusicXml: kind parentheses-degrees=
    StringList xmlDegrees;   // MusicXml: list of degrees (if any)
    HChord chord;             // C based chord
    std::list<RenderAction> renderList;
    bool generated = false;
    bool renderListGenerated = false;
    bool exportOk = false;
    String _quality;

public:
    ChordDescription() {}
    ChordDescription(int);
    ChordDescription(const String&);
    String quality() const { return _quality; }
    void complete(ParsedChord* pc, const ChordList*);
    void read(XmlReader&);
    void write(XmlWriter&) const;
};

//---------------------------------------------------------
//   ChordSymbol
//---------------------------------------------------------

struct ChordSymbol {
    int fontIdx;
    String name;
    String value;
    Char code;

    ChordSymbol() { fontIdx = -1; }
    bool isValid() const { return fontIdx != -1; }
};

//---------------------------------------------------------
//   ChordFont
//---------------------------------------------------------

struct ChordFont {
    String family;
    String fontClass;
    double mag;
};

//---------------------------------------------------------
//   ChordList
//---------------------------------------------------------

class ChordList : public std::map<int, ChordDescription>
{
    OBJECT_ALLOCATOR(engraving, ChordList)

    INJECT(engraving, IEngravingConfiguration, configuration)

    std::map<String, ChordSymbol> symbols;
    bool _autoAdjust = false;
    double _nmag = 1.0, _nadjust = 0.0;
    double _emag = 1.0, _eadjust = 0.0;
    double _mmag = 1.0, _madjust = 0.0;

    bool _customChordList = false; // if true, chordlist will be saved as part of score

public:
    std::list<ChordFont> fonts;
    std::list<RenderAction> renderListRoot;
    std::list<RenderAction> renderListFunction;
    std::list<RenderAction> renderListBase;
    std::list<ChordToken> chordTokenList;
    static int privateID;

    bool autoAdjust() const { return _autoAdjust; }
    double nominalMag() const { return _nmag; }
    double nominalAdjust() const { return _nadjust; }
    void configureAutoAdjust(double emag = 1.0, double eadjust = 0.0, double mmag = 1.0, double madjust = 0.0);
    double position(const StringList& names, ChordTokenClass ctc) const;

    bool read(const String&);
    bool read(io::IODevice* device);
    bool write(const String&) const;
    bool write(io::IODevice* device) const;
    bool loaded() const;
    void unload();

    const ChordDescription* description(int id) const;
    ChordSymbol symbol(const String& s) const { return mu::value(symbols, s); }

    void setCustomChordList(bool t) { _customChordList = t; }
    bool customChordList() const { return _customChordList; }

    void checkChordList(const MStyle& style);

private:

    friend class compat::ReadChordListHook;

    void read(XmlReader&);
    void write(XmlWriter& xml) const;
};
} // namespace mu::engraving
#endif
