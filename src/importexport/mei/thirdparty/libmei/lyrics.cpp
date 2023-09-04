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

#include "lyrics.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

Refrain::Refrain() :
    Element("refrain"), AttLabelled(), AttTyped(), AttLang(), AttColor(), AttPlacementRelStaff(), AttTypography(), AttVisualOffsetVo(), AttVoltaGroupingSym()
{
}

Refrain::~Refrain() {}

bool Refrain::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVoltaGroupingSym(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Refrain::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("refrain");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteVoltaGroupingSym(element) || hasAttribute);
    return hasAttribute;
}

void Refrain::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
    ResetColor();
    ResetPlacementRelStaff();
    ResetTypography();
    ResetVisualOffsetVo();
    ResetVoltaGroupingSym();
}

Verse::Verse() :
    Element("verse"), AttLabelled(), AttTyped(), AttColor(), AttPlacementRelStaff(), AttTypography(), AttVisualOffsetVo(), AttVoltaGroupingSym(), AttNNumberLike()
{
}

Verse::~Verse() {}

bool Verse::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVoltaGroupingSym(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNNumberLike(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Verse::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("verse");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteVoltaGroupingSym(element) || hasAttribute);
    hasAttribute = (WriteNNumberLike(element) || hasAttribute);
    return hasAttribute;
}

void Verse::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetColor();
    ResetPlacementRelStaff();
    ResetTypography();
    ResetVisualOffsetVo();
    ResetVoltaGroupingSym();
    ResetNNumberLike();
}

Volta::Volta() :
    Element("volta"), AttLabelled(), AttTyped(), AttLang(), AttColor(), AttTypography(), AttVisualOffsetVo()
{
}

Volta::~Volta() {}

bool Volta::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Volta::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("volta");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void Volta::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
    ResetColor();
    ResetTypography();
    ResetVisualOffsetVo();
}



} // namespace libmei
