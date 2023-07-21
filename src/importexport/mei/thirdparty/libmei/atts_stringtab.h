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

#ifndef __LIBMEI_ATTS_STRINGTAB_H__
#define __LIBMEI_ATTS_STRINGTAB_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include <string>

namespace libmei {

//----------------------------------------------------------------------------
// AttStringtab
//----------------------------------------------------------------------------

class AttStringtab : public Att {
protected:
    AttStringtab();
    ~AttStringtab() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetStringtab();

    /** Read the values for the attribute class **/
    bool ReadStringtab(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteStringtab(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetTabFing(std::string tabFing_) { m_tabFing = tabFing_; }
    std::string GetTabFing() const { return m_tabFing; }
    bool HasTabFing() const;
    //
    void SetTabFret(std::string tabFret_) { m_tabFret = tabFret_; }
    std::string GetTabFret() const { return m_tabFret; }
    bool HasTabFret() const;
    //
    void SetTabString(std::string tabString_) { m_tabString = tabString_; }
    std::string GetTabString() const { return m_tabString; }
    bool HasTabString() const;
    ///@}

private:
    /**
     * Indicates which finger, if any, should be used to play an individual string.
     * The index, middle, ring, and little fingers are represented by the values 1-4,
     * while t is for the thumb. The values x and o indicate muffled and open strings,
     * respectively.
     **/
    std::string m_tabFing;
    /** Records the location at which a string should be stopped against a fret. **/
    std::string m_tabFret;
    /** Records which string is to be played. **/
    std::string m_tabString;
};

//----------------------------------------------------------------------------
// InstStringtab
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttStringtab
 */

class InstStringtab : public AttStringtab {
public:
    InstStringtab() = default;
    virtual ~InstStringtab() = default;
};

//----------------------------------------------------------------------------
// AttStringtabPosition
//----------------------------------------------------------------------------

class AttStringtabPosition : public Att {
protected:
    AttStringtabPosition();
    ~AttStringtabPosition() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetStringtabPosition();

    /** Read the values for the attribute class **/
    bool ReadStringtabPosition(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteStringtabPosition(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetTabPos(int tabPos_) { m_tabPos = tabPos_; }
    int GetTabPos() const { return m_tabPos; }
    bool HasTabPos() const;
    ///@}

private:
    /** Records fret position. **/
    int m_tabPos;
};

//----------------------------------------------------------------------------
// InstStringtabPosition
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttStringtabPosition
 */

class InstStringtabPosition : public AttStringtabPosition {
public:
    InstStringtabPosition() = default;
    virtual ~InstStringtabPosition() = default;
};

//----------------------------------------------------------------------------
// AttStringtabTuning
//----------------------------------------------------------------------------

class AttStringtabTuning : public Att {
protected:
    AttStringtabTuning();
    ~AttStringtabTuning() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetStringtabTuning();

    /** Read the values for the attribute class **/
    bool ReadStringtabTuning(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteStringtabTuning(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetTabStrings(std::string tabStrings_) { m_tabStrings = tabStrings_; }
    std::string GetTabStrings() const { return m_tabStrings; }
    bool HasTabStrings() const;
    ///@}

private:
    /** Provides a *written* pitch and octave for each open string or course of strings. **/
    std::string m_tabStrings;
};

//----------------------------------------------------------------------------
// InstStringtabTuning
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttStringtabTuning
 */

class InstStringtabTuning : public AttStringtabTuning {
public:
    InstStringtabTuning() = default;
    virtual ~InstStringtabTuning() = default;
};

} // namespace libmei

#endif // __LIBMEI_ATTS_STRINGTAB_H__
