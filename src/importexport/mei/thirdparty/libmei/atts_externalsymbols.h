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

#ifndef __LIBMEI_ATTS_EXTERNALSYMBOLS_H__
#define __LIBMEI_ATTS_EXTERNALSYMBOLS_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include <string>

namespace libmei {

//----------------------------------------------------------------------------
// AttExtSymAuth
//----------------------------------------------------------------------------

class AttExtSymAuth : public Att {
protected:
    AttExtSymAuth();
    ~AttExtSymAuth() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetExtSymAuth();

    /** Read the values for the attribute class **/
    bool ReadExtSymAuth(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteExtSymAuth(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetGlyphAuth(std::string glyphAuth_) { m_glyphAuth = glyphAuth_; }
    std::string GetGlyphAuth() const { return m_glyphAuth; }
    bool HasGlyphAuth() const;
    //
    void SetGlyphUri(std::string glyphUri_) { m_glyphUri = glyphUri_; }
    std::string GetGlyphUri() const { return m_glyphUri; }
    bool HasGlyphUri() const;
    ///@}

private:
    /**
     * A name or label associated with the controlled vocabulary from which the value
     * of glyph.name or glyph.num is taken, or the textual content of the element.
     **/
    std::string m_glyphAuth;
    /**
     * The web-accessible location of the controlled vocabulary from which the value of
     * glyph.name or glyph.num is taken, or the textual content of the element.
     **/
    std::string m_glyphUri;
};

//----------------------------------------------------------------------------
// InstExtSymAuth
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttExtSymAuth
 */

class InstExtSymAuth : public AttExtSymAuth {
public:
    InstExtSymAuth() = default;
    virtual ~InstExtSymAuth() = default;
};

//----------------------------------------------------------------------------
// AttExtSymNames
//----------------------------------------------------------------------------

class AttExtSymNames : public Att {
protected:
    AttExtSymNames();
    ~AttExtSymNames() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetExtSymNames();

    /** Read the values for the attribute class **/
    bool ReadExtSymNames(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteExtSymNames(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetGlyphName(std::string glyphName_) { m_glyphName = glyphName_; }
    std::string GetGlyphName() const { return m_glyphName; }
    bool HasGlyphName() const;
    //
    void SetGlyphNum(data_HEXNUM glyphNum_) { m_glyphNum = glyphNum_; }
    data_HEXNUM GetGlyphNum() const { return m_glyphNum; }
    bool HasGlyphNum() const;
    ///@}

private:
    /** Glyph name. **/
    std::string m_glyphName;
    /**
     * Numeric glyph reference in hexadecimal notation, e.g., "#xE000" or "U+E000".
     * N.B. SMuFL version 1.18 uses the range U+E000 - U+ECBF.
     **/
    data_HEXNUM m_glyphNum;
};

//----------------------------------------------------------------------------
// InstExtSymNames
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttExtSymNames
 */

class InstExtSymNames : public AttExtSymNames {
public:
    InstExtSymNames() = default;
    virtual ~InstExtSymNames() = default;
};

} // namespace libmei

#endif // __LIBMEI_ATTS_EXTERNALSYMBOLS_H__
