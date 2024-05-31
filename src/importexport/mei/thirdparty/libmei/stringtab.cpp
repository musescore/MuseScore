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

#include "stringtab.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

Barre::Barre() :
    Element("barre"), AttLabelled(), AttTyped(), AttStartEndId(), AttStartId()
{
}

Barre::~Barre() {}

bool Barre::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Barre::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("barre");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    return hasAttribute;
}

void Barre::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetStartEndId();
    ResetStartId();
}

Course::Course() :
    Element("course"), AttLabelled(), AttTyped(), AttAccidental(), AttPitch(), AttOctave()
{
}

Course::~Course() {}

bool Course::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadAccidental(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPitch(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadOctave(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Course::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("course");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteAccidental(element) || hasAttribute);
    hasAttribute = (WritePitch(element) || hasAttribute);
    hasAttribute = (WriteOctave(element) || hasAttribute);
    return hasAttribute;
}

void Course::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetAccidental();
    ResetPitch();
    ResetOctave();
}

String::String() :
    Element("string"), AttLabelled(), AttTyped(), AttAccidental(), AttPitch(), AttOctave()
{
}

String::~String() {}

bool String::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadAccidental(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPitch(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadOctave(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool String::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("string");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteAccidental(element) || hasAttribute);
    hasAttribute = (WritePitch(element) || hasAttribute);
    hasAttribute = (WriteOctave(element) || hasAttribute);
    return hasAttribute;
}

void String::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetAccidental();
    ResetPitch();
    ResetOctave();
}

TabDurSym::TabDurSym() :
    Element("tabDurSym"), AttLabelled(), AttTyped(), AttStringtab(), AttLayerIdent(), AttColor(), AttExtSymAuth(), AttExtSymNames(), AttPlacementRelStaff(), AttTypography(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

TabDurSym::~TabDurSym() {}

bool TabDurSym::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStringtab(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool TabDurSym::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("tabDurSym");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteStringtab(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void TabDurSym::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetStringtab();
    ResetLayerIdent();
    ResetColor();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetPlacementRelStaff();
    ResetTypography();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

TabGrp::TabGrp() :
    Element("tabGrp"), AttLabelled(), AttTyped(), AttAugmentDots(), AttDurationLog(), AttLayerIdent(), AttStaffIdent(), AttVisualOffsetHo()
{
}

TabGrp::~TabGrp() {}

bool TabGrp::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadAugmentDots(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool TabGrp::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("tabGrp");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteAugmentDots(element) || hasAttribute);
    hasAttribute = (WriteDurationLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    return hasAttribute;
}

void TabGrp::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetAugmentDots();
    ResetDurationLog();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetVisualOffsetHo();
}



} // namespace libmei
