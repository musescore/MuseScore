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

#include "harmony.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

ChordDef::ChordDef() :
    Element("chordDef"), AttLabelled(), AttTyped(), AttStringtabPosition(), AttStringtabTuning()
{
}

ChordDef::~ChordDef() {}

bool ChordDef::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStringtabPosition(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStringtabTuning(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool ChordDef::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("chordDef");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteStringtabPosition(element) || hasAttribute);
    hasAttribute = (WriteStringtabTuning(element) || hasAttribute);
    return hasAttribute;
}

void ChordDef::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetStringtabPosition();
    ResetStringtabTuning();
}

ChordMember::ChordMember() :
    Element("chordMember"), AttLabelled(), AttTyped(), AttAccidentalGes(), AttPitch(), AttOctave(), AttStringtab()
{
}

ChordMember::~ChordMember() {}

bool ChordMember::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadAccidentalGes(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPitch(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadOctave(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStringtab(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool ChordMember::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("chordMember");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteAccidentalGes(element) || hasAttribute);
    hasAttribute = (WritePitch(element) || hasAttribute);
    hasAttribute = (WriteOctave(element) || hasAttribute);
    hasAttribute = (WriteStringtab(element) || hasAttribute);
    return hasAttribute;
}

void ChordMember::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetAccidentalGes();
    ResetPitch();
    ResetOctave();
    ResetStringtab();
}

ChordTable::ChordTable() :
    Element("chordTable"), AttLabelled(), AttTyped()
{
}

ChordTable::~ChordTable() {}

bool ChordTable::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool ChordTable::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("chordTable");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    return hasAttribute;
}

void ChordTable::Reset() 
{     
    ResetLabelled();
    ResetTyped();
}

F::F() :
    Element("f"), AttLabelled(), AttTyped(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttDurationAdditive(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttColor(), AttExtender(), AttLineRend(), AttLineRendBase(), AttPlacementRelStaff(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

F::~F() {}

bool F::Read(pugi::xml_node element, bool removeAttr)
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

bool F::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("f");
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

void F::Reset() 
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

Fb::Fb() :
    Element("fb"), AttLabelled(), AttTyped()
{
}

Fb::~Fb() {}

bool Fb::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Fb::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("fb");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    return hasAttribute;
}

void Fb::Reset() 
{     
    ResetLabelled();
    ResetTyped();
}

Harm::Harm() :
    Element("harm"), AttLabelled(), AttTyped(), AttHarmLog(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttDurationAdditive(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttHarmVis(), AttColor(), AttExtender(), AttLineRend(), AttLineRendBase(), AttPlacementRelStaff(), AttVisualOffsetHo(), AttVisualOffsetVo(), AttVisualOffset2Ho()
{
}

Harm::~Harm() {}

bool Harm::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadHarmLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationAdditive(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestamp2Log(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadHarmVis(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtender(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRend(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRendBase(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Ho(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Harm::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("harm");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteHarmLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteDurationAdditive(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestamp2Log(element) || hasAttribute);
    hasAttribute = (WriteHarmVis(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExtender(element) || hasAttribute);
    hasAttribute = (WriteLineRend(element) || hasAttribute);
    hasAttribute = (WriteLineRendBase(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Ho(element) || hasAttribute);
    return hasAttribute;
}

void Harm::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetHarmLog();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetDurationAdditive();
    ResetStartEndId();
    ResetStartId();
    ResetTimestamp2Log();
    ResetHarmVis();
    ResetColor();
    ResetExtender();
    ResetLineRend();
    ResetLineRendBase();
    ResetPlacementRelStaff();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
    ResetVisualOffset2Ho();
}



} // namespace libmei
