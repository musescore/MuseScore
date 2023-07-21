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

#include "header.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

Availability::Availability() :
    Element("availability"), AttLabelled(), AttTyped(), AttDataPointing()
{
}

Availability::~Availability() {}

bool Availability::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDataPointing(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Availability::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("availability");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteDataPointing(element) || hasAttribute);
    return hasAttribute;
}

void Availability::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetDataPointing();
}

FileDesc::FileDesc() :
    Element("fileDesc"), AttLabelled(), AttTyped()
{
}

FileDesc::~FileDesc() {}

bool FileDesc::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool FileDesc::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("fileDesc");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    return hasAttribute;
}

void FileDesc::Reset() 
{     
    ResetLabelled();
    ResetTyped();
}

MeiHead::MeiHead() :
    Element("meiHead"), AttLabelled(), AttLang()
{
}

MeiHead::~MeiHead() {}

bool MeiHead::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool MeiHead::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("meiHead");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    return hasAttribute;
}

void MeiHead::Reset() 
{     
    ResetLabelled();
    ResetLang();
}

PubStmt::PubStmt() :
    Element("pubStmt"), AttLabelled(), AttTyped()
{
}

PubStmt::~PubStmt() {}

bool PubStmt::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool PubStmt::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("pubStmt");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    return hasAttribute;
}

void PubStmt::Reset() 
{     
    ResetLabelled();
    ResetTyped();
}

TitleStmt::TitleStmt() :
    Element("titleStmt"), AttLabelled(), AttTyped()
{
}

TitleStmt::~TitleStmt() {}

bool TitleStmt::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool TitleStmt::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("titleStmt");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    return hasAttribute;
}

void TitleStmt::Reset() 
{     
    ResetLabelled();
    ResetTyped();
}



} // namespace libmei
