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

#ifndef __LIBMEI_ATTS_CMN_H__
#define __LIBMEI_ATTS_CMN_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include <string>

namespace libmei {

//----------------------------------------------------------------------------
// AttArpegLog
//----------------------------------------------------------------------------

class AttArpegLog : public Att {
protected:
    AttArpegLog();
    ~AttArpegLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetArpegLog();

    /** Read the values for the attribute class **/
    bool ReadArpegLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteArpegLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetOrder(arpegLog_ORDER order_) { m_order = order_; }
    arpegLog_ORDER GetOrder() const { return m_order; }
    bool HasOrder() const;
    ///@}

private:
    /** Describes the direction in which an arpeggio is to be performed. **/
    arpegLog_ORDER m_order;
};

//----------------------------------------------------------------------------
// InstArpegLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttArpegLog
 */

class InstArpegLog : public AttArpegLog {
public:
    InstArpegLog() = default;
    virtual ~InstArpegLog() = default;
};

//----------------------------------------------------------------------------
// AttBeamSecondary
//----------------------------------------------------------------------------

class AttBeamSecondary : public Att {
protected:
    AttBeamSecondary();
    ~AttBeamSecondary() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetBeamSecondary();

    /** Read the values for the attribute class **/
    bool ReadBeamSecondary(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteBeamSecondary(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetBreaksec(int breaksec_) { m_breaksec = breaksec_; }
    int GetBreaksec() const { return m_breaksec; }
    bool HasBreaksec() const;
    ///@}

private:
    /**
     * Presence of this attribute indicates that the secondary beam should be broken
     * following this note/chord.
     * The value of the attribute records the number of beams which should remain
     * unbroken.
     **/
    int m_breaksec;
};

//----------------------------------------------------------------------------
// InstBeamSecondary
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttBeamSecondary
 */

class InstBeamSecondary : public AttBeamSecondary {
public:
    InstBeamSecondary() = default;
    virtual ~InstBeamSecondary() = default;
};

//----------------------------------------------------------------------------
// AttBeatRptLog
//----------------------------------------------------------------------------

class AttBeatRptLog : public Att {
protected:
    AttBeatRptLog();
    ~AttBeatRptLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetBeatRptLog();

    /** Read the values for the attribute class **/
    bool ReadBeatRptLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteBeatRptLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetBeatdef(double beatdef_) { m_beatdef = beatdef_; }
    double GetBeatdef() const { return m_beatdef; }
    bool HasBeatdef() const;
    ///@}

private:
    /**
     * Indicates the performed duration represented by the beatRpt symbol; expressed in
     * time signature denominator units.
     **/
    double m_beatdef;
};

//----------------------------------------------------------------------------
// InstBeatRptLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttBeatRptLog
 */

class InstBeatRptLog : public AttBeatRptLog {
public:
    InstBeatRptLog() = default;
    virtual ~InstBeatRptLog() = default;
};

//----------------------------------------------------------------------------
// AttCutout
//----------------------------------------------------------------------------

class AttCutout : public Att {
protected:
    AttCutout();
    ~AttCutout() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetCutout();

    /** Read the values for the attribute class **/
    bool ReadCutout(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteCutout(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetCutout(cutout_CUTOUT cutout_) { m_cutout = cutout_; }
    cutout_CUTOUT GetCutout() const { return m_cutout; }
    bool HasCutout() const;
    ///@}

private:
    /** "Cut-out" style. **/
    cutout_CUTOUT m_cutout;
};

//----------------------------------------------------------------------------
// InstCutout
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttCutout
 */

class InstCutout : public AttCutout {
public:
    InstCutout() = default;
    virtual ~InstCutout() = default;
};

//----------------------------------------------------------------------------
// AttExpandable
//----------------------------------------------------------------------------

class AttExpandable : public Att {
protected:
    AttExpandable();
    ~AttExpandable() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetExpandable();

    /** Read the values for the attribute class **/
    bool ReadExpandable(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteExpandable(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetExpand(data_BOOLEAN expand_) { m_expand = expand_; }
    data_BOOLEAN GetExpand() const { return m_expand; }
    bool HasExpand() const;
    ///@}

private:
    /**
     * Indicates whether to render a repeat symbol or the source material to which it
     * refers.
     * A value of 'true' renders the source material, while 'false' displays the repeat
     * symbol.
     **/
    data_BOOLEAN m_expand;
};

//----------------------------------------------------------------------------
// InstExpandable
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttExpandable
 */

class InstExpandable : public AttExpandable {
public:
    InstExpandable() = default;
    virtual ~InstExpandable() = default;
};

//----------------------------------------------------------------------------
// AttGraceGrpLog
//----------------------------------------------------------------------------

class AttGraceGrpLog : public Att {
protected:
    AttGraceGrpLog();
    ~AttGraceGrpLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetGraceGrpLog();

    /** Read the values for the attribute class **/
    bool ReadGraceGrpLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteGraceGrpLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetAttach(graceGrpLog_ATTACH attach_) { m_attach = attach_; }
    graceGrpLog_ATTACH GetAttach() const { return m_attach; }
    bool HasAttach() const;
    ///@}

private:
    /**
     * Records whether the grace note group is attached to the following event or to
     * the preceding one.
     * The usual name for the latter is "Nachschlag".
     **/
    graceGrpLog_ATTACH m_attach;
};

//----------------------------------------------------------------------------
// InstGraceGrpLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttGraceGrpLog
 */

class InstGraceGrpLog : public AttGraceGrpLog {
public:
    InstGraceGrpLog() = default;
    virtual ~InstGraceGrpLog() = default;
};

//----------------------------------------------------------------------------
// AttGraced
//----------------------------------------------------------------------------

class AttGraced : public Att {
protected:
    AttGraced();
    ~AttGraced() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetGraced();

    /** Read the values for the attribute class **/
    bool ReadGraced(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteGraced(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetGrace(data_GRACE grace_) { m_grace = grace_; }
    data_GRACE GetGrace() const { return m_grace; }
    bool HasGrace() const;
    //
    void SetGraceTime(data_PERCENT graceTime_) { m_graceTime = graceTime_; }
    data_PERCENT GetGraceTime() const { return m_graceTime; }
    bool HasGraceTime() const;
    ///@}

private:
    /**
     * Marks a note or chord as a "grace" (without a definite performed duration) and
     * records from which other note/chord it should "steal" time.
     **/
    data_GRACE m_grace;
    /** Records the amount of time to be "stolen" from a non-grace note/chord. **/
    data_PERCENT m_graceTime;
};

//----------------------------------------------------------------------------
// InstGraced
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttGraced
 */

class InstGraced : public AttGraced {
public:
    InstGraced() = default;
    virtual ~InstGraced() = default;
};

//----------------------------------------------------------------------------
// AttHairpinLog
//----------------------------------------------------------------------------

class AttHairpinLog : public Att {
protected:
    AttHairpinLog();
    ~AttHairpinLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetHairpinLog();

    /** Read the values for the attribute class **/
    bool ReadHairpinLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteHairpinLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetForm(hairpinLog_FORM form_) { m_form = form_; }
    hairpinLog_FORM GetForm() const { return m_form; }
    bool HasForm() const;
    //
    void SetNiente(data_BOOLEAN niente_) { m_niente = niente_; }
    data_BOOLEAN GetNiente() const { return m_niente; }
    bool HasNiente() const;
    ///@}

private:
    /**
     * Captures the visual rendition and function of the hairpin; that is, whether it
     * indicates an increase or a decrease in volume.
     **/
    hairpinLog_FORM m_form;
    /**
     * Indicates that the hairpin starts from or ends in silence.
     * Often rendered as a small circle attached to the closed end of the hairpin. See
     * Gould, p. 108.
     **/
    data_BOOLEAN m_niente;
};

//----------------------------------------------------------------------------
// InstHairpinLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttHairpinLog
 */

class InstHairpinLog : public AttHairpinLog {
public:
    InstHairpinLog() = default;
    virtual ~InstHairpinLog() = default;
};

//----------------------------------------------------------------------------
// AttHarpPedalLog
//----------------------------------------------------------------------------

class AttHarpPedalLog : public Att {
protected:
    AttHarpPedalLog();
    ~AttHarpPedalLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetHarpPedalLog();

    /** Read the values for the attribute class **/
    bool ReadHarpPedalLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteHarpPedalLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetC(data_HARPPEDALPOSITION c_) { m_c = c_; }
    data_HARPPEDALPOSITION GetC() const { return m_c; }
    bool HasC() const;
    //
    void SetD(data_HARPPEDALPOSITION d_) { m_d = d_; }
    data_HARPPEDALPOSITION GetD() const { return m_d; }
    bool HasD() const;
    //
    void SetE(data_HARPPEDALPOSITION e_) { m_e = e_; }
    data_HARPPEDALPOSITION GetE() const { return m_e; }
    bool HasE() const;
    //
    void SetF(data_HARPPEDALPOSITION f_) { m_f = f_; }
    data_HARPPEDALPOSITION GetF() const { return m_f; }
    bool HasF() const;
    //
    void SetG(data_HARPPEDALPOSITION g_) { m_g = g_; }
    data_HARPPEDALPOSITION GetG() const { return m_g; }
    bool HasG() const;
    //
    void SetA(data_HARPPEDALPOSITION a_) { m_a = a_; }
    data_HARPPEDALPOSITION GetA() const { return m_a; }
    bool HasA() const;
    //
    void SetB(data_HARPPEDALPOSITION b_) { m_b = b_; }
    data_HARPPEDALPOSITION GetB() const { return m_b; }
    bool HasB() const;
    ///@}

private:
    /** Indicates the pedal setting for the harp’s C strings. **/
    data_HARPPEDALPOSITION m_c;
    /** Indicates the pedal setting for the harp’s D strings. **/
    data_HARPPEDALPOSITION m_d;
    /** Indicates the pedal setting for the harp’s E strings. **/
    data_HARPPEDALPOSITION m_e;
    /** Indicates the pedal setting for the harp’s F strings. **/
    data_HARPPEDALPOSITION m_f;
    /** Indicates the pedal setting for the harp’s G strings. **/
    data_HARPPEDALPOSITION m_g;
    /** Indicates the pedal setting for the harp’s A strings. **/
    data_HARPPEDALPOSITION m_a;
    /** Indicates the pedal setting for the harp’s B strings. **/
    data_HARPPEDALPOSITION m_b;
};

//----------------------------------------------------------------------------
// InstHarpPedalLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttHarpPedalLog
 */

class InstHarpPedalLog : public AttHarpPedalLog {
public:
    InstHarpPedalLog() = default;
    virtual ~InstHarpPedalLog() = default;
};

//----------------------------------------------------------------------------
// AttMeasureLog
//----------------------------------------------------------------------------

class AttMeasureLog : public Att {
protected:
    AttMeasureLog();
    ~AttMeasureLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetMeasureLog();

    /** Read the values for the attribute class **/
    bool ReadMeasureLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteMeasureLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetLeft(data_BARRENDITION left_) { m_left = left_; }
    data_BARRENDITION GetLeft() const { return m_left; }
    bool HasLeft() const;
    //
    void SetRight(data_BARRENDITION right_) { m_right = right_; }
    data_BARRENDITION GetRight() const { return m_right; }
    bool HasRight() const;
    ///@}

private:
    /**
     * Indicates the visual rendition of the left bar line.
     * It is present here only for facilitation of translation from legacy encodings
     * which use it. Usually, it can be safely ignored.
     **/
    data_BARRENDITION m_left;
    /** Indicates the function of the right bar line and is structurally important. **/
    data_BARRENDITION m_right;
};

//----------------------------------------------------------------------------
// InstMeasureLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttMeasureLog
 */

class InstMeasureLog : public AttMeasureLog {
public:
    InstMeasureLog() = default;
    virtual ~InstMeasureLog() = default;
};

//----------------------------------------------------------------------------
// AttNumberPlacement
//----------------------------------------------------------------------------

class AttNumberPlacement : public Att {
protected:
    AttNumberPlacement();
    ~AttNumberPlacement() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetNumberPlacement();

    /** Read the values for the attribute class **/
    bool ReadNumberPlacement(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteNumberPlacement(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetNumPlace(data_STAFFREL_basic numPlace_) { m_numPlace = numPlace_; }
    data_STAFFREL_basic GetNumPlace() const { return m_numPlace; }
    bool HasNumPlace() const;
    //
    void SetNumVisible(data_BOOLEAN numVisible_) { m_numVisible = numVisible_; }
    data_BOOLEAN GetNumVisible() const { return m_numVisible; }
    bool HasNumVisible() const;
    ///@}

private:
    /** States where the tuplet number will be placed in relation to the note heads. **/
    data_STAFFREL_basic m_numPlace;
    /** Determines if the tuplet number is visible. **/
    data_BOOLEAN m_numVisible;
};

//----------------------------------------------------------------------------
// InstNumberPlacement
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttNumberPlacement
 */

class InstNumberPlacement : public AttNumberPlacement {
public:
    InstNumberPlacement() = default;
    virtual ~InstNumberPlacement() = default;
};

//----------------------------------------------------------------------------
// AttNumbered
//----------------------------------------------------------------------------

class AttNumbered : public Att {
protected:
    AttNumbered();
    ~AttNumbered() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetNumbered();

    /** Read the values for the attribute class **/
    bool ReadNumbered(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteNumbered(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetNum(int num_) { m_num = num_; }
    int GetNum() const { return m_num; }
    bool HasNum() const;
    ///@}

private:
    /** Records a number or count accompanying a notational feature. **/
    int m_num;
};

//----------------------------------------------------------------------------
// InstNumbered
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttNumbered
 */

class InstNumbered : public AttNumbered {
public:
    InstNumbered() = default;
    virtual ~InstNumbered() = default;
};

//----------------------------------------------------------------------------
// AttOctaveLog
//----------------------------------------------------------------------------

class AttOctaveLog : public Att {
protected:
    AttOctaveLog();
    ~AttOctaveLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetOctaveLog();

    /** Read the values for the attribute class **/
    bool ReadOctaveLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteOctaveLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetColl(octaveLog_COLL coll_) { m_coll = coll_; }
    octaveLog_COLL GetColl() const { return m_coll; }
    bool HasColl() const;
    ///@}

private:
    /**
     * Indicates whether the octave displacement should be performed simultaneously
     * with the written notes, i.e., "coll' ottava".
     * Unlike other octave signs which are indicated by broken lines, coll' ottava
     * typically uses an unbroken line or a series of longer broken lines, ending with
     * a short vertical stroke. See Read, p. 47-48.
     **/
    octaveLog_COLL m_coll;
};

//----------------------------------------------------------------------------
// InstOctaveLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttOctaveLog
 */

class InstOctaveLog : public AttOctaveLog {
public:
    InstOctaveLog() = default;
    virtual ~InstOctaveLog() = default;
};

//----------------------------------------------------------------------------
// AttPedalLog
//----------------------------------------------------------------------------

class AttPedalLog : public Att {
protected:
    AttPedalLog();
    ~AttPedalLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetPedalLog();

    /** Read the values for the attribute class **/
    bool ReadPedalLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WritePedalLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetDir(pedalLog_DIR dir_) { m_dir = dir_; }
    pedalLog_DIR GetDir() const { return m_dir; }
    bool HasDir() const;
    //
    void SetFunc(std::string func_) { m_func = func_; }
    std::string GetFunc() const { return m_func; }
    bool HasFunc() const;
    ///@}

private:
    /** Records the position of the piano damper pedal. **/
    pedalLog_DIR m_dir;
    /**
     * Indicates the function of the depressed pedal, but not necessarily the text
     * associated with its use.
     * Use the dir element for such text.
     **/
    std::string m_func;
};

//----------------------------------------------------------------------------
// InstPedalLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttPedalLog
 */

class InstPedalLog : public AttPedalLog {
public:
    InstPedalLog() = default;
    virtual ~InstPedalLog() = default;
};

//----------------------------------------------------------------------------
// AttTremForm
//----------------------------------------------------------------------------

class AttTremForm : public Att {
protected:
    AttTremForm();
    ~AttTremForm() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetTremForm();

    /** Read the values for the attribute class **/
    bool ReadTremForm(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteTremForm(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetForm(tremForm_FORM form_) { m_form = form_; }
    tremForm_FORM GetForm() const { return m_form; }
    bool HasForm() const;
    ///@}

private:
    /**
     * Captures the visual rendition and function of the hairpin; that is, whether it
     * indicates an increase or a decrease in volume.
     **/
    tremForm_FORM m_form;
};

//----------------------------------------------------------------------------
// InstTremForm
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttTremForm
 */

class InstTremForm : public AttTremForm {
public:
    InstTremForm() = default;
    virtual ~InstTremForm() = default;
};

//----------------------------------------------------------------------------
// AttTremMeasured
//----------------------------------------------------------------------------

class AttTremMeasured : public Att {
protected:
    AttTremMeasured();
    ~AttTremMeasured() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetTremMeasured();

    /** Read the values for the attribute class **/
    bool ReadTremMeasured(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteTremMeasured(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetUnitdur(data_DURATION unitdur_) { m_unitdur = unitdur_; }
    data_DURATION GetUnitdur() const { return m_unitdur; }
    bool HasUnitdur() const;
    ///@}

private:
    /** The performed duration of an individual note in a measured tremolo. **/
    data_DURATION m_unitdur;
};

//----------------------------------------------------------------------------
// InstTremMeasured
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttTremMeasured
 */

class InstTremMeasured : public AttTremMeasured {
public:
    InstTremMeasured() = default;
    virtual ~InstTremMeasured() = default;
};

} // namespace libmei

#endif // __LIBMEI_ATTS_CMN_H__
