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

#ifndef __LIBMEI_SHARED_H__
#define __LIBMEI_SHARED_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include "element.h"
#include "atts_cmn.h"
#include "atts_cmnornaments.h"
#include "atts_externalsymbols.h"
#include "atts_gestural.h"
#include "atts_midi.h"
#include "atts_shared.h"
#include "atts_stringtab.h"
#include "atts_visual.h"


namespace libmei {

/** Records a temporary alteration to the pitch of a note. **/
class Accid : public Element, public AttLabelled, public AttTyped, public AttAccidentalGes, public AttAccidLog, public AttAccidental, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttColor, public AttEnclosingChars, public AttExtSymAuth, public AttExtSymNames, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        Accid();
        virtual ~Accid();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * A person or organization who transcribes a musical composition, usually for a
 * different medium from that of the original; in an arrangement the musical
 * substance remains essentially unchanged.
 **/
class Arranger : public Element, public AttLabelled, public AttTyped, public AttLang {
    public:
        Arranger();
        virtual ~Arranger();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** An indication of how to play a note or chord. **/
class Artic : public Element, public AttLabelled, public AttTyped, public AttArticulation, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttColor, public AttEnclosingChars, public AttExtSymAuth, public AttExtSymNames, public AttPlacementRelEvent, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        Artic();
        virtual ~Artic();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Contains the whole of a single musical text, excluding any front or back matter. **/
class Body : public Element, public AttLabelled, public AttTyped {
    public:
        Body();
        virtual ~Body();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * Break, pause, or interruption in the normal tempo of a composition.
 * Typically indicated by "railroad tracks", i.e., two diagonal slashes.
 **/
class Caesura : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttStaffIdent, public AttStartId, public AttTimestampLog, public AttColor, public AttExtSymAuth, public AttExtSymNames, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        Caesura();
        virtual ~Caesura();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * A simultaneous sounding of two or more notes in the same layer *with the same
 * duration*.
 **/
class Chord : public Element, public AttLabelled, public AttTyped, public AttInstrumentIdent, public AttAugmentDots, public AttGraced, public AttCue, public AttDurationLog, public AttLayerIdent, public AttStaffIdent, public AttChordVis, public AttColor, public AttEnclosingChars, public AttExtSymAuth, public AttExtSymNames, public AttStems, public AttVisualOffsetHo, public AttBeamSecondary {
    public:
        Chord();
        virtual ~Chord();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * Indication of the exact location of a particular note on the staff and,
 * therefore, the other notes as well.
 **/
class Clef : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttStaffIdent, public AttClefLog, public AttClefShape, public AttLineLoc, public AttOctave, public AttOctaveDisplacement, public AttColor, public AttEnclosingChars, public AttExtSymAuth, public AttExtSymNames, public AttTypography, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        Clef();
        virtual ~Clef();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** A set of simultaneously-occurring clefs. **/
class ClefGrp : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttStaffIdent {
    public:
        ClefGrp();
        virtual ~ClefGrp();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** The name of the creator of the intellectual content of a musical work. **/
class Composer : public Element, public AttLabelled, public AttTyped, public AttLang {
    public:
        Composer();
        virtual ~Composer();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** A string identifying a point in time or the time period between two such points. **/
class Date : public Element, public AttLabelled, public AttTyped, public AttCalendared, public AttDatable, public AttLang {
    public:
        Date();
        virtual ~Date();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * An instruction expressed as a combination of text and symbols, typically above,
 * below, or between staves, but not on the staff — that is not encoded elsewhere
 * in more specific elements, like tempo, dynam or repeatMark.
 **/
class Dir : public Element, public AttLabelled, public AttTyped, public AttLang, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttDurationAdditive, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttColor, public AttExtender, public AttLineRend, public AttLineRendBase, public AttPlacementRelStaff, public AttVisualOffsetHo, public AttVisualOffsetVo, public AttVisualOffset2Ho {
    public:
        Dir();
        virtual ~Dir();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Indication of the volume of a note, phrase, or section of music. **/
class Dynam : public Element, public AttLabelled, public AttTyped, public AttLang, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttDurationAdditive, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttColor, public AttExtender, public AttLineRend, public AttLineRendBase, public AttPlacementRelStaff, public AttVisualOffsetHo, public AttVisualOffsetVo, public AttVisualOffset2Ho {
    public:
        Dynam();
        virtual ~Dynam();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * Alternative ending for a repeated passage of music; i.e., prima volta, seconda
 * volta, etc.
 **/
class Ending : public Element, public AttLabelled, public AttTyped, public AttLineRend, public AttLineRendBase {
    public:
        Ending();
        virtual ~Ending();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * A container for document text that identifies the feature to which it is
 * attached.
 * For a "tool tip" or other generated label, use the label attribute.
 **/
class Label : public Element, public AttLabelled, public AttTyped, public AttLang {
    public:
        Label();
        virtual ~Label();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** A label on the pages following the first. **/
class LabelAbbr : public Element, public AttLabelled, public AttTyped, public AttLang {
    public:
        LabelAbbr();
        virtual ~LabelAbbr();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** An independent stream of events on a staff. **/
class Layer : public Element, public AttLabelled, public AttNInteger, public AttTyped {
    public:
        Layer();
        virtual ~Layer();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** An empty formatting element that forces text to begin on a new line. **/
class Lb : public Element, public AttLabelled, public AttTyped {
    public:
        Lb();
        virtual ~Lb();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Person or organization who is a writer of the text of a song. **/
class Lyricist : public Element, public AttLabelled, public AttTyped, public AttLang {
    public:
        Lyricist();
        virtual ~Lyricist();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** (musical division) – Contains a subdivision of the body of a musical text. **/
class Mdiv : public Element, public AttLabelled, public AttTyped, public AttAttacking {
    public:
        Mdiv();
        virtual ~Mdiv();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * Contains a single MEI-conformant document, consisting of an MEI header and a
 * musical text, either in isolation or as part of an meiCorpus element.
 **/
class Mei : public Element, public AttMeiVersion {
    public:
        Mei();
        virtual ~Mei();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * Contains a single musical text of any kind, whether unitary or composite, for
 * example, an etude, opera, song cycle, symphony, or anthology of piano solos.
 **/
class Music : public Element, public AttLabelled, public AttTyped {
    public:
        Music();
        virtual ~Music();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** A single pitched event. **/
class Note : public Element, public AttLabelled, public AttTyped, public AttNoteGes, public AttInstrumentIdent, public AttMidiVelocity, public AttStringtab, public AttAugmentDots, public AttCue, public AttDurationLog, public AttLayerIdent, public AttStaffIdent, public AttGraced, public AttPitch, public AttOctave, public AttColor, public AttEnclosingChars, public AttExtSymAuth, public AttExtSymNames, public AttStems, public AttTypography, public AttVisualOffsetHo, public AttBeamSecondary {
    public:
        Note();
        virtual ~Note();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** An element indicating an ornament that is not a mordent, turn, or trill. **/
class Ornam : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttDurationAdditive, public AttOrnamentAccid, public AttOrnamentAccidGes, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttColor, public AttEnclosingChars, public AttPlacementRelStaff, public AttVisualOffsetHo, public AttVisualOffsetVo, public AttVisualOffset2Ho {
    public:
        Ornam();
        virtual ~Ornam();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** An empty formatting element that forces text to begin on a new page. **/
class Pb : public Element, public AttLabelled, public AttTyped, public AttPbVis {
    public:
        Pb();
        virtual ~Pb();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** A running footer. **/
class PgFoot : public Element, public AttLabelled, public AttTyped, public AttFormework, public AttHorizontalAlign, public AttLang {
    public:
        PgFoot();
        virtual ~PgFoot();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** A running header. **/
class PgHead : public Element, public AttLabelled, public AttTyped, public AttFormework, public AttHorizontalAlign, public AttLang {
    public:
        PgHead();
        virtual ~PgHead();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Name of the place where a bibliographic item was published. **/
class PubPlace : public Element, public AttLabelled, public AttTyped, public AttLang {
    public:
        PubPlace();
        virtual ~PubPlace();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * A formatting element indicating special visual rendering, e.g., bold or
 * italicized, of a text word or phrase.
 **/
class Rend : public Element, public AttColor, public AttLabelled, public AttTyped, public AttExtSymAuth, public AttHorizontalAlign, public AttLang, public AttTextRendition, public AttTypography, public AttVerticalAlign, public AttWhitespace {
    public:
        Rend();
        virtual ~Rend();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * Transcription of text that names one or more individuals, groups, or in rare
 * cases, mechanical processes, responsible for creation, realization, production,
 * funding, or distribution of the intellectual or artistic content.
 **/
class RespStmt : public Element, public AttLabelled, public AttTyped {
    public:
        RespStmt();
        virtual ~RespStmt();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** A non-sounding event found in the source being transcribed. **/
class Rest : public Element, public AttLabelled, public AttTyped, public AttInstrumentIdent, public AttAugmentDots, public AttCue, public AttDurationLog, public AttLayerIdent, public AttStaffIdent, public AttColor, public AttEnclosingChars, public AttBeamSecondary, public AttStaffLocPitched, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        Rest();
        virtual ~Rest();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** An empty formatting element that forces musical notation to begin on a new line. **/
class Sb : public Element, public AttLabelled, public AttTyped, public AttSbVis, public AttExtSymAuth, public AttExtSymNames, public AttTypography {
    public:
        Sb();
        virtual ~Sb();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Full score view of the musical content. **/
class Score : public Element, public AttLabelled, public AttTyped {
    public:
        Score();
        virtual ~Score();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** (score definition) – Container for score meta-information. **/
class ScoreDef : public Element, public AttLabelled, public AttTyped, public AttKeySigDefaultLog, public AttMeterSigDefaultLog {
    public:
        ScoreDef();
        virtual ~ScoreDef();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Segment of music data. **/
class Section : public Element, public AttLabelled, public AttTyped, public AttAttacking, public AttSectionVis {
    public:
        Section();
        virtual ~Section();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * A placeholder used to fill an incomplete measure, layer, etc.
 * most often so that the combined duration of the events equals the number of
 * beats in the measure.
 **/
class Space : public Element, public AttLabelled, public AttTyped, public AttAugmentDots, public AttDurationLog, public AttLayerIdent, public AttStaffIdent, public AttSpaceVis, public AttCutout {
    public:
        Space();
        virtual ~Space();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * A group of equidistant horizontal lines on which notes are placed in order to
 * represent pitch or a grouping element for individual 'strands' of notes, rests,
 * etc.
 * that may or may not actually be rendered on staff lines; that is, both
 * diastematic and non-diastematic signs.
 **/
class Staff : public Element, public AttLabelled, public AttNInteger, public AttTyped {
    public:
        Staff();
        virtual ~Staff();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Container for staff meta-information. **/
class StaffDef : public Element, public AttLabelled, public AttNInteger, public AttTyped, public AttInstrumentIdent, public AttStringtabTuning, public AttTimeBase, public AttTuning, public AttStaffDefLog, public AttCleffingLog, public AttKeySigDefaultLog, public AttMeterSigDefaultLog, public AttTransposition, public AttStaffDefVis, public AttScalable {
    public:
        StaffDef();
        virtual ~StaffDef();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** (staff group) – A group of bracketed or braced staves. **/
class StaffGrp : public Element, public AttLabelled, public AttTyped, public AttStaffGrpVis, public AttStaffGroupingSym, public AttInstrumentIdent {
    public:
        StaffGrp();
        virtual ~StaffGrp();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Individual lyric syllable. **/
class Syl : public Element, public AttLabelled, public AttTyped, public AttLang, public AttSylLog, public AttPlacementRelStaff, public AttTypography, public AttVisualOffsetHo, public AttVisualOffsetVo, public AttHorizontalAlign {
    public:
        Syl();
        virtual ~Syl();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** A reference to a previously defined symbol. **/
class Symbol : public Element, public AttLabelled, public AttTyped, public AttColor, public AttExtSymAuth, public AttExtSymNames, public AttScalable, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        Symbol();
        virtual ~Symbol();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * Text and symbols descriptive of tempo, mood, or style, e.g., "allarg.", "a
 * tempo", "cantabile", "Moderato", "♩=60", "Moderato ♩ =60").
 **/
class Tempo : public Element, public AttLabelled, public AttTyped, public AttLang, public AttMidiTempo, public AttTempoLog, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttMmTempo, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttColor, public AttExtender, public AttLineRend, public AttLineRendBase, public AttPlacementRelStaff, public AttVisualOffsetHo, public AttVisualOffsetVo, public AttVisualOffset2Ho {
    public:
        Tempo();
        virtual ~Tempo();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Title of a bibliographic entity. **/
class Title : public Element, public AttLabelled, public AttLang, public AttNNumberLike {
    public:
        Title();
        virtual ~Title();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

} // namespace libmei

#endif // __LIBMEI_SHARED_H__
