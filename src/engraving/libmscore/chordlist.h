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

#include <QMap>
#include "style/style.h"

namespace mu::engraving::compat {
class ReadChordListHook;
}

namespace Ms {
class XmlWriter;
class XmlReader;
class ChordList;

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
    QString text() const;
};

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

class HChord
{
    QString str;

protected:
    int keys;

public:
    HChord() { keys = 0; }
    HChord(int k) { keys = k; }
    HChord(int a, int b, int c=-1, int d=-1, int e=-1, int f=-1, int g=-1, int h=-1, int i=-1, int k=-1, int l=-1);
    HChord(const QString&);

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

    QString name(int tpc) const;
    QString voicing() const;
    void add(const QList<HDegree>& degreeList);
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
    qreal movex = 0.0, movey = 0.0; // MOVE
    QString text;                   // SET

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
    QStringList names;
    QList<RenderAction> renderList;
    void read(XmlReader&);
    void write(XmlWriter&) const;
};

//---------------------------------------------------------
//   ParsedChord
//---------------------------------------------------------

class ParsedChord
{
public:
    bool parse(const QString&, const ChordList*, bool syntaxOnly = false, bool preferMinor = false);
    QString fromXml(const QString&, const QString&, const QString&, const QString&, const QList<HDegree>&, const ChordList*);
    const QList<RenderAction>& renderList(const ChordList*);
    bool parseable() const { return _parseable; }
    bool understandable() const { return _understandable; }
    const QString& name() const { return _name; }
    const QString& quality() const { return _quality; }
    const QString& extension() const { return _extension; }
    const QString& modifiers() const { return _modifiers; }
    const QStringList& modifierList() const { return _modifierList; }
    const QString& xmlKind() const { return _xmlKind; }
    const QString& xmlText() const { return _xmlText; }
    const QString& xmlSymbols() const { return _xmlSymbols; }
    const QString& xmlParens() const { return _xmlParens; }
    const QStringList& xmlDegrees() const { return _xmlDegrees; }
    int keys() const { return chord.getKeys(); }
    const QString& handle() const { return _handle; }
    operator QString() const {
        return _handle;
    }
    bool operator==(const ParsedChord& c) const { return this->_handle == c._handle; }
    bool operator!=(const ParsedChord& c) const { return !(*this == c); }
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
    int id = 0;               // Chord id number (Band In A Box Chord Number)
    QStringList names;        // list of alternative chord names
                              // that will by recognized from keyboard entry (without root/base)
    QList<ParsedChord> parsedChords;
    // parsed forms of primary name (optionally also include parsed forms of other names)
    QString xmlKind;          // MusicXml: kind
    QString xmlText;          // MusicXml: kind text=
    QString xmlSymbols;       // MusicXml: kind use-symbols=
    QString xmlParens;        // MusicXml: kind parentheses-degrees=
    QStringList xmlDegrees;   // MusicXml: list of degrees (if any)
    HChord chord;             // C based chord
    QList<RenderAction> renderList;
    bool generated = false;
    bool renderListGenerated = false;
    bool exportOk = false;
    QString _quality;

public:
    ChordDescription() {}
    ChordDescription(int);
    ChordDescription(const QString&);
    QString quality() const { return _quality; }
    void complete(ParsedChord* pc, const ChordList*);
    void read(XmlReader&);
    void write(XmlWriter&) const;
};

//---------------------------------------------------------
//   ChordSymbol
//---------------------------------------------------------

struct ChordSymbol {
    int fontIdx;
    QString name;
    QString value;
    QChar code;

    ChordSymbol() { fontIdx = -1; }
    bool isValid() const { return fontIdx != -1; }
};

//---------------------------------------------------------
//   ChordFont
//---------------------------------------------------------

struct ChordFont {
    QString family;
    QString fontClass;
    qreal mag;
};

//---------------------------------------------------------
//   ChordList
//---------------------------------------------------------

class ChordList : public QMap<int, ChordDescription>
{
    QMap<QString, ChordSymbol> symbols;
    bool _autoAdjust = false;
    qreal _nmag = 1.0, _nadjust = 0.0;
    qreal _emag = 1.0, _eadjust = 0.0;
    qreal _mmag = 1.0, _madjust = 0.0;

    bool _customChordList = false; // if true, chordlist will be saved as part of score

public:
    QList<ChordFont> fonts;
    QList<RenderAction> renderListRoot;
    QList<RenderAction> renderListFunction;
    QList<RenderAction> renderListBase;
    QList<ChordToken> chordTokenList;
    static int privateID;

    bool autoAdjust() const { return _autoAdjust; }
    qreal nominalMag() const { return _nmag; }
    qreal nominalAdjust() const { return _nadjust; }
    void configureAutoAdjust(qreal emag = 1.0, qreal eadjust = 0.0, qreal mmag = 1.0, qreal madjust = 0.0);
    qreal position(const QStringList& names, ChordTokenClass ctc) const;

    bool read(const QString&);
    bool read(QIODevice* device);
    bool write(const QString&) const;
    bool write(QIODevice* device) const;
    bool loaded() const;
    void unload();

    const ChordDescription* description(int id) const;
    ChordSymbol symbol(const QString& s) const { return symbols.value(s); }

    void setCustomChordList(bool t) { _customChordList = t; }
    bool customChordList() const { return _customChordList; }

    void checkChordList(const MStyle& style);

private:

    friend class mu::engraving::compat::ReadChordListHook;

    void read(XmlReader&);
    void write(XmlWriter& xml) const;
};
}     // namespace Ms
#endif
