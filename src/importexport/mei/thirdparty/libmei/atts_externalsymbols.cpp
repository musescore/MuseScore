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

#include "atts_externalsymbols.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

//----------------------------------------------------------------------------
// AttExtSymAuth
//----------------------------------------------------------------------------

AttExtSymAuth::AttExtSymAuth() : Att()
{
    ResetExtSymAuth();
}

void AttExtSymAuth::ResetExtSymAuth()
{
    m_glyphAuth = "";
    m_glyphUri = "";
}

bool AttExtSymAuth::ReadExtSymAuth(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("glyph.auth")) {
        this->SetGlyphAuth(StrToStr(element.attribute("glyph.auth").value()));
        if (removeAttr) element.remove_attribute("glyph.auth");
        hasAttribute = true;
    }
    if (element.attribute("glyph.uri")) {
        this->SetGlyphUri(StrToStr(element.attribute("glyph.uri").value()));
        if (removeAttr) element.remove_attribute("glyph.uri");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttExtSymAuth::WriteExtSymAuth(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasGlyphAuth()) {
        element.append_attribute("glyph.auth") = StrToStr(this->GetGlyphAuth()).c_str();
        wroteAttribute = true;
    }
    if (this->HasGlyphUri()) {
        element.append_attribute("glyph.uri") = StrToStr(this->GetGlyphUri()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttExtSymAuth::HasGlyphAuth() const
{
    return (m_glyphAuth != "");
}

bool AttExtSymAuth::HasGlyphUri() const
{
    return (m_glyphUri != "");
}

//----------------------------------------------------------------------------
// AttExtSymNames
//----------------------------------------------------------------------------

AttExtSymNames::AttExtSymNames() : Att()
{
    ResetExtSymNames();
}

void AttExtSymNames::ResetExtSymNames()
{
    m_glyphName = "";
    m_glyphNum = 0;
}

bool AttExtSymNames::ReadExtSymNames(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("glyph.name")) {
        this->SetGlyphName(StrToStr(element.attribute("glyph.name").value()));
        if (removeAttr) element.remove_attribute("glyph.name");
        hasAttribute = true;
    }
    if (element.attribute("glyph.num")) {
        this->SetGlyphNum(StrToHexnum(element.attribute("glyph.num").value()));
        if (removeAttr) element.remove_attribute("glyph.num");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttExtSymNames::WriteExtSymNames(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasGlyphName()) {
        element.append_attribute("glyph.name") = StrToStr(this->GetGlyphName()).c_str();
        wroteAttribute = true;
    }
    if (this->HasGlyphNum()) {
        element.append_attribute("glyph.num") = HexnumToStr(this->GetGlyphNum()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttExtSymNames::HasGlyphName() const
{
    return (m_glyphName != "");
}

bool AttExtSymNames::HasGlyphNum() const
{
    return (m_glyphNum != 0);
}

} // namespace libmei
