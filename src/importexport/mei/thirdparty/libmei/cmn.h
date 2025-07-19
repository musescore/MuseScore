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

#ifndef __LIBMEI_CMN_H__
#define __LIBMEI_CMN_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include "element.h"
#include "atts_cmn.h"
#include "atts_externalsymbols.h"
#include "atts_midi.h"
#include "atts_shared.h"
#include "atts_visual.h"


namespace libmei {

/**
 * Indicates that the notes of a chord are to be performed successively rather than
 * simultaneously, usually from lowest to highest.
 * Sometimes called a "roll".
 **/
class Arpeg : public Element, public AttLabelled, public AttTyped, public AttArpegLog, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttStartId, public AttArpegVis, public AttColor, public AttEnclosingChars, public AttExtSymAuth, public AttExtSymNames, public AttLineRendBase, public AttTypography, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        Arpeg();
        virtual ~Arpeg();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** A rapid alternation on a single pitch or chord. **/
class BTrem : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttStaffIdent, public AttAugmentDots, public AttDurationLog, public AttNumbered, public AttTremForm, public AttNumberPlacement, public AttTremMeasured {
    public:
        BTrem();
        virtual ~BTrem();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * A container for a series of explicitly beamed events that begins and ends
 * entirely within a measure.
 **/
class Beam : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttStaffIdent, public AttColor, public AttCue {
    public:
        Beam();
        virtual ~Beam();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** An indication that material on a preceding beat should be repeated. **/
class BeatRpt : public Element, public AttLabelled, public AttTyped, public AttBeatRptLog, public AttLayerIdent, public AttStaffIdent, public AttBeatRptVis, public AttColor, public AttExpandable, public AttExtSymAuth, public AttExtSymNames, public AttPlist {
    public:
        BeatRpt();
        virtual ~BeatRpt();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * An indication of a point at which the performer on an instrument requiring
 * breath (including the voice) may breathe.
 **/
class Breath : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttStaffIdent, public AttStartId, public AttTimestampLog, public AttColor, public AttExtSymAuth, public AttExtSymNames, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        Breath();
        virtual ~Breath();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * A rapid alternation between a pair of notes (or chords or perhaps between a note
 * and a chord) that are (usually) farther apart than a major second.
 **/
class FTrem : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttStaffIdent, public AttAugmentDots, public AttDurationLog, public AttTremForm, public AttTremMeasured {
    public:
        FTrem();
        virtual ~FTrem();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * An indication placed over a note or rest to indicate that it should be held
 * longer than its written value.
 * May also occur over a bar line to indicate the end of a phrase or section.
 * Sometimes called a 'hold' or 'pause'.
 **/
class Fermata : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttStartEndId, public AttStartId, public AttFermataVis, public AttColor, public AttExtSymAuth, public AttExtSymNames, public AttPlacementRelStaff, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        Fermata();
        virtual ~Fermata();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * A continuous or sliding movement from one pitch to another, usually indicated by
 * a straight or wavy line.
 **/
class Gliss : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttDurationAdditive, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttColor, public AttExtSymAuth, public AttExtSymNames, public AttTypography, public AttVisualOffsetHo, public AttVisualOffsetVo, public AttVisualOffset2Ho, public AttVisualOffset2Vo, public AttLineRend, public AttLineRendBase {
    public:
        Gliss();
        virtual ~Gliss();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** A container for a sequence of grace notes. **/
class GraceGrp : public Element, public AttLabelled, public AttTyped, public AttGraceGrpLog, public AttLayerIdent, public AttStaffIdent, public AttGraced, public AttColor {
    public:
        GraceGrp();
        virtual ~GraceGrp();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * Indicates continuous dynamics expressed on the score as wedge-shaped graphics,
 * e.g., < and >.
 **/
class Hairpin : public Element, public AttLabelled, public AttTyped, public AttHairpinLog, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttDurationAdditive, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttHairpinVis, public AttColor, public AttLineRendBase, public AttPlacementRelStaff, public AttVisualOffsetHo, public AttVisualOffsetVo, public AttVisualOffset2Ho, public AttVisualOffset2Vo, public AttMidiValue, public AttMidiValue2 {
    public:
        Hairpin();
        virtual ~Hairpin();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** A half-measure repeat in any meter. **/
class HalfmRpt : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttStaffIdent, public AttDurationAdditive, public AttColor, public AttExpandable, public AttExtSymAuth, public AttExtSymNames, public AttTypography, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        HalfmRpt();
        virtual ~HalfmRpt();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Harp pedal diagram. **/
class HarpPedal : public Element, public AttLabelled, public AttTyped, public AttHarpPedalLog, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttStartEndId, public AttStartId, public AttColor, public AttExtSymAuth, public AttExtSymNames, public AttPlacementRelStaff, public AttTypography, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        HarpPedal();
        virtual ~HarpPedal();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** A "tie-like" indication that a note should ring beyond its written duration. **/
class Lv : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttColor, public AttCurvature, public AttLineRendBase, public AttVisualOffsetHo, public AttVisualOffsetVo, public AttVisualOffset2Ho, public AttVisualOffset2Vo {
    public:
        Lv();
        virtual ~Lv();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * Designation, name, or label for a measure, often but not always consisting of
 * digits.
 * Use this element when the n attribute on measure does not adequately capture the
 * appearance or placement of the measure number/label.
 **/
class MNum : public Element, public AttLabelled, public AttTyped, public AttLang, public AttColor, public AttPlacementRelStaff, public AttTypography, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        MNum();
        virtual ~MNum();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Complete measure rest in any meter. **/
class MRest : public Element, public AttLabelled, public AttTyped, public AttCue, public AttDurationAdditive, public AttLayerIdent, public AttStaffIdent, public AttColor, public AttCutout, public AttExtSymAuth, public AttExtSymNames, public AttStaffLocPitched, public AttTypography, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        MRest();
        virtual ~MRest();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** An indication that the previous measure should be repeated. **/
class MRpt : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttStaffIdent, public AttNumbered, public AttColor, public AttExpandable, public AttExtSymAuth, public AttExtSymNames, public AttNumberPlacement, public AttTypography {
    public:
        MRpt();
        virtual ~MRpt();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * Unit of musical time consisting of a fixed number of note values of a given
 * type, as determined by the prevailing meter, and delimited in musical notation
 * by bar lines.
 **/
class Measure : public Element, public AttLabelled, public AttTyped, public AttMeasureLog, public AttMeterConformanceBar, public AttNNumberLike {
    public:
        Measure();
        virtual ~Measure();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * Multiple full measure rests compressed into a single bar, frequently found in
 * performer parts.
 **/
class MultiRest : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttStaffIdent, public AttNumbered, public AttMultiRestVis, public AttColor, public AttExtSymAuth, public AttExtSymNames, public AttNumberPlacement, public AttStaffLocPitched, public AttTypography, public AttWidth {
    public:
        MultiRest();
        virtual ~MultiRest();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Multiple repeated measures. **/
class MultiRpt : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttStaffIdent, public AttNumbered, public AttColor, public AttExpandable, public AttExtSymAuth, public AttExtSymNames, public AttTypography {
    public:
        MultiRpt();
        virtual ~MultiRpt();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * An indication that a passage should be performed one or more octaves above or
 * below its written pitch.
 **/
class Octave : public Element, public AttLabelled, public AttTyped, public AttOctaveLog, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttDurationAdditive, public AttOctaveDisplacement, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttColor, public AttExtender, public AttLineRend, public AttLineRendBase, public AttVisualOffsetHo, public AttVisualOffsetVo, public AttVisualOffset2Ho {
    public:
        Octave();
        virtual ~Octave();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Piano pedal mark. **/
class Pedal : public Element, public AttLabelled, public AttTyped, public AttPedalLog, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttPedalVis, public AttColor, public AttExtSymAuth, public AttExtSymNames, public AttLineRend, public AttLineRendBase, public AttPlacementRelStaff, public AttTypography, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        Pedal();
        virtual ~Pedal();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * In an orchestral score and its corresponding parts, a mark indicating a
 * convenient point from which to resume rehearsal after a break.
 **/
class Reh : public Element, public AttLabelled, public AttTyped, public AttLang, public AttStaffIdent, public AttStartId, public AttTimestampLog, public AttColor, public AttPlacementRelStaff, public AttTypography, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        Reh();
        virtual ~Reh();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * An instruction expressed as a combination of text and symbols – segno and coda –
 * typically above, below, or between staves, but not on the staff.
 **/
class RepeatMark : public Element, public AttLabelled, public AttTyped, public AttLang, public AttRepeatMarkLog, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttDurationAdditive, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttColor, public AttExtender, public AttLineRend, public AttLineRendBase, public AttExtSymAuth, public AttExtSymNames, public AttPlacementRelStaff, public AttVisualOffsetHo, public AttVisualOffsetVo, public AttVisualOffset2Ho {
    public:
        RepeatMark();
        virtual ~RepeatMark();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Indication of 1) a "unified melodic idea" or 2) performance technique. **/
class Slur : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttDurationAdditive, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttColor, public AttCurvature, public AttLineRendBase, public AttVisualOffsetHo, public AttVisualOffsetVo, public AttVisualOffset2Ho, public AttVisualOffset2Vo {
    public:
        Slur();
        virtual ~Slur();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * An indication that two notes of the same pitch form a single note with their
 * combined rhythmic values.
 **/
class Tie : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttColor, public AttCurvature, public AttLineRendBase, public AttVisualOffsetHo, public AttVisualOffsetVo, public AttVisualOffset2Ho, public AttVisualOffset2Vo {
    public:
        Tie();
        virtual ~Tie();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * A group of notes with "irregular" (sometimes called "irrational") rhythmic
 * values, for example, three notes in the time normally occupied by two or nine in
 * the time of five.
 **/
class Tuplet : public Element, public AttLabelled, public AttTyped, public AttDurationAdditive, public AttDurationRatio, public AttLayerIdent, public AttStaffIdent, public AttStartEndId, public AttStartId, public AttTupletVis, public AttColor, public AttNumberPlacement {
    public:
        Tuplet();
        virtual ~Tuplet();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

} // namespace libmei

#endif // __LIBMEI_CMN_H__
