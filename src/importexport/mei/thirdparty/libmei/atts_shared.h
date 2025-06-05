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

#ifndef __LIBMEI_ATTS_SHARED_H__
#define __LIBMEI_ATTS_SHARED_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include <string>

namespace libmei {

//----------------------------------------------------------------------------
// AttAccidLog
//----------------------------------------------------------------------------

class AttAccidLog : public Att {
protected:
    AttAccidLog();
    ~AttAccidLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetAccidLog();

    /** Read the values for the attribute class **/
    bool ReadAccidLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteAccidLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetFunc(accidLog_FUNC func_) { m_func = func_; }
    accidLog_FUNC GetFunc() const { return m_func; }
    bool HasFunc() const;
    ///@}

private:
    /**
     * Indicates the function of the depressed pedal, but not necessarily the text
     * associated with its use.
     * Use the dir element for such text.
     **/
    accidLog_FUNC m_func;
};

//----------------------------------------------------------------------------
// InstAccidLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttAccidLog
 */

class InstAccidLog : public AttAccidLog {
public:
    InstAccidLog() = default;
    virtual ~InstAccidLog() = default;
};

//----------------------------------------------------------------------------
// AttAccidental
//----------------------------------------------------------------------------

class AttAccidental : public Att {
protected:
    AttAccidental();
    ~AttAccidental() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetAccidental();

