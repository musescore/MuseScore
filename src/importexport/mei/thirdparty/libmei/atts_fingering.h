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

#ifndef __LIBMEI_ATTS_FINGERING_H__
#define __LIBMEI_ATTS_FINGERING_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include <string>

namespace libmei {

//----------------------------------------------------------------------------
// AttFingGrpLog
//----------------------------------------------------------------------------

class AttFingGrpLog : public Att {
protected:
    AttFingGrpLog();
    ~AttFingGrpLog() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetFingGrpLog();

    /** Read the values for the attribute class **/
    bool ReadFingGrpLog(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteFingGrpLog(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetForm(fingGrpLog_FORM form_) { m_form = form_; }
    fingGrpLog_FORM GetForm() const { return m_form; }
    bool HasForm() const;
    ///@}

private:
    /**
     * Captures the visual rendition and function of the hairpin; that is, whether it
     * indicates an increase or a decrease in volume.
     **/
    fingGrpLog_FORM m_form;
};

//----------------------------------------------------------------------------
// InstFingGrpLog
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttFingGrpLog
 */

class InstFingGrpLog : public AttFingGrpLog {
public:
    InstFingGrpLog() = default;
    virtual ~InstFingGrpLog() = default;
};

} // namespace libmei

#endif // __LIBMEI_ATTS_FINGERING_H__
