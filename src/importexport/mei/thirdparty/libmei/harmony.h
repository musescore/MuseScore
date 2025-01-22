/////////////////////////////////////////////////////////////////////////////
// Authors:     Laurent Pugin and Rodolfo Zitellini
// Created:     2014
// Copyright (c) Authors and others. All rights reserved.
//
// Code generated using a modified version of libmei
// by Andrew Hankinson, Alastair Porter, and Others
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// NOTE: this file was generated with the Verovio libmei version and
// should not be edited because changes will be lost.
/////////////////////////////////////////////////////////////////////////////

#ifndef __LIBMEI_HARMONY_H__
#define __LIBMEI_HARMONY_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include "element.h"
#include "atts_gestural.h"
#include "atts_harmony.h"
#include "atts_shared.h"
#include "atts_stringtab.h"
#include "atts_visual.h"


namespace libmei {

/** Chord tablature definition. **/
class ChordDef : public Element, public AttLabelled, public AttTyped, public AttStringtabPosition, public AttStringtabTuning {
    public:
        ChordDef();
        virtual ~ChordDef();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** An individual pitch in a chord defined by a chordDef element. **/
class ChordMember : public Element, public AttLabelled, public AttTyped, public AttAccidentalGes, public AttPitch, public AttOctave, public AttStringtab {
    public:
        ChordMember();
        virtual ~ChordMember();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Chord/tablature look-up table. **/
class ChordTable : public Element, public AttLabelled, public AttTyped {
    public:
        ChordTable();
        virtual ~ChordTable();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Single element of a figured bass indication. **/
class F : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttDurationAdditive, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttColor, public AttExtender, public AttLineRend, public AttLineRendBase, public AttPlacementRelStaff, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        F();
        virtual ~F();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * Symbols added to a bass line that indicate harmony.
 * Used to improvise a chordal accompaniment. Sometimes called Generalbass,
 * thoroughbass, or basso continuo.
 **/
class Fb : public Element, public AttLabelled, public AttTyped {
    public:
        Fb();
        virtual ~Fb();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * An indication of harmony, e.g., chord names, tablature grids, harmonic analysis,
 * figured bass.
 **/
class Harm : public Element, public AttLabelled, public AttTyped, public AttHarmLog, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttDurationAdditive, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttHarmVis, public AttColor, public AttExtender, public AttLineRend, public AttLineRendBase, public AttPlacementRelStaff, public AttVisualOffsetHo, public AttVisualOffsetVo, public AttVisualOffset2Ho {
    public:
        Harm();
        virtual ~Harm();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

} // namespace libmei

#endif // __LIBMEI_HARMONY_H__
