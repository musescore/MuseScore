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

#ifndef __LIBMEI_ATTS_CMNORNAMENTS_H__
#define __LIBMEI_ATTS_CMNORNAMENTS_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include <string>

namespace libmei {

//----------------------------------------------------------------------------
// AttMordentLog
//----------------------------------------------------------------------------

class AttMordentLog : public Att {
protected:
    AttMordentLog();
    ~AttMordentLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetMordentLog();

    /** Read the values for the attribute class **/
    bool ReadMordentLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteMordentLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetForm(mordentLog_FORM form_) { m_form = form_; }
    mordentLog_FORM GetForm() const { return m_form; }
    bool HasForm() const;
    //
    void SetLong(data_BOOLEAN long_) { m_long = long_; }
    data_BOOLEAN GetLong() const { return m_long; }
    bool HasLong() const;
    ///@}

private:
    /**
     * Captures the visual rendition and function of the hairpin; that is, whether it
     * indicates an increase or a decrease in volume.
     **/
    mordentLog_FORM m_form;
    /**
     * When set to 'true', a double or long mordent, sometimes called a "pincé double",
     * consisting of 5 notes, is indicated.
     **/
    data_BOOLEAN m_long;
};

//----------------------------------------------------------------------------
// InstMordentLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttMordentLog
 */

class InstMordentLog : public AttMordentLog {
public:
    InstMordentLog() = default;
    virtual ~InstMordentLog() = default;
};

//----------------------------------------------------------------------------
// AttOrnamentAccid
//----------------------------------------------------------------------------

class AttOrnamentAccid : public Att {
protected:
    AttOrnamentAccid();
    ~AttOrnamentAccid() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetOrnamentAccid();

    /** Read the values for the attribute class **/
    bool ReadOrnamentAccid(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteOrnamentAccid(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetAccidupper(data_ACCIDENTAL_WRITTEN accidupper_) { m_accidupper = accidupper_; }
    data_ACCIDENTAL_WRITTEN GetAccidupper() const { return m_accidupper; }
    bool HasAccidupper() const;
    //
    void SetAccidlower(data_ACCIDENTAL_WRITTEN accidlower_) { m_accidlower = accidlower_; }
    data_ACCIDENTAL_WRITTEN GetAccidlower() const { return m_accidlower; }
    bool HasAccidlower() const;
    ///@}

private:
    /** Records the written accidental associated with an upper neighboring note. **/
    data_ACCIDENTAL_WRITTEN m_accidupper;
    /** Records the written accidental associated with a lower neighboring note. **/
    data_ACCIDENTAL_WRITTEN m_accidlower;
};

//----------------------------------------------------------------------------
// InstOrnamentAccid
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttOrnamentAccid
 */

class InstOrnamentAccid : public AttOrnamentAccid {
public:
    InstOrnamentAccid() = default;
    virtual ~InstOrnamentAccid() = default;
};

//----------------------------------------------------------------------------
// AttTurnLog
//----------------------------------------------------------------------------

class AttTurnLog : public Att {
protected:
    AttTurnLog();
    ~AttTurnLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetTurnLog();

    /** Read the values for the attribute class **/
    bool ReadTurnLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteTurnLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetDelayed(data_BOOLEAN delayed_) { m_delayed = delayed_; }
    data_BOOLEAN GetDelayed() const { return m_delayed; }
    bool HasDelayed() const;
    //
    void SetForm(turnLog_FORM form_) { m_form = form_; }
    turnLog_FORM GetForm() const { return m_form; }
    bool HasForm() const;
    ///@}

private:
    /** When set to 'true', the turn begins on the second half of the beat. **/
    data_BOOLEAN m_delayed;
    /**
     * Captures the visual rendition and function of the hairpin; that is, whether it
     * indicates an increase or a decrease in volume.
     **/
    turnLog_FORM m_form;
};

//----------------------------------------------------------------------------
// InstTurnLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttTurnLog
 */

class InstTurnLog : public AttTurnLog {
public:
    InstTurnLog() = default;
    virtual ~InstTurnLog() = default;
};

} // namespace libmei

#endif // __LIBMEI_ATTS_CMNORNAMENTS_H__