    /** Read the values for the attribute class **/
    bool ReadAccidental(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteAccidental(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetAccid(data_ACCIDENTAL_WRITTEN accid_) { m_accid = accid_; }
    data_ACCIDENTAL_WRITTEN GetAccid() const { return m_accid; }
    bool HasAccid() const;
    ///@}

private:
    /** Captures a written accidental. **/
    data_ACCIDENTAL_WRITTEN m_accid;
};

//----------------------------------------------------------------------------
// InstAccidental
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttAccidental
 */

class InstAccidental : public AttAccidental {
public:
    InstAccidental() = default;
    virtual ~InstAccidental() = default;
};

//----------------------------------------------------------------------------
// AttArticulation
//----------------------------------------------------------------------------

class AttArticulation : public Att {
protected:
    AttArticulation();
    ~AttArticulation() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetArticulation();

    /** Read the values for the attribute class **/
    bool ReadArticulation(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteArticulation(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetArtic(data_ARTICULATION_List artic_) { m_artic = artic_; }
    data_ARTICULATION_List GetArtic() const { return m_artic; }
    bool HasArtic() const;
    ///@}

private:
    /**
     * Encodes the written articulation(s).
     * Articulations are normally encoded in order from the note head outward; that is,
     * away from the stem. See additional notes at att.vis.note. Only articulations
     * should be encoded in the artic attribute; for example, fingerings should be
     * encoded using the fing element.
     **/
    data_ARTICULATION_List m_artic;
};

//----------------------------------------------------------------------------
// InstArticulation
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttArticulation
 */

class InstArticulation : public AttArticulation {
public:
    InstArticulation() = default;
    virtual ~InstArticulation() = default;
};

//----------------------------------------------------------------------------
// AttAugmentDots
//----------------------------------------------------------------------------

class AttAugmentDots : public Att {
protected:
    AttAugmentDots();
    ~AttAugmentDots() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetAugmentDots();

    /** Read the values for the attribute class **/
    bool ReadAugmentDots(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteAugmentDots(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetDots(int dots_) { m_dots = dots_; }
    int GetDots() const { return m_dots; }
    bool HasDots() const;
    ///@}

private:
    /** Records the number of augmentation dots required by a written dotted duration. **/
    int m_dots;
};

//----------------------------------------------------------------------------
// InstAugmentDots
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttAugmentDots
 */

class InstAugmentDots : public AttAugmentDots {
public:
    InstAugmentDots() = default;
    virtual ~InstAugmentDots() = default;
};

//----------------------------------------------------------------------------
// AttCalendared
//----------------------------------------------------------------------------

class AttCalendared : public Att {
protected:
    AttCalendared();
    ~AttCalendared() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetCalendared();

    /** Read the values for the attribute class **/
    bool ReadCalendared(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteCalendared(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetCalendar(std::string calendar_) { m_calendar = calendar_; }
    std::string GetCalendar() const { return m_calendar; }
    bool HasCalendar() const;
    ///@}

private:
    /**
     * Indicates the calendar system to which a date belongs, for example, Gregorian,
     * Julian, Roman, Mosaic, Revolutionary, Islamic, etc.
     **/
    std::string m_calendar;
};

//----------------------------------------------------------------------------
// InstCalendared
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttCalendared
 */

class InstCalendared : public AttCalendared {
public:
    InstCalendared() = default;
    virtual ~InstCalendared() = default;
};

//----------------------------------------------------------------------------
// AttClefLog
//----------------------------------------------------------------------------

class AttClefLog : public Att {
protected:
    AttClefLog();
    ~AttClefLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetClefLog();

    /** Read the values for the attribute class **/
    bool ReadClefLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteClefLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetCautionary(data_BOOLEAN cautionary_) { m_cautionary = cautionary_; }
    data_BOOLEAN GetCautionary() const { return m_cautionary; }
    bool HasCautionary() const;
    ///@}

private:
    /**
     * Records the function of the clef.
     * A "cautionary" clef does not change the following pitches.
     **/
    data_BOOLEAN m_cautionary;
};

//----------------------------------------------------------------------------
// InstClefLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttClefLog
 */

class InstClefLog : public AttClefLog {
public:
    InstClefLog() = default;
    virtual ~InstClefLog() = default;
};

//----------------------------------------------------------------------------
// AttClefShape
//----------------------------------------------------------------------------

class AttClefShape : public Att {
protected:
    AttClefShape();
    ~AttClefShape() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetClefShape();

    /** Read the values for the attribute class **/
    bool ReadClefShape(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteClefShape(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetShape(data_CLEFSHAPE shape_) { m_shape = shape_; }
    data_CLEFSHAPE GetShape() const { return m_shape; }
    bool HasShape() const;
    ///@}

private:
    /** Describes a clef’s shape. **/
    data_CLEFSHAPE m_shape;
};

//----------------------------------------------------------------------------
// InstClefShape
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttClefShape
 */

class InstClefShape : public AttClefShape {
public:
    InstClefShape() = default;
    virtual ~InstClefShape() = default;
};

//----------------------------------------------------------------------------
// AttCleffingLog
//----------------------------------------------------------------------------

class AttCleffingLog : public Att {
protected:
    AttCleffingLog();
    ~AttCleffingLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetCleffingLog();

    /** Read the values for the attribute class **/
    bool ReadCleffingLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteCleffingLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetClefShape(data_CLEFSHAPE clefShape_) { m_clefShape = clefShape_; }
    data_CLEFSHAPE GetClefShape() const { return m_clefShape; }
    bool HasClefShape() const;
    //
    void SetClefLine(char clefLine_) { m_clefLine = clefLine_; }
    char GetClefLine() const { return m_clefLine; }
    bool HasClefLine() const;
    //
    void SetClefDis(data_OCTAVE_DIS clefDis_) { m_clefDis = clefDis_; }
    data_OCTAVE_DIS GetClefDis() const { return m_clefDis; }
    bool HasClefDis() const;
    //
    void SetClefDisPlace(data_STAFFREL_basic clefDisPlace_) { m_clefDisPlace = clefDisPlace_; }
    data_STAFFREL_basic GetClefDisPlace() const { return m_clefDisPlace; }
    bool HasClefDisPlace() const;
    ///@}

private:
    /** Encodes a value for the clef symbol. **/
    data_CLEFSHAPE m_clefShape;
    /**
     * Contains a default value for the position of the clef.
     * The value must be in the range between 1 and the number of lines on the staff.
     * The numbering of lines starts with the lowest line of the staff.
     **/
    char m_clefLine;
    /** Records the amount of octave displacement to be applied to the clef. **/
    data_OCTAVE_DIS m_clefDis;
    /** Records the direction of octave displacement to be applied to the clef. **/
    data_STAFFREL_basic m_clefDisPlace;
};

//----------------------------------------------------------------------------
// InstCleffingLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttCleffingLog
 */

class InstCleffingLog : public AttCleffingLog {
public:
    InstCleffingLog() = default;
    virtual ~InstCleffingLog() = default;
};

//----------------------------------------------------------------------------
// AttColor
//----------------------------------------------------------------------------

class AttColor : public Att {
protected:
    AttColor();
    ~AttColor() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetColor();

    /** Read the values for the attribute class **/
    bool ReadColor(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteColor(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetColor(std::string color_) { m_color = color_; }
    std::string GetColor() const { return m_color; }
    bool HasColor() const;
    ///@}

private:
    /**
     * Used to indicate visual appearance.
     * Do not confuse this with the musical term 'color' as used in pre-CMN notation.
     **/
    std::string m_color;
};

//----------------------------------------------------------------------------
// InstColor
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttColor
 */

class InstColor : public AttColor {
public:
    InstColor() = default;
    virtual ~InstColor() = default;
};

//----------------------------------------------------------------------------
// AttCue
//----------------------------------------------------------------------------

class AttCue : public Att {
protected:
    AttCue();
    ~AttCue() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetCue();

    /** Read the values for the attribute class **/
    bool ReadCue(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteCue(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetCue(data_BOOLEAN cue_) { m_cue = cue_; }
    data_BOOLEAN GetCue() const { return m_cue; }
    bool HasCue() const;
    ///@}

private:
    /** --- **/
    data_BOOLEAN m_cue;
};

//----------------------------------------------------------------------------
// InstCue
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttCue
 */

class InstCue : public AttCue {
public:
    InstCue() = default;
    virtual ~InstCue() = default;
};

//----------------------------------------------------------------------------
// AttCurvature
//----------------------------------------------------------------------------

class AttCurvature : public Att {
protected:
    AttCurvature();
    ~AttCurvature() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetCurvature();

    /** Read the values for the attribute class **/
    bool ReadCurvature(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteCurvature(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetCurvedir(curvature_CURVEDIR curvedir_) { m_curvedir = curvedir_; }
    curvature_CURVEDIR GetCurvedir() const { return m_curvedir; }
    bool HasCurvedir() const;
    ///@}

private:
    /** Describes a curve with a generic term indicating the direction of curvature. **/
    curvature_CURVEDIR m_curvedir;
};

//----------------------------------------------------------------------------
// InstCurvature
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttCurvature
 */

class InstCurvature : public AttCurvature {
public:
    InstCurvature() = default;
    virtual ~InstCurvature() = default;
};

//----------------------------------------------------------------------------
// AttDataPointing
//----------------------------------------------------------------------------

class AttDataPointing : public Att {
protected:
    AttDataPointing();
    ~AttDataPointing() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetDataPointing();

    /** Read the values for the attribute class **/
    bool ReadDataPointing(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteDataPointing(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetData(std::string data_) { m_data = data_; }
    std::string GetData() const { return m_data; }
    bool HasData() const;
    ///@}

private:
    /** Used to link metadata elements to one or more data-containing elements. **/
    std::string m_data;
};

//----------------------------------------------------------------------------
// InstDataPointing
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttDataPointing
 */

class InstDataPointing : public AttDataPointing {
public:
    InstDataPointing() = default;
    virtual ~InstDataPointing() = default;
};

//----------------------------------------------------------------------------
// AttDatable
//----------------------------------------------------------------------------

class AttDatable : public Att {
protected:
    AttDatable();
    ~AttDatable() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetDatable();

    /** Read the values for the attribute class **/
    bool ReadDatable(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteDatable(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetEnddate(std::string enddate_) { m_enddate = enddate_; }
    std::string GetEnddate() const { return m_enddate; }
    bool HasEnddate() const;
    //
    void SetIsodate(std::string isodate_) { m_isodate = isodate_; }
    std::string GetIsodate() const { return m_isodate; }
    bool HasIsodate() const;
    //
    void SetNotafter(std::string notafter_) { m_notafter = notafter_; }
    std::string GetNotafter() const { return m_notafter; }
    bool HasNotafter() const;
    //
    void SetNotbefore(std::string notbefore_) { m_notbefore = notbefore_; }
    std::string GetNotbefore() const { return m_notbefore; }
    bool HasNotbefore() const;
    //
    void SetStartdate(std::string startdate_) { m_startdate = startdate_; }
    std::string GetStartdate() const { return m_startdate; }
    bool HasStartdate() const;
    ///@}

private:
    /** Contains the end point of a date range in standard ISO form. **/
    std::string m_enddate;
    /** Provides the value of a textual date in standard ISO form. **/
    std::string m_isodate;
    /** Contains an upper boundary for an uncertain date in standard ISO form. **/
    std::string m_notafter;
    /** Contains a lower boundary, in standard ISO form, for an uncertain date. **/
    std::string m_notbefore;
    /** Contains the starting point of a date range in standard ISO form. **/
    std::string m_startdate;
};

//----------------------------------------------------------------------------
// InstDatable
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttDatable
 */

class InstDatable : public AttDatable {
public:
    InstDatable() = default;
    virtual ~InstDatable() = default;
};

//----------------------------------------------------------------------------
// AttDurationAdditive
//----------------------------------------------------------------------------

class AttDurationAdditive : public Att {
protected:
    AttDurationAdditive();
    ~AttDurationAdditive() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetDurationAdditive();

    /** Read the values for the attribute class **/
    bool ReadDurationAdditive(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteDurationAdditive(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetDur(data_DURATION dur_) { m_dur = dur_; }
    data_DURATION GetDur() const { return m_dur; }
    bool HasDur() const;
    ///@}

private:
    /**
     * When a duration cannot be represented as a single power-of-two value, multiple
     * space-separated values that add up to the total duration may be used.
     **/
    data_DURATION m_dur;
};

//----------------------------------------------------------------------------
// InstDurationAdditive
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttDurationAdditive
 */

class InstDurationAdditive : public AttDurationAdditive {
public:
    InstDurationAdditive() = default;
    virtual ~InstDurationAdditive() = default;
};

//----------------------------------------------------------------------------
// AttDurationLog
//----------------------------------------------------------------------------

class AttDurationLog : public Att {
protected:
    AttDurationLog();
    ~AttDurationLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetDurationLog();

    /** Read the values for the attribute class **/
    bool ReadDurationLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteDurationLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetDur(data_DURATION dur_) { m_dur = dur_; }
    data_DURATION GetDur() const { return m_dur; }
    bool HasDur() const;
    ///@}

private:
    /**
     * When a duration cannot be represented as a single power-of-two value, multiple
     * space-separated values that add up to the total duration may be used.
     **/
    data_DURATION m_dur;
};

//----------------------------------------------------------------------------
// InstDurationLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttDurationLog
 */

class InstDurationLog : public AttDurationLog {
public:
    InstDurationLog() = default;
    virtual ~InstDurationLog() = default;
};

//----------------------------------------------------------------------------
// AttDurationRatio
//----------------------------------------------------------------------------

class AttDurationRatio : public Att {
protected:
    AttDurationRatio();
    ~AttDurationRatio() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetDurationRatio();

    /** Read the values for the attribute class **/
    bool ReadDurationRatio(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteDurationRatio(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetNum(int num_) { m_num = num_; }
    int GetNum() const { return m_num; }
    bool HasNum() const;
    //
    void SetNumbase(int numbase_) { m_numbase = numbase_; }
    int GetNumbase() const { return m_numbase; }
    bool HasNumbase() const;
    ///@}

private:
    /** Records a number or count accompanying a notational feature. **/
    int m_num;
    /**
     * Along with num, describes duration as a ratio.
     * num is the first value in the ratio, while numbase is the second.
     **/
    int m_numbase;
};

//----------------------------------------------------------------------------
// InstDurationRatio
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttDurationRatio
 */

class InstDurationRatio : public AttDurationRatio {
public:
    InstDurationRatio() = default;
    virtual ~InstDurationRatio() = default;
};

//----------------------------------------------------------------------------
// AttEnclosingChars
//----------------------------------------------------------------------------

class AttEnclosingChars : public Att {
protected:
    AttEnclosingChars();
    ~AttEnclosingChars() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetEnclosingChars();

    /** Read the values for the attribute class **/
    bool ReadEnclosingChars(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteEnclosingChars(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetEnclose(data_ENCLOSURE enclose_) { m_enclose = enclose_; }
    data_ENCLOSURE GetEnclose() const { return m_enclose; }
    bool HasEnclose() const;
    ///@}

private:
    /**
     * Records the characters often used to mark accidentals, articulations, and
     * sometimes notes as having a cautionary or editorial function.
     * For an example of cautionary accidentals enclosed in parentheses, see Read, p.
     * 131, ex. 9-14.
     **/
    data_ENCLOSURE m_enclose;
};

//----------------------------------------------------------------------------
// InstEnclosingChars
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttEnclosingChars
 */

class InstEnclosingChars : public AttEnclosingChars {
public:
    InstEnclosingChars() = default;
    virtual ~InstEnclosingChars() = default;
};

//----------------------------------------------------------------------------
// AttExtender
//----------------------------------------------------------------------------

class AttExtender : public Att {
protected:
    AttExtender();
    ~AttExtender() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetExtender();

    /** Read the values for the attribute class **/
    bool ReadExtender(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteExtender(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetExtender(data_BOOLEAN extender_) { m_extender = extender_; }
    data_BOOLEAN GetExtender() const { return m_extender; }
    bool HasExtender() const;
    ///@}

private:
    /** Indicates the presence of an extension symbol, typically a line. **/
    data_BOOLEAN m_extender;
};

//----------------------------------------------------------------------------
// InstExtender
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttExtender
 */

class InstExtender : public AttExtender {
public:
    InstExtender() = default;
    virtual ~InstExtender() = default;
};

//----------------------------------------------------------------------------
// AttFormework
//----------------------------------------------------------------------------

class AttFormework : public Att {
protected:
    AttFormework();
    ~AttFormework() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetFormework();

    /** Read the values for the attribute class **/
    bool ReadFormework(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteFormework(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetFunc(data_PGFUNC func_) { m_func = func_; }
    data_PGFUNC GetFunc() const { return m_func; }
    bool HasFunc() const;
    ///@}

private:
    /**
     * Indicates the function of the depressed pedal, but not necessarily the text
     * associated with its use.
     * Use the dir element for such text.
     **/
    data_PGFUNC m_func;
};

//----------------------------------------------------------------------------
// InstFormework
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttFormework
 */

class InstFormework : public AttFormework {
public:
    InstFormework() = default;
    virtual ~InstFormework() = default;
};

//----------------------------------------------------------------------------
// AttHorizontalAlign
//----------------------------------------------------------------------------

class AttHorizontalAlign : public Att {
protected:
    AttHorizontalAlign();
    ~AttHorizontalAlign() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetHorizontalAlign();

    /** Read the values for the attribute class **/
    bool ReadHorizontalAlign(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteHorizontalAlign(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetHalign(data_HORIZONTALALIGNMENT halign_) { m_halign = halign_; }
    data_HORIZONTALALIGNMENT GetHalign() const { return m_halign; }
    bool HasHalign() const;
    ///@}

private:
    /** Records horizontal alignment. **/
    data_HORIZONTALALIGNMENT m_halign;
};

//----------------------------------------------------------------------------
// InstHorizontalAlign
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttHorizontalAlign
 */

class InstHorizontalAlign : public AttHorizontalAlign {
public:
    InstHorizontalAlign() = default;
    virtual ~InstHorizontalAlign() = default;
};

//----------------------------------------------------------------------------
// AttKeySigDefaultLog
//----------------------------------------------------------------------------

class AttKeySigDefaultLog : public Att {
protected:
    AttKeySigDefaultLog();
    ~AttKeySigDefaultLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetKeySigDefaultLog();

    /** Read the values for the attribute class **/
    bool ReadKeySigDefaultLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteKeySigDefaultLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetKeysig(data_KEYSIGNATURE keysig_) { m_keysig = keysig_; }
    data_KEYSIGNATURE GetKeysig() const { return m_keysig; }
    bool HasKeysig() const;
    ///@}

private:
    /** Written key signature. **/
    data_KEYSIGNATURE m_keysig;
};

//----------------------------------------------------------------------------
// InstKeySigDefaultLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttKeySigDefaultLog
 */

class InstKeySigDefaultLog : public AttKeySigDefaultLog {
public:
    InstKeySigDefaultLog() = default;
    virtual ~InstKeySigDefaultLog() = default;
};

//----------------------------------------------------------------------------
// AttLabelled
//----------------------------------------------------------------------------

class AttLabelled : public Att {
protected:
    AttLabelled();
    ~AttLabelled() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetLabelled();

    /** Read the values for the attribute class **/
    bool ReadLabelled(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteLabelled(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetLabel(std::string label_) { m_label = label_; }
    std::string GetLabel() const { return m_label; }
    bool HasLabel() const;
    ///@}

private:
    /**
     * Captures text to be used to generate a label for the element to which it’s
     * attached, a "tool tip" or prefatory text, for example.
     * Should not be used to record document content.
     **/
    std::string m_label;
};

//----------------------------------------------------------------------------
// InstLabelled
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttLabelled
 */

class InstLabelled : public AttLabelled {
public:
    InstLabelled() = default;
    virtual ~InstLabelled() = default;
};

//----------------------------------------------------------------------------
// AttLang
//----------------------------------------------------------------------------

class AttLang : public Att {
protected:
    AttLang();
    ~AttLang() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetLang();

    /** Read the values for the attribute class **/
    bool ReadLang(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteLang(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetLang(std::string lang_) { m_lang = lang_; }
    std::string GetLang() const { return m_lang; }
    bool HasLang() const;
    ///@}

private:
    /** --- **/
    std::string m_lang;
};

//----------------------------------------------------------------------------
// InstLang
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttLang
 */

class InstLang : public AttLang {
public:
    InstLang() = default;
    virtual ~InstLang() = default;
};

//----------------------------------------------------------------------------
// AttLayerIdent
//----------------------------------------------------------------------------

class AttLayerIdent : public Att {
protected:
    AttLayerIdent();
    ~AttLayerIdent() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetLayerIdent();

    /** Read the values for the attribute class **/
    bool ReadLayerIdent(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteLayerIdent(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetLayer(int layer_) { m_layer = layer_; }
    int GetLayer() const { return m_layer; }
    bool HasLayer() const;
    ///@}

private:
    /** Identifies the layer to which a feature applies. **/
    int m_layer;
};

//----------------------------------------------------------------------------
// InstLayerIdent
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttLayerIdent
 */

class InstLayerIdent : public AttLayerIdent {
public:
    InstLayerIdent() = default;
    virtual ~InstLayerIdent() = default;
};

//----------------------------------------------------------------------------
// AttLineLoc
//----------------------------------------------------------------------------

class AttLineLoc : public Att {
protected:
    AttLineLoc();
    ~AttLineLoc() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetLineLoc();

    /** Read the values for the attribute class **/
    bool ReadLineLoc(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteLineLoc(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetLine(char line_) { m_line = line_; }
    char GetLine() const { return m_line; }
    bool HasLine() const;
    ///@}

private:
    /**
     * Indicates the line upon which a feature stands.
     * The value must be in the range between 1 and the number of lines on the staff.
     * The numbering of lines starts with the lowest line of the staff.
     **/
    char m_line;
};

//----------------------------------------------------------------------------
// InstLineLoc
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttLineLoc
 */

class InstLineLoc : public AttLineLoc {
public:
    InstLineLoc() = default;
    virtual ~InstLineLoc() = default;
};

//----------------------------------------------------------------------------
// AttLineRend
//----------------------------------------------------------------------------

class AttLineRend : public Att {
protected:
    AttLineRend();
    ~AttLineRend() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetLineRend();

    /** Read the values for the attribute class **/
    bool ReadLineRend(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteLineRend(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetLendsym(data_LINESTARTENDSYMBOL lendsym_) { m_lendsym = lendsym_; }
    data_LINESTARTENDSYMBOL GetLendsym() const { return m_lendsym; }
    bool HasLendsym() const;
    //
    void SetLendsymSize(int lendsymSize_) { m_lendsymSize = lendsymSize_; }
    int GetLendsymSize() const { return m_lendsymSize; }
    bool HasLendsymSize() const;
    //
    void SetLstartsym(data_LINESTARTENDSYMBOL lstartsym_) { m_lstartsym = lstartsym_; }
    data_LINESTARTENDSYMBOL GetLstartsym() const { return m_lstartsym; }
    bool HasLstartsym() const;
    //
    void SetLstartsymSize(int lstartsymSize_) { m_lstartsymSize = lstartsymSize_; }
    int GetLstartsymSize() const { return m_lstartsymSize; }
    bool HasLstartsymSize() const;
    ///@}

private:
    /** Symbol rendered at end of line. **/
    data_LINESTARTENDSYMBOL m_lendsym;
    /** Holds the relative size of the line-end symbol. **/
    int m_lendsymSize;
    /** Symbol rendered at start of line. **/
    data_LINESTARTENDSYMBOL m_lstartsym;
    /** Holds the relative size of the line-start symbol. **/
    int m_lstartsymSize;
};

//----------------------------------------------------------------------------
// InstLineRend
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttLineRend
 */

class InstLineRend : public AttLineRend {
public:
    InstLineRend() = default;
    virtual ~InstLineRend() = default;
};

//----------------------------------------------------------------------------
// AttLineRendBase
//----------------------------------------------------------------------------

class AttLineRendBase : public Att {
protected:
    AttLineRendBase();
    ~AttLineRendBase() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetLineRendBase();

    /** Read the values for the attribute class **/
    bool ReadLineRendBase(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteLineRendBase(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetLform(data_LINEFORM lform_) { m_lform = lform_; }
    data_LINEFORM GetLform() const { return m_lform; }
    bool HasLform() const;
    //
    void SetLwidth(data_LINEWIDTH lwidth_) { m_lwidth = lwidth_; }
    data_LINEWIDTH GetLwidth() const { return m_lwidth; }
    bool HasLwidth() const;
    /** Getter for reference (for alternate type only) */
    data_LINEWIDTH *GetLwidthAlternate() { return &m_lwidth; }
    //
    void SetLsegs(int lsegs_) { m_lsegs = lsegs_; }
    int GetLsegs() const { return m_lsegs; }
    bool HasLsegs() const;
    ///@}

private:
    /** Describes the style of a line. **/
    data_LINEFORM m_lform;
    /** Width of a line. **/
    data_LINEWIDTH m_lwidth;
    /**
     * Describes the number of segments into which a dashed or dotted line may be
     * divided, or the number of "peaks" of a wavy line; a pair of space-separated
     * values (minimum and maximum, respectively) provides a range between which a
     * rendering system-supplied value may fall, while a single value indicates a fixed
     * amount of space; that is, the minimum and maximum values are equal.
     **/
    int m_lsegs;
};

//----------------------------------------------------------------------------
// InstLineRendBase
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttLineRendBase
 */

class InstLineRendBase : public AttLineRendBase {
public:
    InstLineRendBase() = default;
    virtual ~InstLineRendBase() = default;
};

//----------------------------------------------------------------------------
// AttMeiVersion
//----------------------------------------------------------------------------

class AttMeiVersion : public Att {
protected:
    AttMeiVersion();
    ~AttMeiVersion() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetMeiVersion();

    /** Read the values for the attribute class **/
    bool ReadMeiVersion(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteMeiVersion(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetMeiversion(meiVersion_MEIVERSION meiversion_) { m_meiversion = meiversion_; }
    meiVersion_MEIVERSION GetMeiversion() const { return m_meiversion; }
    bool HasMeiversion() const;
    ///@}

private:
    /** Specifies a generic MEI version label. **/
    meiVersion_MEIVERSION m_meiversion;
};

//----------------------------------------------------------------------------
// InstMeiVersion
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttMeiVersion
 */

class InstMeiVersion : public AttMeiVersion {
public:
    InstMeiVersion() = default;
    virtual ~InstMeiVersion() = default;
};

//----------------------------------------------------------------------------
// AttMeterConformanceBar
//----------------------------------------------------------------------------

class AttMeterConformanceBar : public Att {
protected:
    AttMeterConformanceBar();
    ~AttMeterConformanceBar() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetMeterConformanceBar();

    /** Read the values for the attribute class **/
    bool ReadMeterConformanceBar(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteMeterConformanceBar(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetMetcon(data_BOOLEAN metcon_) { m_metcon = metcon_; }
    data_BOOLEAN GetMetcon() const { return m_metcon; }
    bool HasMetcon() const;
    ///@}

private:
    /**
     * Indicates the relationship between the content of a measure and the prevailing
     * meter.
     **/
    data_BOOLEAN m_metcon;
};

//----------------------------------------------------------------------------
// InstMeterConformanceBar
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttMeterConformanceBar
 */

class InstMeterConformanceBar : public AttMeterConformanceBar {
public:
    InstMeterConformanceBar() = default;
    virtual ~InstMeterConformanceBar() = default;
};

//----------------------------------------------------------------------------
// AttMeterSigDefaultLog
//----------------------------------------------------------------------------

class AttMeterSigDefaultLog : public Att {
protected:
    AttMeterSigDefaultLog();
    ~AttMeterSigDefaultLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetMeterSigDefaultLog();

    /** Read the values for the attribute class **/
    bool ReadMeterSigDefaultLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteMeterSigDefaultLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetMeterCount(data_METERCOUNT_pair meterCount_) { m_meterCount = meterCount_; }
    data_METERCOUNT_pair GetMeterCount() const { return m_meterCount; }
    bool HasMeterCount() const;
    //
    void SetMeterUnit(int meterUnit_) { m_meterUnit = meterUnit_; }
    int GetMeterUnit() const { return m_meterUnit; }
    bool HasMeterUnit() const;
    //
    void SetMeterSym(data_METERSIGN meterSym_) { m_meterSym = meterSym_; }
    data_METERSIGN GetMeterSym() const { return m_meterSym; }
    bool HasMeterSym() const;
    ///@}

private:
    /**
     * Captures the number of beats in a measure, that is, the top number of the meter
     * signature.
     * It must contain a decimal number or an expression that evaluates to a decimal
     * number, such as 2+3 or 3*2.
     **/
    data_METERCOUNT_pair m_meterCount;
    /**
     * Contains the number indicating the beat unit, that is, the bottom number of the
     * meter signature.
     **/
    int m_meterUnit;
    /**
     * Indicates the use of a meter symbol instead of a numeric meter signature, that
     * is, 'C' for common time or 'C' with a slash for cut time.
     **/
    data_METERSIGN m_meterSym;
};

//----------------------------------------------------------------------------
// InstMeterSigDefaultLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttMeterSigDefaultLog
 */

class InstMeterSigDefaultLog : public AttMeterSigDefaultLog {
public:
    InstMeterSigDefaultLog() = default;
    virtual ~InstMeterSigDefaultLog() = default;
};

//----------------------------------------------------------------------------
// AttMmTempo
//----------------------------------------------------------------------------

class AttMmTempo : public Att {
protected:
    AttMmTempo();
    ~AttMmTempo() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetMmTempo();

    /** Read the values for the attribute class **/
    bool ReadMmTempo(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteMmTempo(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetMm(double mm_) { m_mm = mm_; }
    double GetMm() const { return m_mm; }
    bool HasMm() const;
    //
    void SetMmUnit(data_DURATION mmUnit_) { m_mmUnit = mmUnit_; }
    data_DURATION GetMmUnit() const { return m_mmUnit; }
    bool HasMmUnit() const;
    //
    void SetMmDots(int mmDots_) { m_mmDots = mmDots_; }
    int GetMmDots() const { return m_mmDots; }
    bool HasMmDots() const;
    ///@}

private:
    /**
     * Used to describe tempo in terms of beats (often the meter signature denominator)
     * per minute, ala M.M.
     * (Maelzel’s Metronome). Do not confuse this attribute with midi.bpm or midi.mspb.
     * In MIDI, a beat is always defined as a quarter note, *not the numerator of the
     * time signature or the metronomic indication*.
     **/
    double m_mm;
    /** Captures the metronomic unit. **/
    data_DURATION m_mmUnit;
    /** Records the number of augmentation dots required by a dotted metronome unit. **/
    int m_mmDots;
};

//----------------------------------------------------------------------------
// InstMmTempo
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttMmTempo
 */

class InstMmTempo : public AttMmTempo {
public:
    InstMmTempo() = default;
    virtual ~InstMmTempo() = default;
};

//----------------------------------------------------------------------------
// AttNInteger
//----------------------------------------------------------------------------

class AttNInteger : public Att {
protected:
    AttNInteger();
    ~AttNInteger() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetNInteger();

    /** Read the values for the attribute class **/
    bool ReadNInteger(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteNInteger(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetN(int n_) { m_n = n_; }
    int GetN() const { return m_n; }
    bool HasN() const;
    ///@}

private:
    /**
     * Provides a numeric designation that indicates an element’s position in a
     * sequence of similar elements.
     * Its value must be a non-negative integer.
     **/
    int m_n;
};

//----------------------------------------------------------------------------
// InstNInteger
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttNInteger
 */

class InstNInteger : public AttNInteger {
public:
    InstNInteger() = default;
    virtual ~InstNInteger() = default;
};

//----------------------------------------------------------------------------
// AttNNumberLike
//----------------------------------------------------------------------------

class AttNNumberLike : public Att {
protected:
    AttNNumberLike();
    ~AttNNumberLike() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetNNumberLike();

    /** Read the values for the attribute class **/
    bool ReadNNumberLike(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteNNumberLike(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetN(std::string n_) { m_n = n_; }
    std::string GetN() const { return m_n; }
    bool HasN() const;
    ///@}

private:
    /**
     * Provides a numeric designation that indicates an element’s position in a
     * sequence of similar elements.
     * Its value must be a non-negative integer.
     **/
    std::string m_n;
};

//----------------------------------------------------------------------------
// InstNNumberLike
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttNNumberLike
 */

class InstNNumberLike : public AttNNumberLike {
public:
    InstNNumberLike() = default;
    virtual ~InstNNumberLike() = default;
};

//----------------------------------------------------------------------------
// AttName
//----------------------------------------------------------------------------

class AttName : public Att {
protected:
    AttName();
    ~AttName() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetName();

    /** Read the values for the attribute class **/
    bool ReadName(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteName(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetNymref(std::string nymref_) { m_nymref = nymref_; }
    std::string GetNymref() const { return m_nymref; }
    bool HasNymref() const;
    //
    void SetRole(std::string role_) { m_role = role_; }
    std::string GetRole() const { return m_role; }
    bool HasRole() const;
    ///@}

private:
    /**
     * Used to record a pointer to the regularized form of the name elsewhere in the
     * document.
     **/
    std::string m_nymref;
    /**
     * Used to specify further information about the entity referenced by this name,
     * for example, the occupation of a person or the status of a place.
     **/
    std::string m_role;
};

//----------------------------------------------------------------------------
// InstName
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttName
 */

class InstName : public AttName {
public:
    InstName() = default;
    virtual ~InstName() = default;
};

//----------------------------------------------------------------------------
// AttOctave
//----------------------------------------------------------------------------

class AttOctave : public Att {
protected:
    AttOctave();
    ~AttOctave() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetOctave();

    /** Read the values for the attribute class **/
    bool ReadOctave(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteOctave(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetOct(data_OCTAVE oct_) { m_oct = oct_; }
    data_OCTAVE GetOct() const { return m_oct; }
    bool HasOct() const;
    ///@}

private:
    /** Captures written octave information. **/
    data_OCTAVE m_oct;
};

//----------------------------------------------------------------------------
// InstOctave
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttOctave
 */

class InstOctave : public AttOctave {
public:
    InstOctave() = default;
    virtual ~InstOctave() = default;
};

//----------------------------------------------------------------------------
// AttOctaveDisplacement
//----------------------------------------------------------------------------

class AttOctaveDisplacement : public Att {
protected:
    AttOctaveDisplacement();
    ~AttOctaveDisplacement() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetOctaveDisplacement();

    /** Read the values for the attribute class **/
    bool ReadOctaveDisplacement(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteOctaveDisplacement(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetDis(data_OCTAVE_DIS dis_) { m_dis = dis_; }
    data_OCTAVE_DIS GetDis() const { return m_dis; }
    bool HasDis() const;
    //
    void SetDisPlace(data_STAFFREL_basic disPlace_) { m_disPlace = disPlace_; }
    data_STAFFREL_basic GetDisPlace() const { return m_disPlace; }
    bool HasDisPlace() const;
    ///@}

private:
    /** Records the amount of octave displacement. **/
    data_OCTAVE_DIS m_dis;
    /** Records the direction of octave displacement. **/
    data_STAFFREL_basic m_disPlace;
};

//----------------------------------------------------------------------------
// InstOctaveDisplacement
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttOctaveDisplacement
 */

class InstOctaveDisplacement : public AttOctaveDisplacement {
public:
    InstOctaveDisplacement() = default;
    virtual ~InstOctaveDisplacement() = default;
};

//----------------------------------------------------------------------------
// AttPitch
//----------------------------------------------------------------------------

class AttPitch : public Att {
protected:
    AttPitch();
    ~AttPitch() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetPitch();

    /** Read the values for the attribute class **/
    bool ReadPitch(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WritePitch(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetPname(data_PITCHNAME pname_) { m_pname = pname_; }
    data_PITCHNAME GetPname() const { return m_pname; }
    bool HasPname() const;
    ///@}

private:
    /** Contains a written pitch name. **/
    data_PITCHNAME m_pname;
};

//----------------------------------------------------------------------------
// InstPitch
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttPitch
 */

class InstPitch : public AttPitch {
public:
    InstPitch() = default;
    virtual ~InstPitch() = default;
};

//----------------------------------------------------------------------------
// AttPlacementRelEvent
//----------------------------------------------------------------------------

class AttPlacementRelEvent : public Att {
protected:
    AttPlacementRelEvent();
    ~AttPlacementRelEvent() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetPlacementRelEvent();

    /** Read the values for the attribute class **/
    bool ReadPlacementRelEvent(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WritePlacementRelEvent(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetPlace(data_STAFFREL place_) { m_place = place_; }
    data_STAFFREL GetPlace() const { return m_place; }
    bool HasPlace() const;
    ///@}

private:
    /**
     * Captures the placement of the item with respect to the event with which it is
     * associated.
     **/
    data_STAFFREL m_place;
};

//----------------------------------------------------------------------------
// InstPlacementRelEvent
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttPlacementRelEvent
 */

class InstPlacementRelEvent : public AttPlacementRelEvent {
public:
    InstPlacementRelEvent() = default;
    virtual ~InstPlacementRelEvent() = default;
};

//----------------------------------------------------------------------------
// AttPlacementRelStaff
//----------------------------------------------------------------------------

class AttPlacementRelStaff : public Att {
protected:
    AttPlacementRelStaff();
    ~AttPlacementRelStaff() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetPlacementRelStaff();

    /** Read the values for the attribute class **/
    bool ReadPlacementRelStaff(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WritePlacementRelStaff(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetPlace(data_STAFFREL place_) { m_place = place_; }
    data_STAFFREL GetPlace() const { return m_place; }
    bool HasPlace() const;
    ///@}

private:
    /**
     * Captures the placement of the item with respect to the event with which it is
     * associated.
     **/
    data_STAFFREL m_place;
};

//----------------------------------------------------------------------------
// InstPlacementRelStaff
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttPlacementRelStaff
 */

class InstPlacementRelStaff : public AttPlacementRelStaff {
public:
    InstPlacementRelStaff() = default;
    virtual ~InstPlacementRelStaff() = default;
};

//----------------------------------------------------------------------------
// AttPlist
//----------------------------------------------------------------------------

class AttPlist : public Att {
protected:
    AttPlist();
    ~AttPlist() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetPlist();

    /** Read the values for the attribute class **/
    bool ReadPlist(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WritePlist(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetPlist(xsdAnyURI_List plist_) { m_plist = plist_; }
    xsdAnyURI_List GetPlist() const { return m_plist; }
    bool HasPlist() const;
    ///@}

private:
    /**
     * When the target attribute is present, plist identifies the active participants;
     * that is, those entities pointed "from", in a relationship with the specified
     * target(s).
     * When the target attribute is not present, it identifies participants in a mutual
     * relationship.
     **/
    xsdAnyURI_List m_plist;
};

//----------------------------------------------------------------------------
// InstPlist
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttPlist
 */

class InstPlist : public AttPlist {
public:
    InstPlist() = default;
    virtual ~InstPlist() = default;
};

//----------------------------------------------------------------------------
// AttRepeatMarkLog
//----------------------------------------------------------------------------

class AttRepeatMarkLog : public Att {
protected:
    AttRepeatMarkLog();
    ~AttRepeatMarkLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetRepeatMarkLog();

    /** Read the values for the attribute class **/
    bool ReadRepeatMarkLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteRepeatMarkLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetFunc(repeatMarkLog_FUNC func_) { m_func = func_; }
    repeatMarkLog_FUNC GetFunc() const { return m_func; }
    bool HasFunc() const;
    ///@}

private:
    /**
     * Indicates the function of the depressed pedal, but not necessarily the text
     * associated with its use.
     * Use the dir element for such text.
     **/
    repeatMarkLog_FUNC m_func;
};

//----------------------------------------------------------------------------
// InstRepeatMarkLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttRepeatMarkLog
 */

class InstRepeatMarkLog : public AttRepeatMarkLog {
public:
    InstRepeatMarkLog() = default;
    virtual ~InstRepeatMarkLog() = default;
};

//----------------------------------------------------------------------------
// AttScalable
//----------------------------------------------------------------------------

class AttScalable : public Att {
protected:
    AttScalable();
    ~AttScalable() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetScalable();

    /** Read the values for the attribute class **/
    bool ReadScalable(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteScalable(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetScale(data_PERCENT scale_) { m_scale = scale_; }
    data_PERCENT GetScale() const { return m_scale; }
    bool HasScale() const;
    ///@}

private:
    /** Scale factor to be applied to the feature to make it the desired display size. **/
    data_PERCENT m_scale;
};

//----------------------------------------------------------------------------
// InstScalable
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttScalable
 */

class InstScalable : public AttScalable {
public:
    InstScalable() = default;
    virtual ~InstScalable() = default;
};

//----------------------------------------------------------------------------
// AttStaffDefLog
//----------------------------------------------------------------------------

class AttStaffDefLog : public Att {
protected:
    AttStaffDefLog();
    ~AttStaffDefLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetStaffDefLog();

    /** Read the values for the attribute class **/
    bool ReadStaffDefLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteStaffDefLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetLines(int lines_) { m_lines = lines_; }
    int GetLines() const { return m_lines; }
    bool HasLines() const;
    ///@}

private:
    /** Indicates the number of staff lines. **/
    int m_lines;
};

//----------------------------------------------------------------------------
// InstStaffDefLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttStaffDefLog
 */

class InstStaffDefLog : public AttStaffDefLog {
public:
    InstStaffDefLog() = default;
    virtual ~InstStaffDefLog() = default;
};

//----------------------------------------------------------------------------
// AttStaffGroupingSym
//----------------------------------------------------------------------------

class AttStaffGroupingSym : public Att {
protected:
    AttStaffGroupingSym();
    ~AttStaffGroupingSym() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetStaffGroupingSym();

    /** Read the values for the attribute class **/
    bool ReadStaffGroupingSym(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteStaffGroupingSym(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetSymbol(staffGroupingSym_SYMBOL symbol_) { m_symbol = symbol_; }
    staffGroupingSym_SYMBOL GetSymbol() const { return m_symbol; }
    bool HasSymbol() const;
    ///@}

private:
    /** Specifies the symbol used to group a set of staves. **/
    staffGroupingSym_SYMBOL m_symbol;
};

//----------------------------------------------------------------------------
// InstStaffGroupingSym
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttStaffGroupingSym
 */

class InstStaffGroupingSym : public AttStaffGroupingSym {
public:
    InstStaffGroupingSym() = default;
    virtual ~InstStaffGroupingSym() = default;
};

//----------------------------------------------------------------------------
// AttStaffIdent
//----------------------------------------------------------------------------

class AttStaffIdent : public Att {
protected:
    AttStaffIdent();
    ~AttStaffIdent() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetStaffIdent();

    /** Read the values for the attribute class **/
    bool ReadStaffIdent(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteStaffIdent(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetStaff(xsdPositiveInteger_List staff_) { m_staff = staff_; }
    xsdPositiveInteger_List GetStaff() const { return m_staff; }
    bool HasStaff() const;
    ///@}

private:
    /**
     * Signifies the staff on which a notated event occurs or to which a control event
     * applies.
     * Mandatory when applicable.
     **/
    xsdPositiveInteger_List m_staff;
};

//----------------------------------------------------------------------------
// InstStaffIdent
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttStaffIdent
 */

class InstStaffIdent : public AttStaffIdent {
public:
    InstStaffIdent() = default;
    virtual ~InstStaffIdent() = default;
};

//----------------------------------------------------------------------------
// AttStaffLocPitched
//----------------------------------------------------------------------------

class AttStaffLocPitched : public Att {
protected:
    AttStaffLocPitched();
    ~AttStaffLocPitched() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetStaffLocPitched();

    /** Read the values for the attribute class **/
    bool ReadStaffLocPitched(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteStaffLocPitched(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetPloc(data_PITCHNAME ploc_) { m_ploc = ploc_; }
    data_PITCHNAME GetPloc() const { return m_ploc; }
    bool HasPloc() const;
    //
    void SetOloc(data_OCTAVE oloc_) { m_oloc = oloc_; }
    data_OCTAVE GetOloc() const { return m_oloc; }
    bool HasOloc() const;
    ///@}

private:
    /** Captures staff location in terms of written pitch name. **/
    data_PITCHNAME m_ploc;
    /** Records staff location in terms of written octave. **/
    data_OCTAVE m_oloc;
};

//----------------------------------------------------------------------------
// InstStaffLocPitched
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttStaffLocPitched
 */

class InstStaffLocPitched : public AttStaffLocPitched {
public:
    InstStaffLocPitched() = default;
    virtual ~InstStaffLocPitched() = default;
};

//----------------------------------------------------------------------------
// AttStartEndId
//----------------------------------------------------------------------------

class AttStartEndId : public Att {
protected:
    AttStartEndId();
    ~AttStartEndId() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetStartEndId();

    /** Read the values for the attribute class **/
    bool ReadStartEndId(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteStartEndId(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetEndid(std::string endid_) { m_endid = endid_; }
    std::string GetEndid() const { return m_endid; }
    bool HasEndid() const;
    ///@}

private:
    /**
     * Indicates the final element in a sequence of events to which the feature
     * applies.
     **/
    std::string m_endid;
};

//----------------------------------------------------------------------------
// InstStartEndId
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttStartEndId
 */

class InstStartEndId : public AttStartEndId {
public:
    InstStartEndId() = default;
    virtual ~InstStartEndId() = default;
};

//----------------------------------------------------------------------------
// AttStartId
//----------------------------------------------------------------------------

class AttStartId : public Att {
protected:
    AttStartId();
    ~AttStartId() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetStartId();

    /** Read the values for the attribute class **/
    bool ReadStartId(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteStartId(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetStartid(std::string startid_) { m_startid = startid_; }
    std::string GetStartid() const { return m_startid; }
    bool HasStartid() const;
    ///@}

private:
    /**
     * Holds a reference to the first element in a sequence of events to which the
     * feature applies.
     **/
    std::string m_startid;
};

//----------------------------------------------------------------------------
// InstStartId
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttStartId
 */

class InstStartId : public AttStartId {
public:
    InstStartId() = default;
    virtual ~InstStartId() = default;
};

//----------------------------------------------------------------------------
// AttStems
//----------------------------------------------------------------------------

class AttStems : public Att {
protected:
    AttStems();
    ~AttStems() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetStems();

    /** Read the values for the attribute class **/
    bool ReadStems(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteStems(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetStemDir(data_STEMDIRECTION stemDir_) { m_stemDir = stemDir_; }
    data_STEMDIRECTION GetStemDir() const { return m_stemDir; }
    bool HasStemDir() const;
    //
    void SetStemLen(double stemLen_) { m_stemLen = stemLen_; }
    double GetStemLen() const { return m_stemLen; }
    bool HasStemLen() const;
    //
    void SetStemMod(data_STEMMODIFIER stemMod_) { m_stemMod = stemMod_; }
    data_STEMMODIFIER GetStemMod() const { return m_stemMod; }
    bool HasStemMod() const;
    ///@}

private:
    /** Describes the direction of a stem. **/
    data_STEMDIRECTION m_stemDir;
    /** Encodes the stem length. **/
    double m_stemLen;
    /**
     * Encodes any stem "modifiers"; that is, symbols rendered on the stem, such as
     * tremolo or Sprechstimme indicators.
     **/
    data_STEMMODIFIER m_stemMod;
};

//----------------------------------------------------------------------------
// InstStems
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttStems
 */

class InstStems : public AttStems {
public:
    InstStems() = default;
    virtual ~InstStems() = default;
};

//----------------------------------------------------------------------------
// AttSylLog
//----------------------------------------------------------------------------

class AttSylLog : public Att {
protected:
    AttSylLog();
    ~AttSylLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetSylLog();

    /** Read the values for the attribute class **/
    bool ReadSylLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteSylLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetCon(sylLog_CON con_) { m_con = con_; }
    sylLog_CON GetCon() const { return m_con; }
    bool HasCon() const;
    //
    void SetWordpos(sylLog_WORDPOS wordpos_) { m_wordpos = wordpos_; }
    sylLog_WORDPOS GetWordpos() const { return m_wordpos; }
    bool HasWordpos() const;
    ///@}

private:
    /**
     * Describes the symbols typically used to indicate breaks between syllables and
     * their functions.
     **/
    sylLog_CON m_con;
    /** Records the position of a syllable within a word. **/
    sylLog_WORDPOS m_wordpos;
};

//----------------------------------------------------------------------------
// InstSylLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttSylLog
 */

class InstSylLog : public AttSylLog {
public:
    InstSylLog() = default;
    virtual ~InstSylLog() = default;
};

//----------------------------------------------------------------------------
// AttTempoLog
//----------------------------------------------------------------------------

class AttTempoLog : public Att {
protected:
    AttTempoLog();
    ~AttTempoLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetTempoLog();

    /** Read the values for the attribute class **/
    bool ReadTempoLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteTempoLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetFunc(tempoLog_FUNC func_) { m_func = func_; }
    tempoLog_FUNC GetFunc() const { return m_func; }
    bool HasFunc() const;
    ///@}

private:
    /**
     * Indicates the function of the depressed pedal, but not necessarily the text
     * associated with its use.
     * Use the dir element for such text.
     **/
    tempoLog_FUNC m_func;
};

//----------------------------------------------------------------------------
// InstTempoLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttTempoLog
 */

class InstTempoLog : public AttTempoLog {
public:
    InstTempoLog() = default;
    virtual ~InstTempoLog() = default;
};

//----------------------------------------------------------------------------
// AttTextRendition
//----------------------------------------------------------------------------

class AttTextRendition : public Att {
protected:
    AttTextRendition();
    ~AttTextRendition() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetTextRendition();

    /** Read the values for the attribute class **/
    bool ReadTextRendition(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteTextRendition(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetAltrend(std::string altrend_) { m_altrend = altrend_; }
    std::string GetAltrend() const { return m_altrend; }
    bool HasAltrend() const;
    //
    void SetRend(data_TEXTRENDITION rend_) { m_rend = rend_; }
    data_TEXTRENDITION GetRend() const { return m_rend; }
    bool HasRend() const;
    ///@}

private:
    /** Used to extend the values of the rend attribute. **/
    std::string m_altrend;
    /** Captures the appearance of the element’s contents using MEI-defined descriptors. **/
    data_TEXTRENDITION m_rend;
};

//----------------------------------------------------------------------------
// InstTextRendition
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttTextRendition
 */

class InstTextRendition : public AttTextRendition {
public:
    InstTextRendition() = default;
    virtual ~InstTextRendition() = default;
};

//----------------------------------------------------------------------------
// AttTimestampLog
//----------------------------------------------------------------------------

class AttTimestampLog : public Att {
protected:
    AttTimestampLog();
    ~AttTimestampLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetTimestampLog();

    /** Read the values for the attribute class **/
    bool ReadTimestampLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteTimestampLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetTstamp(double tstamp_) { m_tstamp = tstamp_; }
    double GetTstamp() const { return m_tstamp; }
    bool HasTstamp() const;
    ///@}

private:
    /**
     * Encodes the onset time in terms of musical time, i.e., beats[.fractional beat
     * part], as expressed in the written time signature.
     **/
    double m_tstamp;
};

//----------------------------------------------------------------------------
// InstTimestampLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttTimestampLog
 */

class InstTimestampLog : public AttTimestampLog {
public:
    InstTimestampLog() = default;
    virtual ~InstTimestampLog() = default;
};

//----------------------------------------------------------------------------
// AttTimestamp2Log
//----------------------------------------------------------------------------

class AttTimestamp2Log : public Att {
protected:
    AttTimestamp2Log();
    ~AttTimestamp2Log() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetTimestamp2Log();

    /** Read the values for the attribute class **/
    bool ReadTimestamp2Log(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteTimestamp2Log(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetTstamp2(data_MEASUREBEAT tstamp2_) { m_tstamp2 = tstamp2_; }
    data_MEASUREBEAT GetTstamp2() const { return m_tstamp2; }
    bool HasTstamp2() const;
    ///@}

private:
    /**
     * Encodes the ending point of an event, i.e., a count of measures plus a beat
     * location in the ending measure.
     **/
    data_MEASUREBEAT m_tstamp2;
};

//----------------------------------------------------------------------------
// InstTimestamp2Log
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttTimestamp2Log
 */

class InstTimestamp2Log : public AttTimestamp2Log {
public:
    InstTimestamp2Log() = default;
    virtual ~InstTimestamp2Log() = default;
};

//----------------------------------------------------------------------------
// AttTransposition
//----------------------------------------------------------------------------

class AttTransposition : public Att {
protected:
    AttTransposition();
    ~AttTransposition() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetTransposition();

    /** Read the values for the attribute class **/
    bool ReadTransposition(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteTransposition(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetTransDiat(int transDiat_) { m_transDiat = transDiat_; }
    int GetTransDiat() const { return m_transDiat; }
    bool HasTransDiat() const;
    //
    void SetTransSemi(int transSemi_) { m_transSemi = transSemi_; }
    int GetTransSemi() const { return m_transSemi; }
    bool HasTransSemi() const;
    ///@}

private:
    /**
     * Records the amount of diatonic pitch shift, e.g., C to C♯ = 0, C to D♭ = 1,
     * necessary to calculate the sounded pitch from the written one.
     **/
    int m_transDiat;
    /**
     * Records the amount of pitch shift in semitones, e.g., C to C♯ = 1, C to D♭ = 1,
     * necessary to calculate the sounded pitch from the written one.
     **/
    int m_transSemi;
};

//----------------------------------------------------------------------------
// InstTransposition
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttTransposition
 */

class InstTransposition : public AttTransposition {
public:
    InstTransposition() = default;
    virtual ~InstTransposition() = default;
};

//----------------------------------------------------------------------------
// AttTuning
//----------------------------------------------------------------------------

class AttTuning : public Att {
protected:
    AttTuning();
    ~AttTuning() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetTuning();

    /** Read the values for the attribute class **/
    bool ReadTuning(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteTuning(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetTuneHz(double tuneHz_) { m_tuneHz = tuneHz_; }
    double GetTuneHz() const { return m_tuneHz; }
    bool HasTuneHz() const;
    //
    void SetTunePname(data_PITCHNAME tunePname_) { m_tunePname = tunePname_; }
    data_PITCHNAME GetTunePname() const { return m_tunePname; }
    bool HasTunePname() const;
    //
    void SetTuneTemper(data_TEMPERAMENT tuneTemper_) { m_tuneTemper = tuneTemper_; }
    data_TEMPERAMENT GetTuneTemper() const { return m_tuneTemper; }
    bool HasTuneTemper() const;
    ///@}

private:
    /** Holds a value for cycles per second, i.e., Hertz, for a tuning reference pitch. **/
    double m_tuneHz;
    /**
     * Holds the pitch name of a tuning reference pitch, i.e., the central tone of a
     * tuning system.
     **/
    data_PITCHNAME m_tunePname;
    /** Provides an indication of the tuning system, just, for example. **/
    data_TEMPERAMENT m_tuneTemper;
};

//----------------------------------------------------------------------------
// InstTuning
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttTuning
 */

class InstTuning : public AttTuning {
public:
    InstTuning() = default;
    virtual ~InstTuning() = default;
};

//----------------------------------------------------------------------------
// AttTyped
//----------------------------------------------------------------------------

class AttTyped : public Att {
protected:
    AttTyped();
    ~AttTyped() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetTyped();

    /** Read the values for the attribute class **/
    bool ReadTyped(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteTyped(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetType(std::string type_) { m_type = type_; }
    std::string GetType() const { return m_type; }
    bool HasType() const;
    ///@}

private:
    /**
     * Specifies the kind of document to which the header is attached, for example
     * whether it is a corpus or individual text.
     **/
    std::string m_type;
};

//----------------------------------------------------------------------------
// InstTyped
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttTyped
 */

class InstTyped : public AttTyped {
public:
    InstTyped() = default;
    virtual ~InstTyped() = default;
};

//----------------------------------------------------------------------------
// AttTypography
//----------------------------------------------------------------------------

class AttTypography : public Att {
protected:
    AttTypography();
    ~AttTypography() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetTypography();

    /** Read the values for the attribute class **/
    bool ReadTypography(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteTypography(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetFontfam(std::string fontfam_) { m_fontfam = fontfam_; }
    std::string GetFontfam() const { return m_fontfam; }
    bool HasFontfam() const;
    //
    void SetFontname(std::string fontname_) { m_fontname = fontname_; }
    std::string GetFontname() const { return m_fontname; }
    bool HasFontname() const;
    //
    void SetFontsize(data_FONTSIZE fontsize_) { m_fontsize = fontsize_; }
    data_FONTSIZE GetFontsize() const { return m_fontsize; }
    bool HasFontsize() const;
    /** Getter for reference (for alternate type only) */
    data_FONTSIZE *GetFontsizeAlternate() { return &m_fontsize; }
    //
    void SetFontstyle(data_FONTSTYLE fontstyle_) { m_fontstyle = fontstyle_; }
    data_FONTSTYLE GetFontstyle() const { return m_fontstyle; }
    bool HasFontstyle() const;
    //
    void SetFontweight(data_FONTWEIGHT fontweight_) { m_fontweight = fontweight_; }
    data_FONTWEIGHT GetFontweight() const { return m_fontweight; }
    bool HasFontweight() const;
    //
    void SetLetterspacing(double letterspacing_) { m_letterspacing = letterspacing_; }
    double GetLetterspacing() const { return m_letterspacing; }
    bool HasLetterspacing() const;
    //
    void SetLineheight(std::string lineheight_) { m_lineheight = lineheight_; }
    std::string GetLineheight() const { return m_lineheight; }
    bool HasLineheight() const;
    ///@}

private:
    /** Contains the name of a font-family. **/
    std::string m_fontfam;
    /** Holds the name of a font. **/
    std::string m_fontname;
    /**
     * Indicates the size of a font expressed in printers' points, i.e., 1/72nd of an
     * inch, relative terms, e.g., small, larger, etc., or percentage values relative
     * to normal size, e.g., 125%.
     **/
    data_FONTSIZE m_fontsize;
    /** Records the style of a font, i.e., italic, oblique, or normal. **/
    data_FONTSTYLE m_fontstyle;
    /** Used to indicate bold type. **/
    data_FONTWEIGHT m_fontweight;
    /**
     * Indicates letter spacing (aka tracking) in analogy to the CSS letter-spacing
     * property.
     **/
    double m_letterspacing;
    /** Indicates line height in analogy to the CSS line-height property. **/
    std::string m_lineheight;
};

//----------------------------------------------------------------------------
// InstTypography
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttTypography
 */

class InstTypography : public AttTypography {
public:
    InstTypography() = default;
    virtual ~InstTypography() = default;
};

//----------------------------------------------------------------------------
// AttVerticalAlign
//----------------------------------------------------------------------------

class AttVerticalAlign : public Att {
protected:
    AttVerticalAlign();
    ~AttVerticalAlign() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetVerticalAlign();

    /** Read the values for the attribute class **/
    bool ReadVerticalAlign(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteVerticalAlign(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetValign(data_VERTICALALIGNMENT valign_) { m_valign = valign_; }
    data_VERTICALALIGNMENT GetValign() const { return m_valign; }
    bool HasValign() const;
    ///@}

private:
    /** Records vertical alignment. **/
    data_VERTICALALIGNMENT m_valign;
};

//----------------------------------------------------------------------------
// InstVerticalAlign
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttVerticalAlign
 */

class InstVerticalAlign : public AttVerticalAlign {
public:
    InstVerticalAlign() = default;
    virtual ~InstVerticalAlign() = default;
};

//----------------------------------------------------------------------------
// AttVisualOffsetHo
//----------------------------------------------------------------------------

class AttVisualOffsetHo : public Att {
protected:
    AttVisualOffsetHo();
    ~AttVisualOffsetHo() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetVisualOffsetHo();

    /** Read the values for the attribute class **/
    bool ReadVisualOffsetHo(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteVisualOffsetHo(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetHo(data_MEASUREMENTSIGNED ho_) { m_ho = ho_; }
    data_MEASUREMENTSIGNED GetHo() const { return m_ho; }
    bool HasHo() const;
    ///@}

private:
    /**
     * Records a horizontal adjustment to a feature’s programmatically-determined
     * location in terms of staff interline distance; that is, in units of 1/2 the
     * distance between adjacent staff lines.
     **/
    data_MEASUREMENTSIGNED m_ho;
};

//----------------------------------------------------------------------------
// InstVisualOffsetHo
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttVisualOffsetHo
 */

class InstVisualOffsetHo : public AttVisualOffsetHo {
public:
    InstVisualOffsetHo() = default;
    virtual ~InstVisualOffsetHo() = default;
};

//----------------------------------------------------------------------------
// AttVisualOffsetVo
//----------------------------------------------------------------------------

class AttVisualOffsetVo : public Att {
protected:
    AttVisualOffsetVo();
    ~AttVisualOffsetVo() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetVisualOffsetVo();

    /** Read the values for the attribute class **/
    bool ReadVisualOffsetVo(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteVisualOffsetVo(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetVo(data_MEASUREMENTSIGNED vo_) { m_vo = vo_; }
    data_MEASUREMENTSIGNED GetVo() const { return m_vo; }
    bool HasVo() const;
    ///@}

private:
    /**
     * Records the vertical adjustment of a feature’s programmatically-determined
     * location in terms of staff interline distance; that is, in units of 1/2 the
     * distance between adjacent staff lines.
     **/
    data_MEASUREMENTSIGNED m_vo;
};

//----------------------------------------------------------------------------
// InstVisualOffsetVo
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttVisualOffsetVo
 */

class InstVisualOffsetVo : public AttVisualOffsetVo {
public:
    InstVisualOffsetVo() = default;
    virtual ~InstVisualOffsetVo() = default;
};

//----------------------------------------------------------------------------
// AttVisualOffset2Ho
//----------------------------------------------------------------------------

class AttVisualOffset2Ho : public Att {
protected:
    AttVisualOffset2Ho();
    ~AttVisualOffset2Ho() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetVisualOffset2Ho();

    /** Read the values for the attribute class **/
    bool ReadVisualOffset2Ho(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteVisualOffset2Ho(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetStartho(data_MEASUREMENTSIGNED startho_) { m_startho = startho_; }
    data_MEASUREMENTSIGNED GetStartho() const { return m_startho; }
    bool HasStartho() const;
    //
    void SetEndho(data_MEASUREMENTSIGNED endho_) { m_endho = endho_; }
    data_MEASUREMENTSIGNED GetEndho() const { return m_endho; }
    bool HasEndho() const;
    ///@}

private:
    /**
     * Records the horizontal adjustment of a feature’s programmatically-determined
     * start point.
     **/
    data_MEASUREMENTSIGNED m_startho;
    /**
     * Records the horizontal adjustment of a feature’s programmatically-determined end
     * point.
     **/
    data_MEASUREMENTSIGNED m_endho;
};

//----------------------------------------------------------------------------
// InstVisualOffset2Ho
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttVisualOffset2Ho
 */

class InstVisualOffset2Ho : public AttVisualOffset2Ho {
public:
    InstVisualOffset2Ho() = default;
    virtual ~InstVisualOffset2Ho() = default;
};

//----------------------------------------------------------------------------
// AttVisualOffset2Vo
//----------------------------------------------------------------------------

class AttVisualOffset2Vo : public Att {
protected:
    AttVisualOffset2Vo();
    ~AttVisualOffset2Vo() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetVisualOffset2Vo();

    /** Read the values for the attribute class **/
    bool ReadVisualOffset2Vo(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteVisualOffset2Vo(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetStartvo(data_MEASUREMENTSIGNED startvo_) { m_startvo = startvo_; }
    data_MEASUREMENTSIGNED GetStartvo() const { return m_startvo; }
    bool HasStartvo() const;
    //
    void SetEndvo(data_MEASUREMENTSIGNED endvo_) { m_endvo = endvo_; }
    data_MEASUREMENTSIGNED GetEndvo() const { return m_endvo; }
    bool HasEndvo() const;
    ///@}

private:
    /**
     * Records a vertical adjustment of a feature’s programmatically-determined start
     * point.
     **/
    data_MEASUREMENTSIGNED m_startvo;
    /**
     * Records a vertical adjustment of a feature’s programmatically-determined end
     * point.
     **/
    data_MEASUREMENTSIGNED m_endvo;
};

//----------------------------------------------------------------------------
// InstVisualOffset2Vo
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttVisualOffset2Vo
 */

class InstVisualOffset2Vo : public AttVisualOffset2Vo {
public:
    InstVisualOffset2Vo() = default;
    virtual ~InstVisualOffset2Vo() = default;
};

//----------------------------------------------------------------------------
// AttVoltaGroupingSym
//----------------------------------------------------------------------------

class AttVoltaGroupingSym : public Att {
protected:
    AttVoltaGroupingSym();
    ~AttVoltaGroupingSym() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetVoltaGroupingSym();

    /** Read the values for the attribute class **/
    bool ReadVoltaGroupingSym(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteVoltaGroupingSym(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetVoltasym(voltaGroupingSym_VOLTASYM voltasym_) { m_voltasym = voltasym_; }
    voltaGroupingSym_VOLTASYM GetVoltasym() const { return m_voltasym; }
    bool HasVoltasym() const;
    ///@}

private:
    /** Specifies the symbol used to group lyrics. **/
    voltaGroupingSym_VOLTASYM m_voltasym;
};

//----------------------------------------------------------------------------
// InstVoltaGroupingSym
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttVoltaGroupingSym
 */

class InstVoltaGroupingSym : public AttVoltaGroupingSym {
public:
    InstVoltaGroupingSym() = default;
    virtual ~InstVoltaGroupingSym() = default;
};

//----------------------------------------------------------------------------
// AttWhitespace
//----------------------------------------------------------------------------

class AttWhitespace : public Att {
protected:
    AttWhitespace();
    ~AttWhitespace() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetWhitespace();

    /** Read the values for the attribute class **/
    bool ReadWhitespace(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteWhitespace(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetSpace(std::string space_) { m_space = space_; }
    std::string GetSpace() const { return m_space; }
    bool HasSpace() const;
    ///@}

private:
    /** --- **/
    std::string m_space;
};

//----------------------------------------------------------------------------
// InstWhitespace
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttWhitespace
 */

class InstWhitespace : public AttWhitespace {
public:
    InstWhitespace() = default;
    virtual ~InstWhitespace() = default;
};

//----------------------------------------------------------------------------
// AttWidth
//----------------------------------------------------------------------------

class AttWidth : public Att {
protected:
    AttWidth();
    ~AttWidth() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetWidth();

    /** Read the values for the attribute class **/
    bool ReadWidth(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteWidth(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetWidth(data_MEASUREMENTUNSIGNED width_) { m_width = width_; }
    data_MEASUREMENTUNSIGNED GetWidth() const { return m_width; }
    bool HasWidth() const;
    ///@}

private:
    /** Measurement of the horizontal dimension of an entity. **/
    data_MEASUREMENTUNSIGNED m_width;
};

//----------------------------------------------------------------------------
// InstWidth
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttWidth
 */

class InstWidth : public AttWidth {
public:
    InstWidth() = default;
    virtual ~InstWidth() = default;
};

} // namespace libmei

#endif // __LIBMEI_ATTS_SHARED_H__
