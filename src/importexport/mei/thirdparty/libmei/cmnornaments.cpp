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

#include "cmnornaments.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

Mordent::Mordent() :
    Element("mordent"), AttLabelled(), AttTyped(), AttMordentLog(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttStartEndId(), AttStartId(), AttOrnamentAccid(), AttOrnamentAccidGes(), AttColor(), AttEnclosingChars(), AttExtSymAuth(), AttExtSymNames(), AttPlacementRelStaff(), AttTypography(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

Mordent::~Mordent() {}

bool Mordent::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadMordentLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadOrnamentAccid(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadOrnamentAccidGes(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadEnclosingChars(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Mordent::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("mordent");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteMordentLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteOrnamentAccid(element) || hasAttribute);
    hasAttribute = (WriteOrnamentAccidGes(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteEnclosingChars(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void Mordent::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetMordentLog();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetStartEndId();
    ResetStartId();
    ResetOrnamentAccid();
    ResetOrnamentAccidGes();
    ResetColor();
    ResetEnclosingChars();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetPlacementRelStaff();
    ResetTypography();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

Trill::Trill() :
    Element("trill"), AttLabelled(), AttTyped(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttDurationAdditive(), AttOrnamentAccid(), AttOrnamentAccidGes(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttColor(), AttEnclosingChars(), AttExtender(), AttLineRend(), AttLineRendBase(), AttExtSymAuth(), AttExtSymNames(), AttPlacementRelStaff(), AttTypography(), AttVisualOffsetHo(), AttVisualOffsetVo(), AttVisualOffset2Ho()
{
}

Trill::~Trill() {}

bool Trill::Read(pugi::xml_node element, bool removeAttr)
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
    hasAttribute = (ReadOrnamentAccid(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadOrnamentAccidGes(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestamp2Log(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadEnclosingChars(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtender(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRend(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRendBase(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Ho(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Trill::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("trill");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteDurationAdditive(element) || hasAttribute);
    hasAttribute = (WriteOrnamentAccid(element) || hasAttribute);
    hasAttribute = (WriteOrnamentAccidGes(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestamp2Log(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteEnclosingChars(element) || hasAttribute);
    hasAttribute = (WriteExtender(element) || hasAttribute);
    hasAttribute = (WriteLineRend(element) || hasAttribute);
    hasAttribute = (WriteLineRendBase(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Ho(element) || hasAttribute);
    return hasAttribute;
}

void Trill::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetDurationAdditive();
    ResetOrnamentAccid();
    ResetOrnamentAccidGes();
    ResetStartEndId();
    ResetStartId();
    ResetTimestamp2Log();
    ResetColor();
    ResetEnclosingChars();
    ResetExtender();
    ResetLineRend();
    ResetLineRendBase();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetPlacementRelStaff();
    ResetTypography();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
    ResetVisualOffset2Ho();
}

Turn::Turn() :
    Element("turn"), AttLabelled(), AttTyped(), AttTurnLog(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttOrnamentAccid(), AttOrnamentAccidGes(), AttStartId(), AttColor(), AttEnclosingChars(), AttExtSymAuth(), AttExtSymNames(), AttPlacementRelStaff(), AttTypography(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

Turn::~Turn() {}

bool Turn::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTurnLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadOrnamentAccid(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadOrnamentAccidGes(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadEnclosingChars(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Turn::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("turn");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteTurnLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteOrnamentAccid(element) || hasAttribute);
    hasAttribute = (WriteOrnamentAccidGes(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteEnclosingChars(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void Turn::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetTurnLog();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetOrnamentAccid();
    ResetOrnamentAccidGes();
    ResetStartId();
    ResetColor();
    ResetEnclosingChars();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetPlacementRelStaff();
    ResetTypography();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}



} // namespace libmei
