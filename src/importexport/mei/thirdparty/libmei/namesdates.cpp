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

#include "namesdates.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

PersName::PersName() :
    Element("persName"), AttLabelled(), AttTyped(), AttLang(), AttName(), AttDatable()
{
}

PersName::~PersName() {}

bool PersName::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadName(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDatable(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool PersName::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("persName");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    hasAttribute = (WriteName(element) || hasAttribute);
    hasAttribute = (WriteDatable(element) || hasAttribute);
    return hasAttribute;
}

void PersName::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
    ResetName();
    ResetDatable();
}



} // namespace libmei
