//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __FRET_H__
#define __FRET_H__

#include "element.h"
#include "harmony.h"

namespace Ms {
class StringData;
class Chord;

enum class Orientation : signed char {
    VERTICAL,
    HORIZONTAL
};

// Keep this in order - not used directly for comparisons, but the dots will appear in
// this order in fret multidot mode. See fretproperties.cpp.
enum class FretDotType : signed char {
    NORMAL = 0,
    CROSS,
    SQUARE,
    TRIANGLE = 3
};

enum class FretMarkerType : signed char {
    NONE,
    CIRCLE,
    CROSS
};

class FretItem
{
public:
    struct Barre {
        int startString;
        int endString;

        Barre() { startString = endString = -1; }
        Barre(int s, int e)
            : startString(s), endString(e) {}
        bool exists() const { return startString > -1; }
    };

    struct Dot {
        int fret                { 0 };
        FretDotType dtype       { FretDotType::NORMAL };
        int fingering           { 0 };     // NOTE:JT - possible future feature?

        Dot() {}
        Dot(int f, FretDotType t = FretDotType::NORMAL)
            : fret(f), dtype(t) {}
        bool exists() const { return fret > 0; }
    };

    struct Marker {
        FretMarkerType mtype;

        Marker() { mtype = FretMarkerType::NONE; }
        Marker(FretMarkerType t)
            : mtype(t) {}
        bool exists() const { return mtype != FretMarkerType::NONE; }
    };

    struct MarkerTypeNameItem {
        FretMarkerType mtype;
        const char* name;
    };

    struct DotTypeNameItem {
        FretDotType dtype;
        const char* name;
    };

    static const std::vector<FretItem::MarkerTypeNameItem> markerTypeNameMap;
    static const std::vector<FretItem::DotTypeNameItem> dotTypeNameMap;

    static QChar markerToChar(FretMarkerType t);
    static QString markerTypeToName(FretMarkerType t);
    static FretMarkerType nameToMarkerType(QString n);
    static QString dotTypeToName(FretDotType t);
    static FretDotType nameToDotType(QString n);
};

// The three main storage containers used by fret diagrams
typedef std::map<int, FretItem::Barre> BarreMap;
typedef std::map<int, FretItem::Marker> MarkerMap;
typedef std::map<int, std::vector<FretItem::Dot> > DotMap;

class FretUndoData
{
    FretDiagram* _diagram         { nullptr };
    BarreMap _barres;
    MarkerMap _markers;
    DotMap _dots;

public:
    FretUndoData() {}
    FretUndoData(FretDiagram* fd);

    void updateDiagram();
};

//---------------------------------------------------------
//   @@ FretDiagram
///    Fretboard diagram
//
//   @P userMag    qreal
//   @P strings    int  number of strings
//   @P frets      int  number of frets
//   @P fretOffset int
//
//   Note that, while strings are zero-indexed, frets are one-indexed
//---------------------------------------------------------

class FretDiagram final : public Element
{
    int _strings       { 6 };
    int _frets         { 4 };
    int _fretOffset    { 0 };
    int _maxFrets      { 24 };
    bool _showNut      { true };
    Orientation _orientation      { Orientation::VERTICAL };

    // Barres are stored in the format: K: fret, V: barre struct
    BarreMap _barres;

    // Dots stored as K: string, V: dot struct
    DotMap _dots;

    // Markers stored as K: string, V: marker struct
    MarkerMap _markers;

    Harmony* _harmony  { nullptr };

    qreal stringLw;
    qreal nutLw;
    qreal stringDist;
    qreal fretDist;
    QFont font;
    qreal _userMag     { 1.0 };                 // allowed 0.1 - 10.0
    qreal markerSize;
    int _numPos;

    void removeDot(int s, int f = 0);
    void removeBarre(int f);
    void removeBarres(int string, int fret = 0);
    void removeMarker(int s);
    void removeDotsMarkers(int ss, int es, int fret);

public:
    FretDiagram(Score* s = nullptr);
    FretDiagram(const FretDiagram&);
    ~FretDiagram();

    // Score Tree functions
    ScoreElement* treeParent() const override;
    ScoreElement* treeChild(int idx) const override;
    int treeChildCount() const override;

    void draw(mu::draw::Painter*) const override;
    Element* linkedClone() override;
    FretDiagram* clone() const override { return new FretDiagram(*this); }

    Segment* segment() const { return toSegment(parent()); }

    static std::shared_ptr<FretDiagram> createFromString(Score* score, const QString& s);

    ElementType type() const override { return ElementType::FRET_DIAGRAM; }
    void layout() override;
    void write(XmlWriter& xml) const override;
    void writeNew(XmlWriter& xml) const;
    void writeOld(XmlWriter& xml) const;
    void read(XmlReader&) override;
    void readNew(XmlReader&);
    QVector<QLineF> dragAnchorLines() const override;
    QPointF pagePos() const override;

    // read / write MusicXML
    void readMusicXML(XmlReader& de);
    void writeMusicXML(XmlWriter& xml) const;

    int  strings() const { return _strings; }
    int  frets()   const { return _frets; }
    void setStrings(int n);
    void setFrets(int n) { _frets = n; }

    void setDot(int string, int fret, bool add = false, FretDotType dtype = FretDotType::NORMAL);
    void setBarre(int startString, int endString, int fret);
    void setBarre(int string, int fret, bool add = false);
    void setMarker(int string, FretMarkerType marker);
    /*void setFingering(int string, int finger);*/
    void clear();
    void undoSetFretDot(int _string, int _fret, bool _add = false, FretDotType _dtype = FretDotType::NORMAL);
    void undoSetFretMarker(int _string, FretMarkerType _mtype);
    void undoSetFretBarre(int _string, int _fret, bool _add = false);
    void undoFretClear();
    int  fretOffset() const { return _fretOffset; }
    void setFretOffset(int val) { _fretOffset = val; }
    int  maxFrets() const { return _maxFrets; }
    void setMaxFrets(int val) { _maxFrets = val; }
    bool showNut() const { return _showNut; }
    void setShowNut(bool val) { _showNut = val; }

    QString harmonyText() const { return _harmony ? _harmony->plainText() : QString(); }
    qreal centerX() const;
    void setHarmony(QString harmonyText);

    std::vector<FretItem::Dot> dot(int s, int f = 0) const;
    FretItem::Marker marker(int s) const;
    FretItem::Barre barre(int fret) const;
#if 0 // NOTE:JT possible future feature
    int fingering(int s) const { return _fingering ? _fingering[s] : 0; }
#endif

    BarreMap barres() const { return _barres; }
    DotMap dots() const { return _dots; }
    MarkerMap markers() const { return _markers; }

    Harmony* harmony() const { return _harmony; }

    void init(Ms::StringData*, Chord*);
    void add(Element*) override;
    void remove(Element*) override;

    bool acceptDrop(EditData&) const override;
    Element* drop(EditData&) override;

    void endEditDrag(EditData& editData) override;
    void scanElements(void* data, void (* func)(void*, Element*), bool all=true) override;

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid) const override;

    qreal userMag() const { return _userMag; }
    void setUserMag(qreal m) { _userMag = m; }

    virtual QString accessibleInfo() const override;
    virtual QString screenReaderInfo() const override;

    friend class FretUndoData;
};
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::FretDiagram*)

#endif
