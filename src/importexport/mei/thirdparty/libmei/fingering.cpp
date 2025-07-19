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

#include "fingering.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

Fing::Fing() :
    Element("fing"), AttLabelled(), AttTyped(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttDurationAdditive(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttColor(), AttExtender(), AttLineRend(), AttLineRendBase(), AttPlacementRelStaff(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

Fing::~Fing() {}

bool Fing::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationAdditive(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestamp2Log(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtender(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRend(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRendBase(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Fing::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("fing");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteDurationAdditive(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestamp2Log(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExtender(element) || hasAttribute);
    hasAttribute = (WriteLineRend(element) || hasAttribute);
    hasAttribute = (WriteLineRendBase(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void Fing::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetDurationAdditive();
    ResetStartEndId();
    ResetStartId();
    ResetTimestamp2Log();
    ResetColor();
    ResetExtender();
    ResetLineRend();
    ResetLineRendBase();
    ResetPlacementRelStaff();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

FingGrp::FingGrp() :
    Element("fingGrp"), AttLabelled(), AttTyped(), AttFingGrpLog(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttDurationAdditive(), AttStartEndId(), AttStartId(), AttTimestamp2Log()
{
}

FingGrp::~FingGrp() {}

bool FingGrp::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadFingGrpLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationAdditive(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestamp2Log(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool FingGrp::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("fingGrp");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteFingGrpLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteDurationAdditive(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestamp2Log(element) || hasAttribute);
    return hasAttribute;
}

void FingGrp::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetFingGrpLog();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetDurationAdditive();
    ResetStartEndId();
    ResetStartId();
    ResetTimestamp2Log();
}



} // namespace libmei
